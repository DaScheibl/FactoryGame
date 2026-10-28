// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryGameInstance.h"
#include "FactoryGameUserSettings.h"
#include "MainMenuGameMode.h"
#include "FactoryGame.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "FactoryGamePlayerController.h"
#include "FactoryInventoryComponent.h"
#include "FactoryGameCharacter.h"
#include "Misc/Guid.h"
#include "FactoryMachine.h"
#include "FactoryCrate.h"
#include "EngineUtils.h"
#include "Engine/World.h"

UFactoryGameInstance::UFactoryGameInstance()
{
	// Vorläufig: flache Testwelt zum Bauen (Tools/CreateFlatMap.py), die Template-Map bleibt Menü-Hintergrund.
	// Beide Properties sind "Config" und lassen sich in DefaultGame.ini überschreiben.
	GameplayMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/FactoryGame/Maps/Lvl_Flat.Lvl_Flat")));
	MainMenuMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/FirstPerson/Lvl_FirstPerson.Lvl_FirstPerson")));
}

UFactoryGameInstance* UFactoryGameInstance::Get(const UObject* WorldContextObject)
{
	return Cast<UFactoryGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject));
}

void UFactoryGameInstance::Init()
{
	Super::Init();

	// Liste der gespeicherten Welten einlesen, damit das Hauptmenü sie sofort anzeigen kann
	LoadWorldIndex();

	// apply the stored user settings, this also selects the UI language
	if (UFactoryGameUserSettings* Settings = UFactoryGameUserSettings::GetFactoryGameUserSettings())
	{
		Settings->ApplySettings(false);
	}

	// Der Core Ticker läuft unabhängig von Levelwechseln und Pause. Gespeichert wird aber nur,
	// wenn gerade eine Welt aktiv ist (siehe SaveCurrentWorld).
	if (AutosaveInterval > 0.0f)
	{
		AutosaveTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UFactoryGameInstance::HandleAutosave), AutosaveInterval);
	}
}

void UFactoryGameInstance::Shutdown()
{
	FTSTicker::GetCoreTicker().RemoveTicker(AutosaveTickerHandle);

	SaveCurrentWorld();

	Super::Shutdown();
}

TSubclassOf<AGameModeBase> UFactoryGameInstance::OverrideGameModeClass(TSubclassOf<AGameModeBase> GameModeClass, const FString& MapName, const FString& Options, const FString& Portal) const
{
	// no world selected yet, so show the main menu
	if (!IsWorldActive())
	{
		return AMainMenuGameMode::StaticClass();
	}

	return Super::OverrideGameModeClass(GameModeClass, MapName, Options, Portal);
}

TArray<FFactoryWorldInfo> UFactoryGameInstance::GetSavedWorlds() const
{
	TArray<FFactoryWorldInfo> Worlds;

	if (WorldIndex)
	{
		Worlds = WorldIndex->Worlds;
		Worlds.Sort([](const FFactoryWorldInfo& A, const FFactoryWorldInfo& B) { return A.LastPlayedAt > B.LastPlayedAt; });
	}

	return Worlds;
}

bool UFactoryGameInstance::GetLastPlayedWorld(FFactoryWorldInfo& OutInfo) const
{
	if (WorldIndex)
	{
		if (const FFactoryWorldInfo* Info = WorldIndex->Worlds.FindByPredicate([this](const FFactoryWorldInfo& Entry) { return Entry.SlotName == WorldIndex->LastPlayedSlot; }))
		{
			OutInfo = *Info;
			return true;
		}
	}

	return false;
}

bool UFactoryGameInstance::CreateNewWorld(const FString& DisplayName)
{
	const FString TrimmedName = DisplayName.TrimStartAndEnd();

	if (TrimmedName.IsEmpty() || !WorldIndex)
	{
		return false;
	}

	// create the world save game
	UFactoryWorldSaveGame* SaveGame = Cast<UFactoryWorldSaveGame>(UGameplayStatics::CreateSaveGameObject(UFactoryWorldSaveGame::StaticClass()));
	SaveGame->DisplayName = TrimmedName;
	SaveGame->SaveVersion = static_cast<int32>(EFactoryWorldSaveVersion::Latest);

	// Eine GUID als Slot-Name erlaubt mehrere Welten mit gleichem Namen und späteres Umbenennen,
	// ohne die Datei zu verschieben.
	const FString SlotName = FString::Printf(TEXT("World_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits));

	if (!UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0))
	{
		UE_LOG(LogFactoryGame, Error, TEXT("Could not create save game for world '%s'."), *TrimmedName);
		return false;
	}

	// register it in the index
	FFactoryWorldInfo& Info = WorldIndex->Worlds.AddDefaulted_GetRef();
	Info.SlotName = SlotName;
	Info.DisplayName = TrimmedName;
	Info.CreatedAt = FDateTime::UtcNow();
	Info.LastPlayedAt = Info.CreatedAt;

	StartWorld(SlotName, SaveGame);
	return true;
}

bool UFactoryGameInstance::LoadWorld(const FString& SlotName)
{
	if (!FindWorldInfo(SlotName))
	{
		return false;
	}

	UFactoryWorldSaveGame* SaveGame = Cast<UFactoryWorldSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

	if (!SaveGame)
	{
		UE_LOG(LogFactoryGame, Error, TEXT("Could not load world from slot '%s'."), *SlotName);
		return false;
	}

	// refuse saves from newer game versions instead of overwriting them with less data
	if (!SaveGame->MigrateToLatest())
	{
		return false;
	}

	StartWorld(SlotName, SaveGame);
	return true;
}

bool UFactoryGameInstance::ContinueLastWorld()
{
	return WorldIndex && LoadWorld(WorldIndex->LastPlayedSlot);
}

void UFactoryGameInstance::DeleteWorld(const FString& SlotName)
{
	if (!WorldIndex || SlotName == CurrentWorldSlot)
	{
		return;
	}

	UGameplayStatics::DeleteGameInSlot(SlotName, 0);

	WorldIndex->Worlds.RemoveAll([&SlotName](const FFactoryWorldInfo& Entry) { return Entry.SlotName == SlotName; });

	if (WorldIndex->LastPlayedSlot == SlotName)
	{
		// fall back to the most recently played remaining world
		const TArray<FFactoryWorldInfo> Remaining = GetSavedWorlds();
		WorldIndex->LastPlayedSlot = Remaining.Num() > 0 ? Remaining[0].SlotName : FString();
	}

	SaveWorldIndex();
}

bool UFactoryGameInstance::SaveCurrentWorld()
{
	if (!IsWorldActive() || !CurrentWorldSave)
	{
		return false;
	}

	CapturePlayerState();

	// accumulate the play time since the last save
	const double Now = FPlatformTime::Seconds();
	CurrentWorldSave->PlayTimeSeconds += Now - PlayTimeCounterStart;
	PlayTimeCounterStart = Now;

	if (!UGameplayStatics::SaveGameToSlot(CurrentWorldSave, CurrentWorldSlot, 0))
	{
		UE_LOG(LogFactoryGame, Error, TEXT("Could not save world to slot '%s'."), *CurrentWorldSlot);
		return false;
	}

	if (FFactoryWorldInfo* Info = FindWorldInfo(CurrentWorldSlot))
	{
		Info->LastPlayedAt = FDateTime::UtcNow();
		Info->PlayTimeSeconds = CurrentWorldSave->PlayTimeSeconds;
	}

	SaveWorldIndex();
	return true;
}

void UFactoryGameInstance::ReturnToMainMenu()
{
	SaveCurrentWorld();

	CurrentWorldSlot.Empty();
	CurrentWorldSave = nullptr;
	bPendingPlayerRestore = false;

	OpenMap(MainMenuMap);
}

void UFactoryGameInstance::RestorePlayerState(APlayerController* PlayerController)
{
	if (!bPendingPlayerRestore || !CurrentWorldSave || !PlayerController)
	{
		return;
	}

	APawn* Pawn = PlayerController->GetPawn();

	if (!Pawn)
	{
		return;
	}

	// TeleportPhysics: kein Sweep und keine Geschwindigkeit aus dem Positionssprung
	Pawn->SetActorLocationAndRotation(CurrentWorldSave->PlayerTransform.GetLocation(), CurrentWorldSave->PlayerTransform.GetRotation(), false, nullptr, ETeleportType::TeleportPhysics);
	PlayerController->SetControlRotation(CurrentWorldSave->ControlRotation);

	if (const AFactoryGameCharacter* Character = Cast<AFactoryGameCharacter>(Pawn))
	{
		// 0 Slots = Welt ohne gespeichertes Inventar, dann bleibt das leere Start-Inventar
		if (CurrentWorldSave->InventoryNumSlots > 0)
		{
			Character->GetInventory()->SetSlots(CurrentWorldSave->InventorySlots, CurrentWorldSave->InventoryNumSlots);
		}

		// die Ausrüstung hat feste Slots, die Anzahl wird deshalb ignoriert
		Character->GetEquipment()->SetSlots(CurrentWorldSave->EquipmentSlots, 0);
	}

	if (AFactoryGamePlayerController* FactoryController = Cast<AFactoryGamePlayerController>(PlayerController))
	{
		FactoryController->SetHotbarSlots(CurrentWorldSave->HotbarSlots);
	}

	RestoreBuildings(PlayerController->GetWorld());

	bPendingPlayerRestore = false;
}

void UFactoryGameInstance::LoadWorldIndex()
{
	if (UGameplayStatics::DoesSaveGameExist(UFactoryWorldIndexSave::SlotName, 0))
	{
		WorldIndex = Cast<UFactoryWorldIndexSave>(UGameplayStatics::LoadGameFromSlot(UFactoryWorldIndexSave::SlotName, 0));
	}

	if (!WorldIndex)
	{
		WorldIndex = Cast<UFactoryWorldIndexSave>(UGameplayStatics::CreateSaveGameObject(UFactoryWorldIndexSave::StaticClass()));
		return;
	}

	// drop entries whose save file was removed by hand
	const int32 Removed = WorldIndex->Worlds.RemoveAll([](const FFactoryWorldInfo& Entry) { return !UGameplayStatics::DoesSaveGameExist(Entry.SlotName, 0); });

	if (Removed > 0)
	{
		SaveWorldIndex();
	}
}

void UFactoryGameInstance::SaveWorldIndex()
{
	if (WorldIndex && !UGameplayStatics::SaveGameToSlot(WorldIndex, UFactoryWorldIndexSave::SlotName, 0))
	{
		UE_LOG(LogFactoryGame, Error, TEXT("Could not save the world index."));
	}
}

FFactoryWorldInfo* UFactoryGameInstance::FindWorldInfo(const FString& SlotName)
{
	if (!WorldIndex || SlotName.IsEmpty())
	{
		return nullptr;
	}

	return WorldIndex->Worlds.FindByPredicate([&SlotName](const FFactoryWorldInfo& Entry) { return Entry.SlotName == SlotName; });
}

void UFactoryGameInstance::StartWorld(const FString& SlotName, UFactoryWorldSaveGame* SaveGame)
{
	CurrentWorldSlot = SlotName;
	CurrentWorldSave = SaveGame;
	// eine neue Welt hat noch keine Spielerdaten, dann bleibt die Figur am PlayerStart
	bPendingPlayerRestore = SaveGame->bHasPlayerData;
	PlayTimeCounterStart = FPlatformTime::Seconds();

	WorldIndex->LastPlayedSlot = SlotName;

	if (FFactoryWorldInfo* Info = FindWorldInfo(SlotName))
	{
		Info->LastPlayedAt = FDateTime::UtcNow();
	}

	SaveWorldIndex();

	// Der Levelwechsel passiert erst im nächsten Frame. Den Spieler stellt danach
	// AFactoryGamePlayerController::OnPossess wieder her.
	OpenMap(GameplayMap);
}

void UFactoryGameInstance::CapturePlayerState()
{
	// keep the previous state if the pawn is already gone, e.g. during map teardown
	const APlayerController* PlayerController = GetFirstLocalPlayerController();
	const APawn* Pawn = IsValid(PlayerController) ? PlayerController->GetPawn() : nullptr;

	if (!IsValid(Pawn) || !CurrentWorldSave)
	{
		return;
	}

	CurrentWorldSave->bHasPlayerData = true;
	CurrentWorldSave->PlayerTransform = Pawn->GetActorTransform();
	CurrentWorldSave->ControlRotation = PlayerController->GetControlRotation();

	if (const AFactoryGameCharacter* Character = Cast<AFactoryGameCharacter>(Pawn))
	{
		CurrentWorldSave->InventorySlots = Character->GetInventory()->GetSlots();
		CurrentWorldSave->InventoryNumSlots = Character->GetInventory()->GetNumSlots();
		CurrentWorldSave->EquipmentSlots = Character->GetEquipment()->GetSlots();
	}

	if (const AFactoryGamePlayerController* FactoryController = Cast<AFactoryGamePlayerController>(PlayerController))
	{
		CurrentWorldSave->HotbarSlots = FactoryController->GetHotbarSlots();
	}

	// Gebäude nur zusammen mit dem Spieler erfassen: fehlt der Pawn, wird die Welt gerade abgebaut
	CaptureBuildings(Pawn->GetWorld());
}

void UFactoryGameInstance::CaptureBuildings(UWorld* World)
{
	if (!World || !CurrentWorldSave)
	{
		return;
	}

	CurrentWorldSave->Machines.Reset();
	CurrentWorldSave->Crates.Reset();

	for (TActorIterator<AFactoryMachine> It(World); It; ++It)
	{
		FFactorySavedMachine& Saved = CurrentWorldSave->Machines.AddDefaulted_GetRef();
		Saved.BuildingId = It->GetProduction()->GetBuildingId();
		Saved.Transform = It->GetActorTransform();
		Saved.Production = It->GetProduction()->GetSaveData();
	}

	for (TActorIterator<AFactoryCrate> It(World); It; ++It)
	{
		FFactorySavedCrate& Saved = CurrentWorldSave->Crates.AddDefaulted_GetRef();
		Saved.Location = It->GetActorLocation() - FVector(0.0, 0.0, 50.0);
		Saved.Items = It->GetItems();
	}
}

void UFactoryGameInstance::RestoreBuildings(UWorld* World)
{
	if (!World || !CurrentWorldSave)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FFactorySavedMachine& Saved : CurrentWorldSave->Machines)
	{
		if (AFactoryMachine* Machine = World->SpawnActor<AFactoryMachine>(Saved.Transform.GetLocation(), Saved.Transform.Rotator(), SpawnParams))
		{
			Machine->InitBuilding(Saved.BuildingId);
			Machine->GetProduction()->LoadSaveData(Saved.Production);
		}
	}

	for (const FFactorySavedCrate& Saved : CurrentWorldSave->Crates)
	{
		AFactoryCrate::SpawnCrate(World, Saved.Location, Saved.Items);
	}

	UE_LOG(LogFactoryGame, Log, TEXT("Restored %d machines and %d crates."), CurrentWorldSave->Machines.Num(), CurrentWorldSave->Crates.Num());
}

void UFactoryGameInstance::OpenMap(const TSoftObjectPtr<UWorld>& Map)
{
	// beim Laden der Map fragt die Engine OverrideGameModeClass(), welcher Game Mode gilt
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Map);
}

bool UFactoryGameInstance::HandleAutosave(float DeltaTime)
{
	SaveCurrentWorld();

	// keep ticking
	return true;
}
