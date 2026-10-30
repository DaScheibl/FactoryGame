// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryItemTypes.h"
#include "FactoryHub.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class UFactoryInventoryComponent;
struct FFactoryBuildingDefinition;
struct FFactoryRecipeDefinition;

/** What has to be delivered at the HUB to unlock a tier */
struct FFactoryMilestone
{
	int32 Tier = 0;
	TArray<FFactoryItemStack> Costs;
};

/**
 *  The HUB: the start building every world has. Holds the workbench, a storage with 24 slots and the milestones.
 *  Delivering research packs unlocks the next tier; buildings and recipes of higher tiers stay locked.
 *
 *  Es gibt genau einen HUB pro Welt. Neue Welten (und alte Spielstände ohne HUB) bekommen ihn
 *  über EnsureHub() neben dem Startpunkt, kostenlos und nicht abreißbar. Er speichert auch den
 *  Fortschritt (freigeschaltete Tier, schon abgegebene Pakete). Regeln: Docs/Standards/16-hub.md
 */
UCLASS()
class FACTORYGAME_API AFactoryHub : public AActor
{
	GENERATED_BODY()

public:

	/** Building table id, used for size and name */
	static const FName HubId;

	static constexpr int32 StorageSlots = 24;

	/** Highest tier that can be unlocked with milestones for now */
	static constexpr int32 MaxMilestoneTier = 2;

	AFactoryHub();

	/** Returns the HUB of the world, or nullptr */
	static AFactoryHub* Get(const UObject* WorldContextObject);

	/** Spawns the HUB next to the start point if the world doesn't have one yet */
	static AFactoryHub* EnsureHub(UWorld* World);

	/** Spawns a HUB at the transform, e.g. when loading */
	static AFactoryHub* SpawnHub(UWorld* World, const FTransform& Transform);

	/** Highest unlocked tier of the world (0 without HUB) */
	static int32 GetUnlockedTier(const UObject* WorldContextObject);

	static bool IsBuildingUnlocked(const UObject* WorldContextObject, const FFactoryBuildingDefinition& Building);

	/** A recipe is unlocked when its tier and the tier of its building are unlocked (workbench recipes: only the recipe tier) */
	static bool IsRecipeUnlocked(const UObject* WorldContextObject, const FFactoryRecipeDefinition& Recipe, bool bAtWorkbench);

	/** Returns the milestone that unlocks the given tier, nullptr if there is none */
	static const FFactoryMilestone* GetMilestone(int32 Tier);

	int32 GetTier() const { return UnlockedTier; }

	void SetTier(int32 NewTier);

	UFactoryInventoryComponent* GetStorage() const { return Storage; }

	/** How many of an item have already been delivered for the next milestone */
	int32 GetDelivered(FName ItemId) const;

	/** Takes items from the inventory for the next milestone, as many as are still needed. Returns true if anything was taken */
	bool Deliver(UFactoryInventoryComponent* Inventory);

	/** True if everything for the next milestone has been delivered */
	bool CanUnlockNext() const;

	/** Unlocks the next tier if everything was delivered */
	bool UnlockNext();

	/** For the world save */
	const TArray<FFactoryItemStack>& GetDeliveredStacks() const { return Delivered; }

	void SetDeliveredStacks(const TArray<FFactoryItemStack>& Stacks) { Delivered = Stacks; }

	virtual void Tick(float DeltaSeconds) override;

private:

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UTextRenderComponent> Label;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UFactoryInventoryComponent> Storage;

	/** Highest unlocked tier */
	UPROPERTY(VisibleAnywhere, Category="HUB")
	int32 UnlockedTier = 0;

	/** Items delivered for the next milestone */
	UPROPERTY(VisibleAnywhere, Category="HUB")
	TArray<FFactoryItemStack> Delivered;
};
