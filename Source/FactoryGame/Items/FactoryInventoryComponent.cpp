// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryInventoryComponent.h"
#include "FactoryItemDatabase.h"

UFactoryInventoryComponent::UFactoryInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	// Inventare ändern sich nur durch Aufrufe und müssen nicht ticken.
	// InitializeComponent wird gebraucht, um die Slots anzulegen.
	bWantsInitializeComponent = true;
}

void UFactoryInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (HasSlotFilters())
	{
		Slots.SetNum(SlotFilters.Num());
	}
	// sonst Standardgröße, falls noch nichts gesetzt wurde
	else if (Slots.Num() == 0)
	{
		Slots.SetNum(DefaultNumSlots);
	}
}

void UFactoryInventoryComponent::SetSlotFilters(const TArray<EFactoryEquipSlot>& InSlotFilters)
{
	SlotFilters = InSlotFilters;
}

bool UFactoryInventoryComponent::CanHold(int32 Index, FName ItemId) const
{
	if (!Slots.IsValidIndex(Index) || GetMaxStackSize(ItemId) <= 0)
	{
		return false;
	}

	// normale Inventare nehmen jedes tragbare Item, Ausrüstungs-Slots nur den passenden Typ
	if (!HasSlotFilters())
	{
		return true;
	}

	const UFactoryItemDatabase* Database = UFactoryItemDatabase::Get(this);
	const FFactoryItemDefinition* Item = Database ? Database->FindItem(ItemId) : nullptr;
	return Item && Item->EquipSlot == GetSlotFilter(Index);
}

TArray<FFactoryItemStack> UFactoryInventoryComponent::SetNumSlots(int32 NewNumSlots)
{
	// equipment storages have one slot per filter
	if (HasSlotFilters())
	{
		return {};
	}

	NewNumSlots = FMath::Max(1, NewNumSlots);

	// beim Verkleinern: Inhalt der wegfallenden Slots einsammeln
	TArray<FFactoryItemStack> Removed;

	for (int32 Index = NewNumSlots; Index < Slots.Num(); ++Index)
	{
		if (!Slots[Index].IsEmpty())
		{
			Removed.Add(Slots[Index]);
		}
	}

	Slots.SetNum(NewNumSlots);

	// put the items of the removed slots back where possible
	TArray<FFactoryItemStack> Overflow;

	for (FFactoryItemStack& Stack : Removed)
	{
		const int32 Added = AddItem(Stack.ItemId, Stack.Count);

		if (Added < Stack.Count)
		{
			Overflow.Emplace(Stack.ItemId, Stack.Count - Added);
		}
	}

	OnInventoryChanged.Broadcast();
	return Overflow;
}

int32 UFactoryInventoryComponent::GetMaxStackSize(FName ItemId) const
{
	const UFactoryItemDatabase* Database = UFactoryItemDatabase::Get(this);
	return Database ? Database->GetMaxStackSize(ItemId) : 0;
}

int32 UFactoryInventoryComponent::GetFreeSpaceFor(FName ItemId) const
{
	const int32 MaxStack = GetMaxStackSize(ItemId);

	if (MaxStack <= 0)
	{
		return 0;
	}

	int32 Space = 0;

	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		const FFactoryItemStack& Stack = Slots[Index];

		if (Stack.IsEmpty())
		{
			Space += CanHold(Index, ItemId) ? MaxStack : 0;
		}
		else if (Stack.ItemId == ItemId)
		{
			Space += FMath::Max(0, MaxStack - Stack.Count);
		}
	}

	return Space;
}

int32 UFactoryInventoryComponent::AddItem(FName ItemId, int32 Count)
{
	const int32 MaxStack = GetMaxStackSize(ItemId);

	if (Count <= 0 || MaxStack <= 0)
	{
		return 0;
	}

	int32 Remaining = Count;

	// fill existing stacks first, then empty slots
	for (int32 Pass = 0; Pass < 2 && Remaining > 0; ++Pass)
	{
		for (int32 Index = 0; Index < Slots.Num(); ++Index)
		{
			FFactoryItemStack& Stack = Slots[Index];
			// Durchgang 0: Stapel desselben Items auffüllen, Durchgang 1: leere Slots belegen, die das Item annehmen
			const bool bMatches = Pass == 0 ? (!Stack.IsEmpty() && Stack.ItemId == ItemId) : (Stack.IsEmpty() && CanHold(Index, ItemId));

			if (!bMatches)
			{
				continue;
			}

			if (Stack.IsEmpty())
			{
				Stack = FFactoryItemStack(ItemId, 0);
			}

			const int32 Moved = FMath::Min(Remaining, MaxStack - Stack.Count);
			Stack.Count += Moved;
			Remaining -= Moved;

			if (Remaining <= 0)
			{
				break;
			}
		}
	}

	const int32 Added = Count - Remaining;

	if (Added > 0)
	{
		OnInventoryChanged.Broadcast();
	}

	return Added;
}

int32 UFactoryInventoryComponent::RemoveItem(FName ItemId, int32 Count)
{
	if (Count <= 0)
	{
		return 0;
	}

	int32 Remaining = Count;

	// take from the back so the first stacks stay full
	for (int32 Index = Slots.Num() - 1; Index >= 0 && Remaining > 0; --Index)
	{
		FFactoryItemStack& Stack = Slots[Index];

		if (Stack.IsEmpty() || Stack.ItemId != ItemId)
		{
			continue;
		}

		const int32 Taken = FMath::Min(Remaining, Stack.Count);
		Stack.Count -= Taken;
		Remaining -= Taken;

		if (Stack.Count <= 0)
		{
			Stack.Clear();
		}
	}

	const int32 Removed = Count - Remaining;

	if (Removed > 0)
	{
		OnInventoryChanged.Broadcast();
	}

	return Removed;
}

int32 UFactoryInventoryComponent::GetItemCount(FName ItemId) const
{
	int32 Total = 0;

	for (const FFactoryItemStack& Stack : Slots)
	{
		if (!Stack.IsEmpty() && Stack.ItemId == ItemId)
		{
			Total += Stack.Count;
		}
	}

	return Total;
}

int32 UFactoryInventoryComponent::MoveTo(int32 FromIndex, UFactoryInventoryComponent* Target, int32 ToIndex, int32 Count)
{
	if (!Target || !Slots.IsValidIndex(FromIndex) || !Target->Slots.IsValidIndex(ToIndex) || Slots[FromIndex].IsEmpty() || (Target == this && FromIndex == ToIndex))
	{
		return 0;
	}

	FFactoryItemStack& From = Slots[FromIndex];
	FFactoryItemStack& To = Target->Slots[ToIndex];
	Count = FMath::Clamp(Count, 1, From.Count);

	int32 Moved = 0;

	// Ziel leer oder gleiches Item: so viel verschieben, wie auf den Zielstapel passt
	if (To.IsEmpty() || To.ItemId == From.ItemId)
	{
		if (!Target->CanHold(ToIndex, From.ItemId))
		{
			return 0;
		}

		if (To.IsEmpty())
		{
			To = FFactoryItemStack(From.ItemId, 0);
		}

		Moved = FMath::Min(Count, GetMaxStackSize(From.ItemId) - To.Count);
		To.Count += Moved;
		From.Count -= Moved;

		if (From.Count <= 0)
		{
			From.Clear();
		}
	}
	else
	{
		// different items are only swapped as whole stacks, otherwise one slot would have to hold two items
		if (Count < From.Count || !Target->CanHold(ToIndex, From.ItemId) || !CanHold(FromIndex, To.ItemId))
		{
			return 0;
		}

		Moved = From.Count;
		Swap(From, To);
	}

	if (Moved > 0)
	{
		OnInventoryChanged.Broadcast();

		if (Target != this)
		{
			Target->OnInventoryChanged.Broadcast();
		}
	}

	return Moved;
}

int32 UFactoryInventoryComponent::TransferStack(int32 Index, UFactoryInventoryComponent* Target)
{
	if (!Target || Target == this || !Slots.IsValidIndex(Index) || Slots[Index].IsEmpty())
	{
		return 0;
	}

	// Shift+Klick: erst normal auf passende Stapel und freie Slots des Ziels verteilen
	const FFactoryItemStack Stack = Slots[Index];
	const int32 Added = Target->AddItem(Stack.ItemId, Stack.Count);

	if (Added > 0)
	{
		Slots[Index].Count -= Added;

		if (Slots[Index].Count <= 0)
		{
			Slots[Index].Clear();
		}

		OnInventoryChanged.Broadcast();
		return Added;
	}

	// an occupied equipment slot gets swapped, e.g. to change the tool in the hand
	if (Target->HasSlotFilters())
	{
		for (int32 TargetIndex = 0; TargetIndex < Target->GetNumSlots(); ++TargetIndex)
		{
			if (Target->CanHold(TargetIndex, Stack.ItemId))
			{
				return MoveTo(Index, Target, TargetIndex, Stack.Count);
			}
		}
	}

	return 0;
}

int32 UFactoryInventoryComponent::TransferAll(FName ItemId, UFactoryInventoryComponent* Target)
{
	if (!Target || Target == this)
	{
		return 0;
	}

	// no equipment swapping here, it would just cycle the stacks through the equipment slot
	const int32 Moved = Target->AddItem(ItemId, GetItemCount(ItemId));
	RemoveItem(ItemId, Moved);
	return Moved;
}

void UFactoryInventoryComponent::Sort()
{
	// equipment slots have fixed meanings
	if (HasSlotFilters())
	{
		return;
	}

	// sum up all items in the order of the item database
	TMap<FName, int32> Totals;
	TArray<FName> Order;

	for (const FFactoryItemStack& Stack : Slots)
	{
		if (!Stack.IsEmpty())
		{
			if (!Totals.Contains(Stack.ItemId))
			{
				Order.Add(Stack.ItemId);
			}

			Totals.FindOrAdd(Stack.ItemId) += Stack.Count;
		}
	}

	if (const UFactoryItemDatabase* Database = UFactoryItemDatabase::Get(this))
	{
		const TArray<FName>& AllIds = Database->GetAllItemIds();

		// unknown items keep their place at the end
		Order.StableSort([&AllIds](const FName& A, const FName& B)
		{
			const int32 IndexA = AllIds.Find(A);
			const int32 IndexB = AllIds.Find(B);
			return (IndexA == INDEX_NONE ? MAX_int32 : IndexA) < (IndexB == INDEX_NONE ? MAX_int32 : IndexB);
		});
	}

	const int32 NumSlots = Slots.Num();
	TArray<FFactoryItemStack> Sorted;
	Sorted.Reserve(NumSlots);

	for (const FName& ItemId : Order)
	{
		// unknown items have no stack limit, so keep them as one stack
		const int32 MaxStack = GetMaxStackSize(ItemId);
		int32 Remaining = Totals[ItemId];

		while (Remaining > 0)
		{
			const int32 Count = MaxStack > 0 ? FMath::Min(Remaining, MaxStack) : Remaining;
			Sorted.Emplace(ItemId, Count);
			Remaining -= Count;
		}
	}

	// only happens if stack sizes were lowered after the items were stored, keep everything as it is then
	if (Sorted.Num() > NumSlots)
	{
		return;
	}

	Sorted.SetNum(NumSlots);
	Slots = MoveTemp(Sorted);

	OnInventoryChanged.Broadcast();
}

void UFactoryInventoryComponent::Clear()
{
	for (FFactoryItemStack& Stack : Slots)
	{
		Stack.Clear();
	}

	OnInventoryChanged.Broadcast();
}

void UFactoryInventoryComponent::SetSlots(const TArray<FFactoryItemStack>& NewSlots, int32 NumSlots)
{
	// die Ausrüstung behält immer einen Slot pro Filter, egal wie viele der Spielstand enthält
	Slots = NewSlots;
	Slots.SetNum(HasSlotFilters() ? SlotFilters.Num() : FMath::Max(1, NumSlots));

	OnInventoryChanged.Broadcast();
}

int32 UFactoryInventoryComponent::GetNumUsedSlots() const
{
	int32 Used = 0;

	for (const FFactoryItemStack& Stack : Slots)
	{
		Used += Stack.IsEmpty() ? 0 : 1;
	}

	return Used;
}
