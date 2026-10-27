// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FactoryItemTypes.h"
#include "FactoryWorldSaveGame.generated.h"

/**
 *  Summary of a saved world, shown in the load world list
 */
USTRUCT(BlueprintType)
struct FFactoryWorldInfo
{
	GENERATED_BODY()

	/** Save game slot holding the world data */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category="World")
	FString SlotName;

	/** Name chosen by the player */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category="World")
	FString DisplayName;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category="World")
	FDateTime CreatedAt;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category="World")
	FDateTime LastPlayedAt;

	/** Total play time in seconds */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category="World")
	double PlayTimeSeconds = 0.0;
};

/**
 *  List of all saved worlds plus the last played one.
 *  Stored in its own slot so the menu doesn't have to load every world.
 */
UCLASS()
class FACTORYGAME_API UFactoryWorldIndexSave : public USaveGame
{
	GENERATED_BODY()

public:

	static const FString SlotName;

	UPROPERTY(SaveGame)
	TArray<FFactoryWorldInfo> Worlds;

	/** Slot of the world that was played last, used by "Continue" */
	UPROPERTY(SaveGame)
	FString LastPlayedSlot;
};

/** Versions of the world save format. Add new versions before LatestPlusOne and handle them in MigrateToLatest() */
enum class EFactoryWorldSaveVersion : int32
{
	/** Saves from before versioning: player position only */
	Initial = 0,

	/** Player inventory and hotbar */
	Inventory = 1,

	/** Equipment slots, player inventory starts with 20 slots */
	Equipment = 2,

	LatestPlusOne,
	Latest = LatestPlusOne - 1
};

/**
 *  The actual data of one world
 */
UCLASS()
class FACTORYGAME_API UFactoryWorldSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	/** Upgrades data loaded from an older save format. Returns false if the save is from a newer game version */
	bool MigrateToLatest();

	/** Format version this save was written with. Old saves don't have the property and keep 0 */
	UPROPERTY(SaveGame)
	int32 SaveVersion = static_cast<int32>(EFactoryWorldSaveVersion::Initial);

	UPROPERTY(SaveGame)
	FString DisplayName;

	/** True once the player state has been saved at least once */
	UPROPERTY(SaveGame)
	bool bHasPlayerData = false;

	UPROPERTY(SaveGame)
	FTransform PlayerTransform;

	UPROPERTY(SaveGame)
	FRotator ControlRotation = FRotator::ZeroRotator;

	/** Total play time in seconds */
	UPROPERTY(SaveGame)
	double PlayTimeSeconds = 0.0;

	/** Player inventory, including empty slots */
	UPROPERTY(SaveGame)
	TArray<FFactoryItemStack> InventorySlots;

	UPROPERTY(SaveGame)
	int32 InventoryNumSlots = 0;

	/** Worn equipment: head, body, back, hand */
	UPROPERTY(SaveGame)
	TArray<FFactoryItemStack> EquipmentSlots;

	/** Build shortcuts on the hotbar */
	UPROPERTY(SaveGame)
	TArray<FName> HotbarSlots;
};
