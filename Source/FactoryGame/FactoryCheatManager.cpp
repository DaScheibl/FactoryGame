// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryCheatManager.h"
#include "FactoryGamePlayerController.h"
#include "FactoryInventoryComponent.h"
#include "FactoryGameCharacter.h"
#include "FactoryItemDatabase.h"
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
}

void UFactoryCheatManager::SetHotbar(int32 SlotNumber, FName Shortcut)
{
	if (AFactoryGamePlayerController* PlayerController = Cast<AFactoryGamePlayerController>(GetPlayerController()))
	{
		// die Konsole zählt ab 1 wie die Tasten, intern beginnen die Slots bei 0
		PlayerController->SetHotbarSlot(SlotNumber - 1, Shortcut);
	}
}
