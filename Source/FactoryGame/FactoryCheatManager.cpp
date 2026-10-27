// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryCheatManager.h"
#include "FactoryGamePlayerController.h"
#include "FactoryInventoryComponent.h"
#include "FactoryGameCharacter.h"
#include "FactoryItemDatabase.h"
#include "FactoryRecipeDatabase.h"
#include "FactoryMachine.h"
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

void UFactoryCheatManager::SpawnMachine(FName BuildingId, FName RecipeId)
{
	const APlayerController* PlayerController = GetPlayerController();
	UWorld* World = GetWorld();

	if (!PlayerController || !World || BuildingId.IsNone())
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	// 8 m vor dem Spieler, auf das 1-m-Raster gerastet und um 90° gedreht (Docs/Standards/03-raster.md)
	const FRotator Facing(0.0, FMath::GridSnap(ViewRotation.Yaw, 90.0), 0.0);
	FVector Location = ViewLocation + FRotator(0.0, ViewRotation.Yaw, 0.0).Vector() * 800.0;
	Location.X = FMath::GridSnap(Location.X, 100.0);
	Location.Y = FMath::GridSnap(Location.Y, 100.0);

	// auf den Boden setzen
	FHitResult Hit;
	FCollisionQueryParams Params(TEXT("CheatSpawnTrace"), false, PlayerController->GetPawn());

	if (World->LineTraceSingleByChannel(Hit, Location + FVector(0.0, 0.0, 1000.0), Location - FVector(0.0, 0.0, 5000.0), ECC_Visibility, Params))
	{
		Location.Z = Hit.ImpactPoint.Z;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFactoryMachine* Machine = World->SpawnActor<AFactoryMachine>(Location, Facing, SpawnParams);

	if (!Machine)
	{
		return;
	}

	Machine->GetProduction()->SetBuildingId(WithPrefix(BuildingId, TEXT("BLD_")));

	if (!RecipeId.IsNone())
	{
		TArray<FFactoryItemStack> Ignored;
		Machine->GetProduction()->SetRecipe(WithPrefix(RecipeId, TEXT("RCP_")), Ignored);
	}

	UE_LOG(LogFactoryGame, Display, TEXT("SpawnMachine: %s at %s."), *Machine->GetProduction()->GetBuildingId().ToString(), *Location.ToString());
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
