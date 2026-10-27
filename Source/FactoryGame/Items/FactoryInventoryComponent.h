// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactoryItemTypes.h"
#include "FactoryInventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnFactoryInventoryChanged);

/**
 *  Slot based item storage with stack limits from the item database.
 *  Used by the player inventory and equipment, and later by chests and machine buffers.
 */
UCLASS(ClassGroup=(Factory), meta=(BlueprintSpawnableComponent))
class FACTORYGAME_API UFactoryInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UFactoryInventoryComponent();

	/** Player inventory size at the start and after all upgrades, see Docs/Standards/10-items-inventar.md */
	static constexpr int32 PlayerStartSlots = 20;
	static constexpr int32 PlayerSlotsPerUpgrade = 10;
	static constexpr int32 PlayerMaxSlots = 60;

	/** Called after every change of the slots */
	FOnFactoryInventoryChanged OnInventoryChanged;

	/** Sets up the slots */
	virtual void InitializeComponent() override;

	/** Returns all slots, empty slots included */
	const TArray<FFactoryItemStack>& GetSlots() const { return Slots; }

	int32 GetNumSlots() const { return Slots.Num(); }

	/** Turns the component into an equipment storage with one slot per entry. Must be called before the component is initialized */
	void SetSlotFilters(const TArray<EFactoryEquipSlot>& InSlotFilters);

	/** Returns the equipment slot type of a slot, None if the slot takes any item */
	EFactoryEquipSlot GetSlotFilter(int32 Index) const { return SlotFilters.IsValidIndex(Index) ? SlotFilters[Index] : EFactoryEquipSlot::None; }

	bool HasSlotFilters() const { return SlotFilters.Num() > 0; }

	/** Returns true if the item may be put into the slot */
	bool CanHold(int32 Index, FName ItemId) const;

	/** Changes the number of slots, e.g. for inventory upgrades. Items in removed slots are moved to free slots where possible and returned otherwise */
	TArray<FFactoryItemStack> SetNumSlots(int32 NewNumSlots);

	/** Adds as many of the items as fit. Returns the number of items that were added */
	int32 AddItem(FName ItemId, int32 Count);

	/** Returns how many of the given item would fit */
	int32 GetFreeSpaceFor(FName ItemId) const;

	/** Removes up to Count items. Returns the number of items that were removed */
	int32 RemoveItem(FName ItemId, int32 Count);

	/** Returns the total count of the given item across all slots */
	int32 GetItemCount(FName ItemId) const;

	/**
	 *  Moves up to Count items from a slot onto a slot of this or another inventory.
	 *  Equal items are merged. Different items are swapped if the whole stack is moved and both slots accept the other item.
	 *  Returns the number of items that left the source slot.
	 */
	int32 MoveTo(int32 FromIndex, UFactoryInventoryComponent* Target, int32 ToIndex, int32 Count);

	/** Moves a whole stack into the first fitting slots of another inventory, or swaps it into a matching equipment slot. Returns the number of moved items */
	int32 TransferStack(int32 Index, UFactoryInventoryComponent* Target);

	/** Moves all stacks of an item into another inventory. Returns the number of moved items */
	int32 TransferAll(FName ItemId, UFactoryInventoryComponent* Target);

	/** Merges partial stacks and sorts the slots by tier and item table order */
	void Sort();

	/** Removes all items */
	void Clear();

	/** Replaces all slots, e.g. when loading a save game. Equipment storages keep their slot count */
	void SetSlots(const TArray<FFactoryItemStack>& NewSlots, int32 NumSlots);

	/** Returns the number of slots that hold items */
	int32 GetNumUsedSlots() const;

protected:

	/** Number of slots when the component is created. Ignored if there are slot filters */
	UPROPERTY(EditAnywhere, Category="Inventory", meta=(ClampMin=1))
	int32 DefaultNumSlots = PlayerStartSlots;

	/** If set, the component has one slot per entry that only takes items of that equipment slot type */
	UPROPERTY(EditAnywhere, Category="Inventory")
	TArray<EFactoryEquipSlot> SlotFilters;

private:

	/** Returns the stack limit of the item */
	int32 GetMaxStackSize(FName ItemId) const;

	UPROPERTY(VisibleInstanceOnly, Category="Inventory")
	TArray<FFactoryItemStack> Slots;
};
