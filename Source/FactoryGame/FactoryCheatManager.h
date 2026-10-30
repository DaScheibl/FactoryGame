// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "FactoryCheatManager.generated.h"

class UFactoryInventoryComponent;
class AFactoryMachine;
class AFactoryBelt;

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

	/** Prints all buildings with size, ports and costs to the log */
	UFUNCTION(Exec)
	void ListBuildings();

	/** Reloads the building table from disk and checks it */
	UFUNCTION(Exec)
	void ReloadBuildings();

	/** Gives the player the materials for a building, optionally several times: "GiveBuildCosts Smelter 3" */
	UFUNCTION(Exec)
	void GiveBuildCosts(FName BuildingId, int32 Times = 1);

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

	/**
	 *  Connects two machines with a graybox belt. First call: machine in view is the source,
	 *  second call: machine in view is the target. Tier 1-5 (Mk1-Mk5), "ConnectBelt 3"
	 */
	UFUNCTION(Exec)
	void ConnectBelt(int32 Tier = 0);

	/** Spawns a smelter and a constructor connected by a belt, ready to run: "BeltTest 2" */
	UFUNCTION(Exec)
	void BeltTest(int32 Tier = 1);

	/** Puts items onto the belt in view, packed from the end. Without count: as many as fit */
	UFUNCTION(Exec)
	void FeedBelt(FName ItemId, int32 Count = 0);

	/** Changes the tier of the belt in view (1-5) */
	UFUNCTION(Exec)
	void SetBeltTier(int32 Tier);

	/** Sets the unlocked tier of the world (0-8), without delivering anything */
	UFUNCTION(Exec)
	void UnlockTier(int32 Tier);

	/** Spawns a resource node 8 m in front of the player. Purity: Impure, Normal (default) or Pure */
	UFUNCTION(Exec)
	void SpawnNode(FName ItemId, FName Purity);

	/** Every machine has power without cables or generators (1 = on, 0 = off) */
	UFUNCTION(Exec)
	void FreePower(int32 bEnabled = 1);

	/** Prints all power networks to the log */
	UFUNCTION(Exec)
	void PowerInfo();

	/** Prints the state of the belt in view to the log */
	UFUNCTION(Exec)
	void BeltInfo();

	/** Removes the belt in view. The items on it go to the player inventory */
	UFUNCTION(Exec)
	void DestroyBelt();

private:

	UFactoryInventoryComponent* GetPlayerInventory() const;

	/** Returns the machine under the crosshair, or else the nearest machine within 20 m */
	AFactoryMachine* GetTargetMachine() const;

	/** Returns the belt under the crosshair, or else the nearest belt within 20 m */
	AFactoryBelt* GetTargetBelt() const;

	/** Returns a point the given distance in front of the player, snapped to the 1 m grid, and a facing in 90° steps */
	bool GetSpawnPoint(double Distance, FVector& OutLocation, FRotator& OutFacing) const;

	/** Spawns a graybox machine on the ground at the given location */
	AFactoryMachine* SpawnMachineAt(FVector Location, const FRotator& Facing, FName BuildingId, FName RecipeId);

	/** Source machine remembered by the first ConnectBelt call */
	TWeakObjectPtr<AFactoryMachine> PendingBeltSource;

	/** Tier given to the first ConnectBelt call */
	int32 PendingBeltTier = 0;

	/** Adds a prefix like ITM_ if the id doesn't have it yet */
	static FName WithPrefix(FName Id, const TCHAR* Prefix);
};
