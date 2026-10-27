// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "FactoryCheatManager.generated.h"

class UFactoryInventoryComponent;

/**
 *  Console commands for testing. Available in the editor and in development builds.
 *
 *  Benutzung: im Spiel die Konsole mit ^ (bzw. `) öffnen und z. B. "GiveItem IronPlate 50" eingeben.
 *  Jede UFUNCTION(Exec) in dieser Klasse wird automatisch zum Konsolenbefehl.
 *  In Shipping-Builds gibt es keinen Cheat Manager, die Befehle fehlen dort also.
 */
UCLASS()
class FACTORYGAME_API UFactoryCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:

	/** Gives the player items. The ITM_ prefix is optional: "GiveItem IronPlate 50" */
	UFUNCTION(Exec)
	void GiveItem(FName ItemId, int32 Count = 1);

	/** Gives the player one stack of every carryable item up to the given tier */
	UFUNCTION(Exec)
	void GiveAllItems(int32 MaxTier = 8);

	/** Removes all items from the player inventory */
	UFUNCTION(Exec)
	void ClearInventory();

	/** Changes the number of inventory slots */
	UFUNCTION(Exec)
	void SetInventorySlots(int32 NumSlots);

	/** Adds one inventory upgrade step (later unlocked by research) */
	UFUNCTION(Exec)
	void UpgradeInventory();

	/** Prints all item ids to the log */
	UFUNCTION(Exec)
	void ListItems();

	/** Reloads the item table from disk */
	UFUNCTION(Exec)
	void ReloadItems();

	/** Puts a shortcut on a hotbar slot (1-10) to test the HUD */
	UFUNCTION(Exec)
	void SetHotbar(int32 SlotNumber, FName Shortcut);

private:

	UFactoryInventoryComponent* GetPlayerInventory() const;
};
