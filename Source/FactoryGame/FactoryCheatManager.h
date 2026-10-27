// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "FactoryCheatManager.generated.h"

class UFactoryInventoryComponent;
class AFactoryMachine;

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

	/** Reloads the item table from disk (and re-checks the recipes against it) */
	UFUNCTION(Exec)
	void ReloadItems();

	/** Prints all recipes to the log. Optional filter by building or item: "ListRecipes Constructor", "ListRecipes IronPlate" */
	UFUNCTION(Exec)
	void ListRecipes(FName Filter = NAME_None);

	/** Reloads the recipe table from disk and checks it against the item table */
	UFUNCTION(Exec)
	void ReloadRecipes();

	/** Puts a shortcut on a hotbar slot (1-10) to test the HUD */
	UFUNCTION(Exec)
	void SetHotbar(int32 SlotNumber, FName Shortcut);

	/** Spawns a graybox machine in front of the player. Prefixes are optional: "SpawnMachine Smelter IronIngot" */
	UFUNCTION(Exec)
	void SpawnMachine(FName BuildingId, FName RecipeId = NAME_None);

	/** Changes the recipe of the machine in view. Items inside go to the player inventory */
	UFUNCTION(Exec)
	void SetMachineRecipe(FName RecipeId);

	/** Puts items into the machine in view. Without arguments all input buffers are filled up */
	UFUNCTION(Exec)
	void FeedMachine(FName ItemId = NAME_None, int32 Count = 0);

	/** Moves the output of the machine in view into the player inventory */
	UFUNCTION(Exec)
	void TakeOutput();

	/** Prints the state of the machine in view to the log */
	UFUNCTION(Exec)
	void MachineInfo();

	/** Removes the machine in view */
	UFUNCTION(Exec)
	void DestroyMachine();

	/** Prints simulation statistics to the log */
	UFUNCTION(Exec)
	void SimStats();

	/** Speeds up the simulation for testing, 1 = real time */
	UFUNCTION(Exec)
	void SimSpeed(float Speed = 1.0f);

private:

	UFactoryInventoryComponent* GetPlayerInventory() const;

	/** Returns the machine under the crosshair, or else the nearest machine within 20 m */
	AFactoryMachine* GetTargetMachine() const;

	/** Adds a prefix like ITM_ if the id doesn't have it yet */
	static FName WithPrefix(FName Id, const TCHAR* Prefix);
};
