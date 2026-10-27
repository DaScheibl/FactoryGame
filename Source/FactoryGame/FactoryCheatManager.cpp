// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryCheatManager.h"
#include "FactoryGamePlayerController.h"
#include "FactoryInventoryComponent.h"
#include "FactoryGameCharacter.h"
#include "FactoryItemDatabase.h"
#include "FactoryRecipeDatabase.h"
#include "FactoryGame.h"
#include "GameFramework/Pawn.h"

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
