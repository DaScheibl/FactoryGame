// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryCheatManager.h"
#include "FactoryGamePlayerController.h"
#include "FactoryInventoryComponent.h"
#include "FactoryGameCharacter.h"
#include "FactoryItemDatabase.h"
#include "FactoryRecipeDatabase.h"
#include "FactoryMachine.h"
#include "FactoryBelt.h"
#include "FactoryBeltComponent.h"
#include "FactoryBuildingDatabase.h"
#include "FactorySimulationSubsystem.h"
#include "FactoryGame.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UFactoryInventoryComponent* UFactoryCheatManager::GetPlayerInventory() const
{
	const APlayerController* PlayerController = GetPlayerController();
	const AFactoryGameCharacter* Character = PlayerController ? Cast<AFactoryGameCharacter>(PlayerController->GetPawn()) : nullptr;
	return Character ? Character->GetInventory() : nullptr;
}

void UFactoryCheatManager::GiveItem(FName ItemId, int32 Count)
{
	AFactoryGamePlayerController* PlayerController = Cast<AFactoryGamePlayerController>(GetPlayerController());
	const UFactoryItemDatabase* Database = UFactoryItemDatabase::Get(this);

	if (!PlayerController || !Database)
	{
		return;
	}

	// Kurzform erlauben: "IronPlate" wird zu "ITM_IronPlate"
	if (!Database->FindItem(ItemId))
	{
		ItemId = FName(TEXT("ITM_") + ItemId.ToString());
	}

	if (!Database->FindItem(ItemId))
	{
		UE_LOG(LogFactoryGame, Warning, TEXT("GiveItem: unknown item '%s'. Use ListItems to see all ids."), *ItemId.ToString());
		return;
	}

	PlayerController->GiveItems(ItemId, Count);
}

void UFactoryCheatManager::GiveAllItems(int32 MaxTier)
{
	AFactoryGamePlayerController* PlayerController = Cast<AFactoryGamePlayerController>(GetPlayerController());
	const UFactoryItemDatabase* Database = UFactoryItemDatabase::Get(this);

	if (!PlayerController || !Database)
	{
		return;
	}

	for (const FName& ItemId : Database->GetAllItemIds())
	{
		const FFactoryItemDefinition* Item = Database->FindItem(ItemId);

		// Fluide (Stackgröße 0) lassen sich nicht tragen und werden übersprungen
		if (Item->Tier <= MaxTier && Item->GetMaxStackSize() > 0)
		{
			PlayerController->GiveItems(ItemId, Item->GetMaxStackSize());
		}
	}
}

void UFactoryCheatManager::ClearInventory()
{
	if (UFactoryInventoryComponent* Inventory = GetPlayerInventory())
	{
		Inventory->Clear();
	}
}

void UFactoryCheatManager::SetInventorySlots(int32 NumSlots)
{
	if (UFactoryInventoryComponent* Inventory = GetPlayerInventory())
	{
		for (const FFactoryItemStack& Lost : Inventory->SetNumSlots(NumSlots))
		{
			UE_LOG(LogFactoryGame, Warning, TEXT("SetInventorySlots: no space left for %d x %s, removed."), Lost.Count, *Lost.ItemId.ToString());
		}
	}
}

void UFactoryCheatManager::UpgradeInventory()
{
	if (UFactoryInventoryComponent* Inventory = GetPlayerInventory())
	{
		const int32 NumSlots = FMath::Min(Inventory->GetNumSlots() + UFactoryInventoryComponent::PlayerSlotsPerUpgrade, UFactoryInventoryComponent::PlayerMaxSlots);
		Inventory->SetNumSlots(NumSlots);
		UE_LOG(LogFactoryGame, Display, TEXT("UpgradeInventory: %d slots."), NumSlots);
	}
}

void UFactoryCheatManager::ListItems()
{
	if (const UFactoryItemDatabase* Database = UFactoryItemDatabase::Get(this))
	{
		for (const FName& ItemId : Database->GetAllItemIds())
		{
			const FFactoryItemDefinition* Item = Database->FindItem(ItemId);
			UE_LOG(LogFactoryGame, Display, TEXT("T%d  %-32s %-28s stack %d"), Item->Tier, *ItemId.ToString(), *Item->GetDisplayName().ToString(), Item->GetMaxStackSize());
		}
	}
}

void UFactoryCheatManager::ReloadItems()
{
	if (UFactoryItemDatabase* Database = UFactoryItemDatabase::Get(this))
	{
		Database->Reload();
	}

	// umbenannte oder gelöschte Items können Rezepte ungültig machen
	ReloadRecipes();
}

void UFactoryCheatManager::ListRecipes(FName Filter)
{
	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);

	if (!Recipes || !Items)
	{
		return;
	}

	// Kurzformen wie bei GiveItem: "Constructor" -> BLD_Constructor, "IronPlate" -> ITM_IronPlate
	TArray<FName> RecipeIds = Recipes->GetAllRecipeIds();

	if (!Filter.IsNone())
	{
		const FString Name = Filter.ToString();
		const FName BuildingId = Name.StartsWith(TEXT("BLD_")) ? Filter : FName(TEXT("BLD_") + Name);
		const FName ItemId = Items->FindItem(Filter) ? Filter : FName(TEXT("ITM_") + Name);

		RecipeIds = Recipes->GetRecipesForBuilding(BuildingId);

		if (RecipeIds.IsEmpty())
		{
			RecipeIds = Recipes->GetRecipesProducing(ItemId);
			RecipeIds.Append(Recipes->GetRecipesUsing(ItemId));
		}
	}

	auto Describe = [](const FFactoryRecipeDefinition& Recipe, const TArray<FFactoryItemStack>& Stacks)
	{
		TArray<FString> Parts;

		for (const FFactoryItemStack& Stack : Stacks)
		{
			Parts.Add(FString::Printf(TEXT("%d %s (%.1f/min)"), Stack.Count, *Stack.ItemId.ToString(), Recipe.GetPerMinute(Stack.Count)));
		}

		return FString::Join(Parts, TEXT(" + "));
	};

	for (const FName& RecipeId : RecipeIds)
	{
		const FFactoryRecipeDefinition* Recipe = Recipes->FindRecipe(RecipeId);
		UE_LOG(LogFactoryGame, Display, TEXT("T%d  %-26s %-18s %5.1f s%s  %s -> %s"), Recipe->Tier, *RecipeId.ToString(), *Recipe->Building.ToString(), Recipe->Duration,
			Recipe->Workbench ? TEXT(" [WB]") : TEXT("     "), *Describe(*Recipe, Recipe->GetInputs()), *Describe(*Recipe, Recipe->GetOutputs()));
	}

	UE_LOG(LogFactoryGame, Display, TEXT("%d recipes."), RecipeIds.Num());
}

void UFactoryCheatManager::ReloadRecipes()
{
	if (UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this))
	{
		Recipes->Reload();
	}
}

void UFactoryCheatManager::SetHotbar(int32 SlotNumber, FName Shortcut)
{
	if (AFactoryGamePlayerController* PlayerController = Cast<AFactoryGamePlayerController>(GetPlayerController()))
	{
		// die Konsole zählt ab 1 wie die Tasten, intern beginnen die Slots bei 0
		PlayerController->SetHotbarSlot(SlotNumber - 1, Shortcut);
	}
}

FName UFactoryCheatManager::WithPrefix(FName Id, const TCHAR* Prefix)
{
	return Id.IsNone() || Id.ToString().StartsWith(Prefix) ? Id : FName(Prefix + Id.ToString());
}

AFactoryMachine* UFactoryCheatManager::GetTargetMachine() const
{
	const APlayerController* PlayerController = GetPlayerController();
	UWorld* World = GetWorld();

	if (!PlayerController || !World)
	{
		return nullptr;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	// zuerst die Maschine unter dem Fadenkreuz (bis 15 m) ...
	FHitResult Hit;
	FCollisionQueryParams Params(TEXT("CheatMachineTrace"), false, PlayerController->GetPawn());

	if (World->LineTraceSingleByChannel(Hit, ViewLocation, ViewLocation + ViewRotation.Vector() * 1500.0, ECC_Visibility, Params))
	{
		if (AFactoryMachine* Machine = Cast<AFactoryMachine>(Hit.GetActor()))
		{
			return Machine;
		}
	}

	// ... sonst die nächste im Umkreis von 20 m
	AFactoryMachine* Nearest = nullptr;
	double NearestDistSq = FMath::Square(2000.0);

	for (TActorIterator<AFactoryMachine> It(World); It; ++It)
	{
		const double DistSq = FVector::DistSquared(It->GetActorLocation(), ViewLocation);

		if (DistSq < NearestDistSq)
		{
			Nearest = *It;
			NearestDistSq = DistSq;
		}
	}

	if (!Nearest)
	{
		UE_LOG(LogFactoryGame, Warning, TEXT("No machine in view or within 20 m."));
	}

	return Nearest;
}

bool UFactoryCheatManager::GetSpawnPoint(double Distance, FVector& OutLocation, FRotator& OutFacing) const
{
	const APlayerController* PlayerController = GetPlayerController();

	if (!PlayerController)
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	// vor dem Spieler, auf das 1-m-Raster gerastet und um 90° gedreht (Docs/Standards/03-raster.md)
	OutFacing = FRotator(0.0, FMath::GridSnap(ViewRotation.Yaw, 90.0), 0.0);
	OutLocation = ViewLocation + FRotator(0.0, ViewRotation.Yaw, 0.0).Vector() * Distance;
	OutLocation.X = FMath::GridSnap(OutLocation.X, 100.0);
	OutLocation.Y = FMath::GridSnap(OutLocation.Y, 100.0);
	return true;
}

AFactoryMachine* UFactoryCheatManager::SpawnMachineAt(FVector Location, const FRotator& Facing, FName BuildingId, FName RecipeId)
{
	const APlayerController* PlayerController = GetPlayerController();
	UWorld* World = GetWorld();

	if (!World || BuildingId.IsNone())
	{
		return nullptr;
	}

	// auf den Boden setzen
	FHitResult Hit;
	FCollisionQueryParams Params(TEXT("CheatSpawnTrace"), false, PlayerController ? PlayerController->GetPawn() : nullptr);

	if (World->LineTraceSingleByChannel(Hit, Location + FVector(0.0, 0.0, 1000.0), Location - FVector(0.0, 0.0, 5000.0), ECC_Visibility, Params))
	{
		Location.Z = Hit.ImpactPoint.Z;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFactoryMachine* Machine = World->SpawnActor<AFactoryMachine>(Location, Facing, SpawnParams);

	if (!Machine)
	{
		return nullptr;
	}

	// Größe und Ports aus DT_Buildings; unbekannte Gebäude bleiben ein 4-m-Würfel
	Machine->InitBuilding(WithPrefix(BuildingId, TEXT("BLD_")));

	if (!RecipeId.IsNone())
	{
		TArray<FFactoryItemStack> Ignored;
		Machine->GetProduction()->SetRecipe(WithPrefix(RecipeId, TEXT("RCP_")), Ignored);
	}

	UE_LOG(LogFactoryGame, Display, TEXT("SpawnMachine: %s at %s."), *Machine->GetProduction()->GetBuildingId().ToString(), *Location.ToString());
	return Machine;
}

void UFactoryCheatManager::ListBuildings()
{
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);

	if (!Buildings)
	{
		return;
	}

	for (const FName& Id : Buildings->GetAllBuildingIds())
	{
		const FFactoryBuildingDefinition* Building = Buildings->FindBuilding(Id);
		TArray<FString> Costs;

		for (const FFactoryItemStack& Cost : Building->GetCosts())
		{
			Costs.Add(FString::Printf(TEXT("%d %s"), Cost.Count, *Cost.ItemId.ToString()));
		}

		UE_LOG(LogFactoryGame, Display, TEXT("T%d  %-20s %dx%dx%d m  in %d  out %d  cost: %s"), Building->Tier, *Id.ToString(),
			Building->SizeX, Building->SizeY, Building->SizeZ, Building->Inputs, Building->Outputs, *FString::Join(Costs, TEXT(", ")));
	}
}

void UFactoryCheatManager::ReloadBuildings()
{
	if (UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this))
	{
		const int32 NumProblems = Buildings->Reload();
		UE_LOG(LogFactoryGame, Display, TEXT("ReloadBuildings: %d buildings, %d problems."), Buildings->GetAllBuildingIds().Num(), NumProblems);
	}
}

void UFactoryCheatManager::GiveBuildCosts(FName BuildingId, int32 Times)
{
	AFactoryGamePlayerController* PlayerController = Cast<AFactoryGamePlayerController>(GetPlayerController());
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(WithPrefix(BuildingId, TEXT("BLD_"))) : nullptr;

	if (!PlayerController || !Building)
	{
		UE_LOG(LogFactoryGame, Warning, TEXT("GiveBuildCosts: unknown building '%s'. Use ListBuildings to see all ids."), *BuildingId.ToString());
		return;
	}

	for (const FFactoryItemStack& Cost : Building->GetCosts())
	{
		PlayerController->GiveItems(Cost.ItemId, Cost.Count * FMath::Max(1, Times));
	}
}

void UFactoryCheatManager::SpawnMachine(FName BuildingId, FName RecipeId)
{
	FVector Location;
	FRotator Facing;

	// 8 m vor dem Spieler
	if (GetSpawnPoint(800.0, Location, Facing))
	{
		SpawnMachineAt(Location, Facing, BuildingId, RecipeId);
	}
}

void UFactoryCheatManager::SetMachineRecipe(FName RecipeId)
{
	AFactoryMachine* Machine = GetTargetMachine();
	AFactoryGamePlayerController* PlayerController = Cast<AFactoryGamePlayerController>(GetPlayerController());

	if (!Machine)
	{
		return;
	}

	TArray<FFactoryItemStack> Returned;

	if (Machine->GetProduction()->SetRecipe(WithPrefix(RecipeId, TEXT("RCP_")), Returned) && PlayerController)
	{
		for (const FFactoryItemStack& Stack : Returned)
		{
			PlayerController->GiveItems(Stack.ItemId, Stack.Count);
		}
	}
}

void UFactoryCheatManager::FeedMachine(FName ItemId, int32 Count)
{
	AFactoryMachine* Machine = GetTargetMachine();

	if (!Machine)
	{
		return;
	}

	UFactoryProductionComponent* Production = Machine->GetProduction();

	// ohne Angabe: alle Eingänge bis zum vollen Stapel auffüllen
	if (ItemId.IsNone())
	{
		for (const FFactoryItemStack& Input : TArray<FFactoryItemStack>(Production->GetInputs()))
		{
			Production->InsertInput(Input.ItemId, Production->GetInputSpace(Input.ItemId));
		}

		return;
	}

	ItemId = WithPrefix(ItemId, TEXT("ITM_"));
	const int32 Inserted = Production->InsertInput(ItemId, Count > 0 ? Count : Production->GetInputSpace(ItemId));
	UE_LOG(LogFactoryGame, Display, TEXT("FeedMachine: %d x %s inserted."), Inserted, *ItemId.ToString());
}

void UFactoryCheatManager::TakeOutput()
{
	AFactoryMachine* Machine = GetTargetMachine();
	AFactoryGamePlayerController* PlayerController = Cast<AFactoryGamePlayerController>(GetPlayerController());
	const UFactoryInventoryComponent* Inventory = GetPlayerInventory();

	if (!Machine || !PlayerController || !Inventory)
	{
		return;
	}

	UFactoryProductionComponent* Production = Machine->GetProduction();

	// nur so viel entnehmen, wie ins Inventar passt, damit nichts verloren geht
	for (const FFactoryItemStack& Output : TArray<FFactoryItemStack>(Production->GetOutputs()))
	{
		const FFactoryItemStack Taken = Production->ExtractOutput(Output.ItemId, Inventory->GetFreeSpaceFor(Output.ItemId));

		if (!Taken.IsEmpty())
		{
			PlayerController->GiveItems(Taken.ItemId, Taken.Count);
		}
	}
}

void UFactoryCheatManager::MachineInfo()
{
	const AFactoryMachine* Machine = GetTargetMachine();

	if (!Machine)
	{
		return;
	}

	const UFactoryProductionComponent* Production = Machine->GetProduction();

	UE_LOG(LogFactoryGame, Display, TEXT("%s  building %s  recipe %s  state %s  progress %.0f %%  speed %d %%  cycles %lld"),
		*Machine->GetName(), *Production->GetBuildingId().ToString(), *Production->GetRecipeId().ToString(),
		*UEnum::GetValueAsString(Production->GetState()), Production->GetProgress() * 100.0f, Production->GetSpeedPercent(), Production->GetCyclesCompleted());

	for (const FFactoryItemStack& Input : Production->GetInputs())
	{
		UE_LOG(LogFactoryGame, Display, TEXT("  in   %4d %s"), Input.Count, *Input.ItemId.ToString());
	}

	for (const FFactoryItemStack& Output : Production->GetOutputs())
	{
		UE_LOG(LogFactoryGame, Display, TEXT("  out  %4d %s"), Output.Count, *Output.ItemId.ToString());
	}
}

void UFactoryCheatManager::DestroyMachine()
{
	if (AFactoryMachine* Machine = GetTargetMachine())
	{
		Machine->Destroy();
	}
}

void UFactoryCheatManager::SimStats()
{
	if (const UFactorySimulationSubsystem* Simulation = UFactorySimulationSubsystem::Get(this))
	{
		UE_LOG(LogFactoryGame, Display, TEXT("Simulation: tick %lld (%d/s), %d objects, %.3f ms per tick, %lld ticks dropped, speed x%.2f"),
			Simulation->GetTickCount(), UFactorySimulationSubsystem::TicksPerSecond, Simulation->GetNumSimulated(),
			Simulation->GetAverageTickMs(), Simulation->GetDroppedTicks(), Simulation->GetSpeed());
	}
}

void UFactoryCheatManager::SimSpeed(float Speed)
{
	if (UFactorySimulationSubsystem* Simulation = UFactorySimulationSubsystem::Get(this))
	{
		Simulation->SetSpeed(Speed);
		UE_LOG(LogFactoryGame, Display, TEXT("SimSpeed: x%.2f"), Simulation->GetSpeed());
	}
}

AFactoryBelt* UFactoryCheatManager::GetTargetBelt() const
{
	const APlayerController* PlayerController = GetPlayerController();
	UWorld* World = GetWorld();

	if (!PlayerController || !World)
	{
		return nullptr;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	// zuerst das Band unter dem Fadenkreuz (bis 15 m) ...
	FHitResult Hit;
	FCollisionQueryParams Params(TEXT("CheatBeltTrace"), false, PlayerController->GetPawn());

	if (World->LineTraceSingleByChannel(Hit, ViewLocation, ViewLocation + ViewRotation.Vector() * 1500.0, ECC_Visibility, Params))
	{
		if (AFactoryBelt* Belt = Cast<AFactoryBelt>(Hit.GetActor()))
		{
			return Belt;
		}
	}

	// ... sonst das nächste im Umkreis von 20 m (gemessen zur Bandlinie, nicht zum Anfang)
	AFactoryBelt* Nearest = nullptr;
	double NearestDistSq = FMath::Square(2000.0);

	for (TActorIterator<AFactoryBelt> It(World); It; ++It)
	{
		// Abstand zur Kurve, grob über die Spline-Punkte und ihre Verbindungslinien
		const TArray<FFactoryBeltPathPoint>& Points = It->GetPath().Points;
		double DistSq = TNumericLimits<double>::Max();

		for (int32 Index = 1; Index < Points.Num(); ++Index)
		{
			DistSq = FMath::Min(DistSq, FVector::DistSquared(FMath::ClosestPointOnSegment(ViewLocation, Points[Index - 1].Location, Points[Index].Location), ViewLocation));
		}

		if (DistSq < NearestDistSq)
		{
			Nearest = *It;
			NearestDistSq = DistSq;
		}
	}

	if (!Nearest)
	{
		UE_LOG(LogFactoryGame, Warning, TEXT("No belt in view or within 20 m."));
	}

	return Nearest;
}

void UFactoryCheatManager::ConnectBelt(int32 Tier)
{
	AFactoryMachine* Machine = GetTargetMachine();

	if (!Machine)
	{
		return;
	}

	// erster Aufruf: Quelle merken
	if (!PendingBeltSource.IsValid())
	{
		PendingBeltSource = Machine;
		PendingBeltTier = Tier;
		UE_LOG(LogFactoryGame, Display, TEXT("ConnectBelt: source %s. Now look at the target machine and enter ConnectBelt again."), *Machine->GetName());
		return;
	}

	AFactoryMachine* Source = PendingBeltSource.Get();
	PendingBeltSource.Reset();

	if (Source == Machine)
	{
		UE_LOG(LogFactoryGame, Display, TEXT("ConnectBelt: same machine twice, cancelled."));
		return;
	}

	// Stufe aus dem zweiten Aufruf, sonst aus dem ersten, sonst Mk1
	const int32 UsedTier = Tier > 0 ? Tier : (PendingBeltTier > 0 ? PendingBeltTier : 1);
	const AFactoryBelt* Belt = AFactoryBelt::SpawnBetween(GetWorld(), Source, Machine, UsedTier);

	if (!Belt)
	{
		UE_LOG(LogFactoryGame, Warning, TEXT("ConnectBelt: no free ports, or no belt shape fits between them (try placing them in a row)."));
		return;
	}

	UE_LOG(LogFactoryGame, Display, TEXT("ConnectBelt: %s -> %s, Mk%d, %.1f m, %d items/min."), *Source->GetName(), *Machine->GetName(),
		Belt->GetBelt()->GetTier(), Belt->GetBelt()->GetLength() / 1000.0, UFactoryBeltComponent::GetItemsPerMinute(Belt->GetBelt()->GetTier()));
}

void UFactoryCheatManager::BeltTest(int32 Tier)
{
	FVector Location;
	FRotator Facing;

	if (!GetSpawnPoint(800.0, Location, Facing))
	{
		return;
	}

	// hintereinander: Ausgang des Schmelzofens (vorn, +2 m) zeigt auf den Eingang des Konstruktors
	// (hinten, 14 m - 4 m = +10 m), dazwischen 8 m gerades Band
	const FVector Forward = Facing.Vector();
	AFactoryMachine* Smelter = SpawnMachineAt(Location, Facing, TEXT("Smelter"), TEXT("IronIngot"));
	AFactoryMachine* Constructor = SpawnMachineAt(Location + Forward * 1400.0, Facing, TEXT("Constructor"), TEXT("IronPlate"));

	if (!Smelter || !Constructor)
	{
		return;
	}

	UFactoryProductionComponent* Production = Smelter->GetProduction();
	Production->InsertInput(TEXT("ITM_IronOre"), Production->GetInputSpace(TEXT("ITM_IronOre")));
	AFactoryBelt::SpawnBetween(GetWorld(), Smelter, Constructor, Tier);
	UE_LOG(LogFactoryGame, Display, TEXT("BeltTest: smelter (full of iron ore) -> Mk%d belt -> constructor."), FMath::Clamp(Tier, 1, UFactoryBeltComponent::MaxTier));
}

void UFactoryCheatManager::FeedBelt(FName ItemId, int32 Count)
{
	AFactoryBelt* Belt = GetTargetBelt();
	const UFactoryItemDatabase* Database = UFactoryItemDatabase::Get(this);
	ItemId = WithPrefix(ItemId, TEXT("ITM_"));

	if (!Belt || !Database)
	{
		return;
	}

	if (!Database->FindItem(ItemId))
	{
		UE_LOG(LogFactoryGame, Warning, TEXT("FeedBelt: unknown item '%s'. Use ListItems to see all ids."), *ItemId.ToString());
		return;
	}

	// mit riesigem Offset landet jedes Item direkt hinter dem letzten, das Band füllt sich also von vorn
	const int32 MaxCount = Count > 0 ? Count : MAX_int32;
	int32 Added = 0;

	while (Added < MaxCount && Belt->GetBelt()->ReceiveItem(ItemId, MAX_int32))
	{
		++Added;
	}

	UE_LOG(LogFactoryGame, Display, TEXT("FeedBelt: %d x %s added."), Added, *ItemId.ToString());
}

void UFactoryCheatManager::SetBeltTier(int32 Tier)
{
	if (AFactoryBelt* Belt = GetTargetBelt())
	{
		Belt->GetBelt()->SetTier(Tier);
		UE_LOG(LogFactoryGame, Display, TEXT("SetBeltTier: Mk%d, %d items/min."), Belt->GetBelt()->GetTier(), UFactoryBeltComponent::GetItemsPerMinute(Belt->GetBelt()->GetTier()));
	}
}

void UFactoryCheatManager::BeltInfo()
{
	const AFactoryBelt* Belt = GetTargetBelt();

	if (!Belt)
	{
		return;
	}

	const UFactoryBeltComponent* Component = Belt->GetBelt();
	const UFactoryProductionComponent* Source = Component->GetSource();
	const UObject* Target = Component->GetTarget();

	UE_LOG(LogFactoryGame, Display, TEXT("%s  Mk%d  %d mm/tick  %d items/min  length %.1f m  %d items  %s  delivered %lld"),
		*Belt->GetName(), Component->GetTier(), Component->GetSpeed(), UFactoryBeltComponent::GetItemsPerMinute(Component->GetTier()),
		Component->GetLength() / 1000.0, Component->GetItems().Num(), Component->IsBlocked() ? TEXT("JAMMED") : TEXT("running"), Component->GetItemsDelivered());

	UE_LOG(LogFactoryGame, Display, TEXT("  from %s  to %s"),
		Source ? *GetNameSafe(Source->GetOwner()) : TEXT("-"),
		Target ? *GetNameSafe(Target->GetTypedOuter<AActor>()) : TEXT("-"));

	// Anzahl pro Sorte
	TMap<FName, int32> Counts;

	for (const FFactoryBeltItem& Item : Component->GetItems())
	{
		++Counts.FindOrAdd(Item.ItemId);
	}

	for (const TPair<FName, int32>& Pair : Counts)
	{
		UE_LOG(LogFactoryGame, Display, TEXT("  %4d %s"), Pair.Value, *Pair.Key.ToString());
	}
}

void UFactoryCheatManager::DestroyBelt()
{
	AFactoryBelt* Belt = GetTargetBelt();
	AFactoryGamePlayerController* PlayerController = Cast<AFactoryGamePlayerController>(GetPlayerController());

	if (!Belt)
	{
		return;
	}

	// Items gehen ins Inventar (was nicht passt, ist weg; ist ja nur ein Test-Befehl)
	TArray<FName> Removed;
	Belt->GetBelt()->ClearItems(Removed);

	if (PlayerController)
	{
		TMap<FName, int32> Counts;

		for (const FName& ItemId : Removed)
		{
			++Counts.FindOrAdd(ItemId);
		}

		for (const TPair<FName, int32>& Pair : Counts)
		{
			PlayerController->GiveItems(Pair.Key, Pair.Value);
		}
	}

	Belt->Destroy();
}
