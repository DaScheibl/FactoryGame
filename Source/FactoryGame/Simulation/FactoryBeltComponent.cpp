// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryBeltComponent.h"
#include "FactoryProductionComponent.h"
#include "FactorySimulationSubsystem.h"

int32 UFactoryBeltComponent::GetSpeedForTier(int32 Tier)
{
	// Mk1 = 25 mm/Tick = 0,5 m/s = 60 Items/min bei 0,5 m Abstand, jede Stufe verdoppelt (Mk5 = 400 mm/Tick)
	return 25 << (FMath::Clamp(Tier, 1, MaxTier) - 1);
}

int32 UFactoryBeltComponent::GetItemsPerMinute(int32 Tier)
{
	return GetSpeedForTier(Tier) * UFactorySimulationSubsystem::TicksPerSecond * 60 / ItemSpacing;
}

UFactoryBeltComponent::UFactoryBeltComponent()
{
	// gerechnet wird im Simulations-Takt, nicht im normalen Tick
	PrimaryComponentTick.bCanEverTick = false;
}

void UFactoryBeltComponent::BeginPlay()
{
	Super::BeginPlay();

	// nach dem Laden stehen nur die Lücken im Spielstand, die Summen werden neu gebildet
	RecalculateTotalGap();

	if (UFactorySimulationSubsystem* Simulation = UFactorySimulationSubsystem::Get(this))
	{
		Simulation->Register(this);
	}
}

void UFactoryBeltComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFactorySimulationSubsystem* Simulation = UFactorySimulationSubsystem::Get(this))
	{
		Simulation->Unregister(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UFactoryBeltComponent::SimTick(int64 Tick)
{
	MoveItems();
	PullFromSource();
}

void UFactoryBeltComponent::MoveItems()
{
	int32 Remaining = GetSpeed();
	bBlocked = false;

	while (Remaining > 0 && Items.Num() > 0)
	{
		FFactoryBeltItem& Front = Items[0];

		// das vorderste Item kommt in diesem Tick nicht bis ans Ende: nur seine Lücke schrumpft,
		// alle Items dahinter rücken damit automatisch mit
		if (Front.Gap >= Remaining)
		{
			Front.Gap -= Remaining;
			TotalGap -= Remaining;
			return;
		}

		// es erreicht das Ende; was von der Bewegung übrig ist, nimmt es auf den Empfänger mit
		Remaining -= Front.Gap;
		TotalGap -= Front.Gap;
		Front.Gap = 0;

		IFactoryItemReceiver* Receiver = Cast<IFactoryItemReceiver>(Target.Get());

		if (!Receiver || !Receiver->ReceiveItem(Front.ItemId, Remaining))
		{
			bBlocked = true;
			break;
		}

		Items.RemoveAt(0, 1, EAllowShrinking::No);
		++ItemsDelivered;
		NumCompressed = 0;

		// das nächste Item ist jetzt vorn; sein Abstand zum Ende = Lücke + Item-Abstand
		if (Items.Num() > 0)
		{
			Items[0].Gap += ItemSpacing;
			TotalGap += ItemSpacing;
		}
	}

	if (!bBlocked)
	{
		return;
	}

	// Stau: das vorderste Item steht. Die restliche Bewegung schließt die Lücken dahinter, von vorn nach hinten.
	// Bereits dicht gepackte Items werden übersprungen, deshalb kostet ein volles Band fast nichts
	int32 Index = FMath::Max(1, NumCompressed);

	while (Index < Items.Num() && Remaining > 0)
	{
		const int32 Closed = FMath::Min(Remaining, Items[Index].Gap);
		Items[Index].Gap -= Closed;
		TotalGap -= Closed;
		Remaining -= Closed;

		if (Items[Index].Gap > 0)
		{
			break;
		}

		++Index;
	}

	NumCompressed = Index;
}

void UFactoryBeltComponent::PullFromSource()
{
	UFactoryProductionComponent* Machine = Source.Get();

	if (!Machine || !HasRoomAtStart())
	{
		return;
	}

	// das Item kann in diesem Tick schon eine Tick-Strecke weit gekommen sein; ohne das würde bei
	// schnellen Bändern (Mk5: 400 mm/Tick bei 500 mm Abstand) Durchsatz verloren gehen
	const TArray<FFactoryItemStack>& Outputs = Machine->GetOutputs();
	const FName OutputItem = Outputs.IsValidIndex(SourceOutput) ? Outputs[SourceOutput].ItemId : NAME_None;
	const FFactoryItemStack Taken = Machine->ExtractOutput(OutputItem, 1);

	if (!Taken.IsEmpty())
	{
		ReceiveItem(Taken.ItemId, GetSpeed());
	}
}

bool UFactoryBeltComponent::ReceiveItem(FName ItemId, int32 Offset)
{
	if (ItemId.IsNone() || !HasRoomAtStart())
	{
		return false;
	}

	// das neue Item darf Offset weit auf dem Band liegen, aber nie dichter als ItemSpacing am letzten Item.
	// Hinweis: Tickt dieses Band in diesem Tick erst noch, rückt das Item einmal zusätzlich vor.
	// Das bleibt deterministisch und fällt erst weg, wenn Bandketten zu einem Segment zusammengelegt werden
	const int32 MaxPosition = Items.Num() > 0 ? GetFreeSpaceAtStart() - ItemSpacing : LengthMm;
	const int32 Position = FMath::Clamp(Offset, 0, MaxPosition);
	const int32 Gap = MaxPosition - Position;

	Items.Emplace(ItemId, Gap);
	TotalGap += Gap;
	return true;
}

bool UFactoryBeltComponent::HasRoomAtStart() const
{
	return Items.Num() == 0 || GetFreeSpaceAtStart() >= ItemSpacing;
}

int32 UFactoryBeltComponent::GetFreeSpaceAtStart() const
{
	if (Items.Num() == 0)
	{
		return LengthMm;
	}

	return LengthMm - (TotalGap + (Items.Num() - 1) * ItemSpacing);
}

void UFactoryBeltComponent::SetTarget(UObject* NewTarget)
{
	Target = Cast<IFactoryItemReceiver>(NewTarget) ? NewTarget : nullptr;
}

void UFactoryBeltComponent::SetLength(int32 NewLengthMm, TArray<FName>& OutRemoved)
{
	LengthMm = FMath::Max(ItemSpacing, NewLengthMm);

	// Positionen zählen vom Bandende; was hinten nicht mehr draufpasst, fliegt runter
	while (Items.Num() > 0 && GetFreeSpaceAtStart() < 0)
	{
		OutRemoved.Add(Items.Last().ItemId);
		TotalGap -= Items.Last().Gap;
		Items.Pop(EAllowShrinking::No);
	}

	NumCompressed = FMath::Min(NumCompressed, Items.Num());
}

void UFactoryBeltComponent::LoadItems(const TArray<FFactoryBeltItem>& NewItems)
{
	Items = NewItems;
	RecalculateTotalGap();

	// passt der Inhalt nicht mehr (Länge geändert), fliegt hinten herunter, was übersteht
	TArray<FName> Ignored;
	SetLength(LengthMm, Ignored);
}

void UFactoryBeltComponent::ClearItems(TArray<FName>& OutRemoved)
{
	for (const FFactoryBeltItem& Item : Items)
	{
		OutRemoved.Add(Item.ItemId);
	}

	Items.Reset();
	TotalGap = 0;
	NumCompressed = 0;
	bBlocked = false;
}

void UFactoryBeltComponent::RecalculateTotalGap()
{
	TotalGap = 0;

	for (const FFactoryBeltItem& Item : Items)
	{
		TotalGap += Item.Gap;
	}

	NumCompressed = 0;
}

void UFactoryBeltComponent::GetDisplayPositions(TArray<float>& OutPositions) const
{
	OutPositions.Reset(Items.Num());

	// die Items springen nur alle 1/20 s; dazwischen um den Anteil bis zum nächsten Tick weiterschieben,
	// aber nie über das Bandende hinaus oder in das Item davor
	const UFactorySimulationSubsystem* Simulation = UFactorySimulationSubsystem::Get(this);
	const float Step = Simulation ? Simulation->GetTickAlpha() * GetSpeed() : 0.0f;

	int32 DistanceFromEnd = 0;
	float PreviousDisplay = 0.0f;

	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		DistanceFromEnd += Items[Index].Gap + (Index > 0 ? ItemSpacing : 0);

		const float MinDisplay = Index > 0 ? PreviousDisplay + ItemSpacing : 0.0f;
		const float Display = FMath::Max(DistanceFromEnd - Step, MinDisplay);
		PreviousDisplay = Display;

		// mm vom Ende -> cm vom Anfang
		OutPositions.Add(FMath::Max(LengthMm - Display, 0.0f) * 0.1f);
	}
}
