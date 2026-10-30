// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FactoryItemTypes.h"
#include "FactoryProductionComponent.h"
#include "FactoryBeltComponent.h"
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

/*
 *  Spielstand-Versionierung: Jede Änderung am Speicherformat bekommt hier eine neue Version.
 *  Beim Laden ruft die Game Instance MigrateToLatest() auf, das alte Stände Schritt für Schritt
 *  hochrüstet. Stände aus einer neueren Spielversion werden abgelehnt statt überschrieben.
 */
/** Versions of the world save format. Add new versions before LatestPlusOne and handle them in MigrateToLatest() */
enum class EFactoryWorldSaveVersion : int32
{
	/** Saves from before versioning: player position only */
	Initial = 0,

	/** Player inventory and hotbar */
	Inventory = 1,

	/** Equipment slots, player inventory starts with 20 slots */
	Equipment = 2,

	/** Placed machines and demolition crates */
	Buildings = 3,

	/** Conveyor poles and belts */
	Belts = 4,

	/** HUB with storage, unlocked tier and delivered milestone items */
	Hub = 5,

	LatestPlusOne,
	Latest = LatestPlusOne - 1
};

/** A placed production machine */
USTRUCT()
struct FFactorySavedMachine
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FName BuildingId;

	UPROPERTY(SaveGame)
	FTransform Transform;

	UPROPERTY(SaveGame)
	FFactoryProductionSaveData Production;
};

/** A conveyor pole */
USTRUCT()
struct FFactorySavedPole
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FTransform Transform;

	UPROPERTY(SaveGame)
	int32 Height = 1;
};

/** What a belt end is attached to: index into Machines or Poles of the same save */
USTRUCT()
struct FFactorySavedBeltEnd
{
	GENERATED_BODY()

	/** 0 = nothing, 1 = machine, 2 = pole */
	UPROPERTY(SaveGame)
	uint8 Type = 0;

	UPROPERTY(SaveGame)
	int32 Index = INDEX_NONE;

	/** Port index of the machine */
	UPROPERTY(SaveGame)
	int32 Port = INDEX_NONE;
};

/** A conveyor belt: its curve, tier, items and both ends */
USTRUCT()
struct FFactorySavedBelt
{
	GENERATED_BODY()

	/** Curve points in world space, with the tangents of each point */
	UPROPERTY(SaveGame)
	TArray<FVector> Locations;

	UPROPERTY(SaveGame)
	TArray<FVector> ArriveTangents;

	UPROPERTY(SaveGame)
	TArray<FVector> LeaveTangents;

	UPROPERTY(SaveGame)
	int32 Tier = 1;

	UPROPERTY(SaveGame)
	TArray<FFactoryBeltItem> Items;

	UPROPERTY(SaveGame)
	FFactorySavedBeltEnd Start;

	UPROPERTY(SaveGame)
	FFactorySavedBeltEnd End;
};

/** A demolition crate with the items that didn't fit into the inventory */
USTRUCT()
struct FFactorySavedCrate
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(SaveGame)
	TArray<FFactoryItemStack> Items;
};

/**
 *  The actual data of one world
 *
 *  Alles, was zu einer Welt gespeichert wird (Saved/SaveGames/World_<GUID>.sav).
 *  Alle Felder sind mit UPROPERTY(SaveGame) markiert, nur solche landen im Spielstand.
 *  Ändert sich Format oder Bedeutung eines Feldes: EFactoryWorldSaveVersion erhöhen und
 *  MigrateToLatest() erweitern.
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

	/** Placed machines */
	UPROPERTY(SaveGame)
	TArray<FFactorySavedMachine> Machines;

	UPROPERTY(SaveGame)
	TArray<FFactorySavedPole> Poles;

	/** Belts refer to Machines and Poles by index */
	UPROPERTY(SaveGame)
	TArray<FFactorySavedBelt> Belts;

	UPROPERTY(SaveGame)
	TArray<FFactorySavedCrate> Crates;

	/** False for worlds saved before the HUB existed, they get a new one */
	UPROPERTY(SaveGame)
	bool bHasHub = false;

	UPROPERTY(SaveGame)
	FTransform HubTransform;

	UPROPERTY(SaveGame)
	TArray<FFactoryItemStack> HubStorage;

	/** Highest unlocked tier */
	UPROPERTY(SaveGame)
	int32 UnlockedTier = 0;

	/** Items already delivered for the next milestone */
	UPROPERTY(SaveGame)
	TArray<FFactoryItemStack> MilestoneDelivered;
};
