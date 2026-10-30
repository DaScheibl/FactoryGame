// Copyright Epic Games, Inc. All Rights Reserved.


#include "FactoryGamePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "InputMappingContext.h"
#include "FactoryGameCameraManager.h"
#include "FactoryGameInstance.h"
#include "PauseMenuWidget.h"
#include "FactoryHUDWidget.h"
#include "FactoryInventoryWidget.h"
#include "FactoryBuildMenuWidget.h"
#include "FactoryInventoryComponent.h"
#include "FactoryGameCharacter.h"
#include "FactoryItemDatabase.h"
#include "FactoryCheatManager.h"
#include "FactoryBuildComponent.h"
#include "FactoryBuildingDatabase.h"
#include "FactoryLocalization.h"
#include "Blueprint/UserWidget.h"
#include "FactoryGame.h"
#include "Widgets/Input/SVirtualJoystick.h"

AFactoryGamePlayerController::AFactoryGamePlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AFactoryGameCameraManager::StaticClass();

	// set the default menus
	PauseMenuWidgetClass = UPauseMenuWidget::StaticClass();
	HUDWidgetClass = UFactoryHUDWidget::StaticClass();
	InventoryWidgetClass = UFactoryInventoryWidget::StaticClass();
	BuildMenuWidgetClass = UFactoryBuildMenuWidget::StaticClass();

	// console commands for testing
	CheatClass = UFactoryCheatManager::StaticClass();

	// alle 10 Slots existieren immer, leere Slots enthalten NAME_None
	HotbarSlots.SetNum(NumHotbarSlots);

	BuildComponent = CreateDefaultSubobject<UFactoryBuildComponent>(TEXT("BuildComponent"));
}

void AFactoryGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// the main menu leaves the viewport in UI mode, so switch back to game input
	if (IsLocalPlayerController())
	{
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
	}

	// HUD auf Z-Order 0; Inventar und Baumenü (50) und Pausemenü (100) liegen darüber
	if (IsLocalPlayerController() && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UFactoryHUDWidget>(this, HUDWidgetClass);

		if (HUDWidget)
		{
			HUDWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(LogFactoryGame, Error, TEXT("Could not spawn HUD widget."));
		}
	}

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogFactoryGame, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AFactoryGamePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// save while the pawn still exists when the game is closed
	// (beim Zurück ins Hauptmenü speichert ReturnToMainMenu schon vorher, deshalb nur Beenden/PIE-Ende)
	if (EndPlayReason == EEndPlayReason::Quit || EndPlayReason == EEndPlayReason::EndPlayInEditor)
	{
		if (UFactoryGameInstance* GameInstance = UFactoryGameInstance::Get(this))
		{
			GameInstance->SaveCurrentWorld();
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AFactoryGamePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (UFactoryGameInstance* GameInstance = UFactoryGameInstance::Get(this))
	{
		// nach dem Laden einer Welt Position, Inventar und Hotbar zurückholen (tut nichts, wenn nichts ansteht)
		GameInstance->RestorePlayerState(this);
	}

	FillDefaultHotbar();
}

void AFactoryGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Feste UI-Tasten werden direkt per BindKey gebunden. Die Bewegung der Spielfigur läuft
	// dagegen über Enhanced Input (AFactoryGameCharacter::SetupPlayerInputComponent).
	// pause menu keys. P is also bound because Escape ends Play In Editor by default
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AFactoryGamePlayerController::HandlePauseKey);
	InputComponent->BindKey(EKeys::P, IE_Pressed, this, &AFactoryGamePlayerController::HandlePauseKey);
	InputComponent->BindKey(EKeys::Gamepad_Special_Right, IE_Pressed, this, &AFactoryGamePlayerController::TogglePauseMenu);

	// inventory
	InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AFactoryGamePlayerController::ToggleInventory);
	InputComponent->BindKey(EKeys::I, IE_Pressed, this, &AFactoryGamePlayerController::ToggleInventory);
	InputComponent->BindKey(EKeys::Gamepad_Special_Left, IE_Pressed, this, &AFactoryGamePlayerController::ToggleInventory);

	// build menu
	InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &AFactoryGamePlayerController::ToggleBuildMenu);
	InputComponent->BindKey(EKeys::Gamepad_DPad_Up, IE_Pressed, this, &AFactoryGamePlayerController::ToggleBuildMenu);

	// hotbar: number keys and d-pad
	for (const FKey& Key : { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine, EKeys::Zero })
	{
		InputComponent->BindKey(Key, IE_Pressed, this, &AFactoryGamePlayerController::HandleHotbarKey);
	}

	InputComponent->BindKey(EKeys::Gamepad_DPad_Left, IE_Pressed, this, &AFactoryGamePlayerController::SelectPreviousHotbarSlot);
	InputComponent->BindKey(EKeys::Gamepad_DPad_Right, IE_Pressed, this, &AFactoryGamePlayerController::SelectNextHotbarSlot);

	// build tool (Docs/Standards/13-bauen.md)
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AFactoryGamePlayerController::HandlePrimaryPressed);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AFactoryGamePlayerController::HandlePrimaryReleased);
	InputComponent->BindKey(EKeys::Gamepad_RightTrigger, IE_Pressed, this, &AFactoryGamePlayerController::HandlePrimaryPressed);
	InputComponent->BindKey(EKeys::Gamepad_RightTrigger, IE_Released, this, &AFactoryGamePlayerController::HandlePrimaryReleased);
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AFactoryGamePlayerController::HandleCancel);
	InputComponent->BindKey(EKeys::Gamepad_FaceButton_Right, IE_Pressed, this, &AFactoryGamePlayerController::HandleCancel);
	InputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &AFactoryGamePlayerController::HandleRotateRight);
	InputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &AFactoryGamePlayerController::HandleRotateLeft);
	InputComponent->BindKey(EKeys::Gamepad_RightShoulder, IE_Pressed, this, &AFactoryGamePlayerController::HandleRotateRight);
	InputComponent->BindKey(EKeys::Gamepad_LeftShoulder, IE_Pressed, this, &AFactoryGamePlayerController::HandleRotateLeft);
	InputComponent->BindKey(EKeys::F, IE_Pressed, this, &AFactoryGamePlayerController::HandleToggleDemolish);
	InputComponent->BindKey(EKeys::Gamepad_DPad_Down, IE_Pressed, this, &AFactoryGamePlayerController::HandleToggleDemolish);
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AFactoryGamePlayerController::HandleCycleMode);
	InputComponent->BindKey(EKeys::Gamepad_FaceButton_Top, IE_Pressed, this, &AFactoryGamePlayerController::HandleCycleMode);
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AFactoryGamePlayerController::HandleInteract);
	InputComponent->BindKey(EKeys::Gamepad_FaceButton_Left, IE_Pressed, this, &AFactoryGamePlayerController::HandleInteract);

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
	
}

bool AFactoryGamePlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AFactoryGamePlayerController::TogglePauseMenu()
{
	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		ClosePauseMenu();
	}
	else
	{
		OpenPauseMenu();
	}
}

void AFactoryGamePlayerController::OpenPauseMenu()
{
	if (!IsLocalPlayerController() || !PauseMenuWidgetClass)
	{
		return;
	}

	// the pause menu replaces the inventory and the build menu
	if (InventoryWidget && InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromParent();
	}

	if (BuildMenuWidget && BuildMenuWidget->IsInViewport())
	{
		BuildMenuWidget->RemoveFromParent();
	}

	if (!PauseMenuWidget)
	{
		PauseMenuWidget = CreateWidget<UPauseMenuWidget>(this, PauseMenuWidgetClass);

		if (!PauseMenuWidget)
		{
			UE_LOG(LogFactoryGame, Error, TEXT("Could not spawn pause menu widget."));
			return;
		}

		PauseMenuWidget->OnResumeRequested.BindUObject(this, &AFactoryGamePlayerController::ClosePauseMenu);
	}

	// rebuild to clear old status messages and pick up language changes
	PauseMenuWidget->RebuildMenu();
	PauseMenuWidget->AddToPlayerScreen(100);

	// SetPause hält die Welt an (Bewegung, Physik, später Maschinen). Widgets laufen weiter.
	SetPause(true);

	// UI-Modus: Eingaben gehen nur an das Menü, der Mauszeiger wird sichtbar
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void AFactoryGamePlayerController::ClosePauseMenu()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromParent();
	}

	SetPause(false);

	RestoreGameInput();
}

void AFactoryGamePlayerController::RestoreGameInput()
{
	// nur in den Spielmodus zurück, wenn kein anderes Fenster mehr offen ist
	const bool bPauseOpen = PauseMenuWidget && PauseMenuWidget->IsInViewport();
	const bool bInventoryOpen = InventoryWidget && InventoryWidget->IsInViewport();
	const bool bBuildMenuOpen = BuildMenuWidget && BuildMenuWidget->IsInViewport();

	if (bPauseOpen || bInventoryOpen || bBuildMenuOpen)
	{
		return;
	}

	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
}

void AFactoryGamePlayerController::ToggleInventory()
{
	if (InventoryWidget && InventoryWidget->IsInViewport())
	{
		CloseInventory();
	}
	else
	{
		OpenInventory();
	}
}

void AFactoryGamePlayerController::OpenInventory()
{
	if (!IsLocalPlayerController() || !InventoryWidgetClass || (PauseMenuWidget && PauseMenuWidget->IsInViewport()))
	{
		return;
	}

	const AFactoryGameCharacter* FactoryCharacter = Cast<AFactoryGameCharacter>(GetPawn());

	if (!FactoryCharacter)
	{
		return;
	}

	// the inventory replaces the build menu
	if (BuildMenuWidget && BuildMenuWidget->IsInViewport())
	{
		BuildMenuWidget->RemoveFromParent();
	}

	// Widget beim ersten Öffnen erzeugen und danach wiederverwenden
	if (!InventoryWidget)
	{
		InventoryWidget = CreateWidget<UFactoryInventoryWidget>(this, InventoryWidgetClass);

		if (!InventoryWidget)
		{
			UE_LOG(LogFactoryGame, Error, TEXT("Could not spawn inventory widget."));
			return;
		}

		InventoryWidget->OnCloseRequested.BindUObject(this, &AFactoryGamePlayerController::CloseInventory);
	}

	InventoryWidget->SetInventory(FactoryCharacter->GetInventory(), FactoryCharacter->GetEquipment());
	InventoryWidget->AddToPlayerScreen(50);

	// the game keeps running while the inventory is open
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
	SetInputMode(InputMode);
	SetShowMouseCursor(true);

	// the input mode focuses the whole widget, move the focus onto a slot for gamepad navigation
	InventoryWidget->FocusCurrentSlot();
}

void AFactoryGamePlayerController::CloseInventory()
{
	if (InventoryWidget)
	{
		InventoryWidget->RemoveFromParent();
	}

	RestoreGameInput();
}

void AFactoryGamePlayerController::ToggleBuildMenu()
{
	if (BuildMenuWidget && BuildMenuWidget->IsInViewport())
	{
		CloseBuildMenu();
	}
	else
	{
		OpenBuildMenu();
	}
}

void AFactoryGamePlayerController::OpenBuildMenu()
{
	if (!IsLocalPlayerController() || !BuildMenuWidgetClass || (PauseMenuWidget && PauseMenuWidget->IsInViewport()))
	{
		return;
	}

	// the build menu replaces the inventory
	if (InventoryWidget && InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromParent();
	}

	if (!BuildMenuWidget)
	{
		BuildMenuWidget = CreateWidget<UFactoryBuildMenuWidget>(this, BuildMenuWidgetClass);

		if (!BuildMenuWidget)
		{
			UE_LOG(LogFactoryGame, Error, TEXT("Could not spawn build menu widget."));
			return;
		}

		BuildMenuWidget->OnCloseRequested.BindUObject(this, &AFactoryGamePlayerController::CloseBuildMenu);
	}

	BuildMenuWidget->AddToPlayerScreen(50);

	// the game keeps running while the build menu is open (Docs/Standards/13-bauen.md)
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(BuildMenuWidget->TakeWidget());
	SetInputMode(InputMode);
	SetShowMouseCursor(true);

	BuildMenuWidget->FocusCurrentEntry();
}

void AFactoryGamePlayerController::CloseBuildMenu()
{
	if (BuildMenuWidget)
	{
		BuildMenuWidget->RemoveFromParent();
	}

	RestoreGameInput();
}

void AFactoryGamePlayerController::PickBuilding(FName BuildingId)
{
	CloseBuildMenu();

	// liegt das Gebäude auf der Hotbar, wird sein Slot mit ausgewählt, sonst keiner
	SelectedHotbarSlot = HotbarSlots.IndexOfByKey(BuildingId);

	if (BuildComponent)
	{
		BuildComponent->BeginPlacing(BuildingId);
	}

	if (HUDWidget)
	{
		HUDWidget->RebuildMenu();
	}
}

int32 AFactoryGamePlayerController::GiveItems(FName ItemId, int32 Count)
{
	const AFactoryGameCharacter* FactoryCharacter = Cast<AFactoryGameCharacter>(GetPawn());
	UFactoryInventoryComponent* Inventory = FactoryCharacter ? FactoryCharacter->GetInventory() : nullptr;

	if (!Inventory || Count <= 0)
	{
		return 0;
	}

	// Zentrale Stelle für "Spieler bekommt Items" (Cheats, später Abbau und Kisten):
	// legt ins Inventar und zeigt "+N Item" bzw. "Inventar voll" im HUD.
	const int32 Added = Inventory->AddItem(ItemId, Count);

	if (HUDWidget)
	{
		if (Added > 0)
		{
			HUDWidget->ShowItemNotification(ItemId, Added);
		}

		if (Added < Count)
		{
			HUDWidget->ShowNotification(FactoryLoc::Get("Notify_InventoryFull"), EFactoryNotificationType::Warning);
		}
	}

	return Added;
}

void AFactoryGamePlayerController::SetHotbarSlot(int32 Index, FName Shortcut)
{
	if (!HotbarSlots.IsValidIndex(Index))
	{
		return;
	}

	HotbarSlots[Index] = Shortcut;

	if (HUDWidget)
	{
		HUDWidget->RebuildMenu();
	}
}

void AFactoryGamePlayerController::SetHotbarSlots(const TArray<FName>& InSlots)
{
	HotbarSlots = InSlots;
	HotbarSlots.SetNum(NumHotbarSlots);
	SelectedHotbarSlot = INDEX_NONE;

	if (HUDWidget)
	{
		HUDWidget->RebuildMenu();
	}
}

void AFactoryGamePlayerController::SelectHotbarSlot(int32 Index)
{
	if (!HotbarSlots.IsValidIndex(Index))
	{
		return;
	}

	// erneutes Drücken derselben Taste wählt den Slot wieder ab
	SelectedHotbarSlot = SelectedHotbarSlot == Index ? INDEX_NONE : Index;

	// ein Gebäude auf dem Slot startet das Platzieren, abwählen beendet es
	if (BuildComponent)
	{
		if (SelectedHotbarSlot != INDEX_NONE && !HotbarSlots[SelectedHotbarSlot].IsNone())
		{
			BuildComponent->BeginPlacing(HotbarSlots[SelectedHotbarSlot]);
		}
		else if (BuildComponent->GetMode() != EFactoryBuildMode::None && BuildComponent->GetMode() != EFactoryBuildMode::Demolish)
		{
			BuildComponent->StopBuilding();
		}
	}

	if (!HUDWidget)
	{
		return;
	}

	// Hinweis für leere Slots: sie werden im Baumenü belegt.
	// (Index + 1) % 10 ergibt die Tastenbeschriftung: Slot 10 liegt auf der 0.
	if (SelectedHotbarSlot != INDEX_NONE && HotbarSlots[SelectedHotbarSlot].IsNone())
	{
		HUDWidget->ShowHint(FactoryLoc::Format("Hotbar_Empty", { FText::AsNumber((SelectedHotbarSlot + 1) % 10) }));
	}

	HUDWidget->RebuildMenu();
}

void AFactoryGamePlayerController::HandleHotbarKey(FKey Key)
{
	static const FKey NumberKeys[] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine, EKeys::Zero };

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(NumberKeys); ++Index)
	{
		if (NumberKeys[Index] == Key)
		{
			SelectHotbarSlot(Index);
			return;
		}
	}
}

void AFactoryGamePlayerController::SelectPreviousHotbarSlot()
{
	// D-Pad links/rechts läuft zyklisch durch die Slots; ohne Auswahl beginnt es am Ende bzw. Anfang
	SelectHotbarSlot(SelectedHotbarSlot == INDEX_NONE ? NumHotbarSlots - 1 : (SelectedHotbarSlot + NumHotbarSlots - 1) % NumHotbarSlots);
}

void AFactoryGamePlayerController::SelectNextHotbarSlot()
{
	SelectHotbarSlot(SelectedHotbarSlot == INDEX_NONE ? 0 : (SelectedHotbarSlot + 1) % NumHotbarSlots);
}

void AFactoryGamePlayerController::HandlePauseKey()
{
	// Esc bricht zuerst das Bauen ab, erst danach öffnet es das Pausemenü
	if (BuildComponent && BuildComponent->GetMode() != EFactoryBuildMode::None && !(PauseMenuWidget && PauseMenuWidget->IsInViewport()))
	{
		HandleCancel();
		return;
	}

	TogglePauseMenu();
}

void AFactoryGamePlayerController::HandlePrimaryPressed()
{
	if (BuildComponent)
	{
		BuildComponent->PrimaryPressed();
	}
}

void AFactoryGamePlayerController::HandlePrimaryReleased()
{
	if (BuildComponent)
	{
		BuildComponent->PrimaryReleased();
	}
}

void AFactoryGamePlayerController::HandleCancel()
{
	// beim Band geht Rechtsklick erst einen Schritt zurück (Start verwerfen), der Slot bleibt dann gewählt
	if (BuildComponent && !BuildComponent->Cancel())
	{
		return;
	}

	// der Hotbar-Slot gehört zum Platzieren, also mit abwählen
	if (SelectedHotbarSlot != INDEX_NONE)
	{
		SelectedHotbarSlot = INDEX_NONE;

		if (HUDWidget)
		{
			HUDWidget->RebuildMenu();
		}
	}
}

void AFactoryGamePlayerController::HandleRotateLeft()
{
	if (BuildComponent)
	{
		BuildComponent->Scroll(-1, IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl));
	}
}

void AFactoryGamePlayerController::HandleRotateRight()
{
	if (BuildComponent)
	{
		BuildComponent->Scroll(1, IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl));
	}
}

void AFactoryGamePlayerController::HandleToggleDemolish()
{
	if (!BuildComponent)
	{
		return;
	}

	// der Abriss-Modus ersetzt das Platzieren, der Hotbar-Slot wird dabei abgewählt
	const bool bWasDemolishing = BuildComponent->GetMode() == EFactoryBuildMode::Demolish;
	BuildComponent->StopBuilding();
	HandleCancel();

	if (!bWasDemolishing)
	{
		BuildComponent->ToggleDemolish();
	}
}

void AFactoryGamePlayerController::HandleCycleMode()
{
	if (BuildComponent)
	{
		BuildComponent->CycleMode();
	}
}

void AFactoryGamePlayerController::HandleInteract()
{
	if (BuildComponent)
	{
		BuildComponent->Interact();
	}
}

void AFactoryGamePlayerController::FillDefaultHotbar()
{
	// Startbelegung für neue Welten: die ersten Gebäude der Tabelle. Danach belegt der Spieler die Hotbar
	// im Baumenü (Q) selbst, eine belegte Hotbar wird nicht mehr angefasst
	const bool bHotbarEmpty = !HotbarSlots.ContainsByPredicate([](const FName& Shortcut) { return !Shortcut.IsNone(); });

	if (const UFactoryBuildingDatabase* Buildings = bHotbarEmpty ? UFactoryBuildingDatabase::Get(this) : nullptr)
	{
		for (const FName& Id : Buildings->GetAllBuildingIds())
		{
			const int32 FreeSlot = HotbarSlots.IndexOfByKey(NAME_None);

			if (FreeSlot == INDEX_NONE)
			{
				break;
			}

			if (!HotbarSlots.Contains(Id))
			{
				HotbarSlots[FreeSlot] = Id;
			}
		}
	}

	if (HUDWidget)
	{
		HUDWidget->RebuildMenu();
	}
}
