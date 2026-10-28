// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactorySimulated.h"
#include "FactoryItemReceiver.h"
#include "FactoryBeltComponent.generated.h"

class UFactoryProductionComponent;

/** One item on a belt */
USTRUCT()
struct FACTORYGAME_API FFactoryBeltItem
{
	GENERATED_BODY()

	FFactoryBeltItem() = default;
	FFactoryBeltItem(FName InItemId, int32 InGap) : ItemId(InItemId), Gap(InGap) {}

	UPROPERTY(SaveGame)
	FName ItemId;

	/** Free space to the item in front (beyond the item spacing) in mm. For the front item: distance to the belt end */
	UPROPERTY(SaveGame)
	int32 Gap = 0;
};

/**
 *  Moves items along one belt segment in the factory simulation. One lane, items keep a fixed spacing.
 *
 *  Segment-basiert statt pro Item: Das Band merkt sich keine Positionen, sondern nur die Lücken
 *  zwischen den Items (vorne beginnend). Läuft das Band, wird pro Tick nur die erste Lücke kleiner,
 *  alle Items dahinter rücken dadurch automatisch mit. Ein volles, gestautes Band kostet so fast
 *  nichts, egal wie viele Items darauf liegen.
 *  - Am Ende wird das vorderste Item an den Empfänger (Maschine, nächstes Band) übergeben.
 *    Nimmt er es nicht an, staut sich das Band von vorn nach hinten.
 *  - Am Anfang holt sich das Band ein Item aus dem Ausgang der Quell-Maschine, sobald hinten Platz ist.
 *  Gerechnet wird in ganzen Millimetern, Werte: Docs/Standards/12-simulation.md
 */
UCLASS(ClassGroup=(Factory), meta=(BlueprintSpawnableComponent))
class FACTORYGAME_API UFactoryBeltComponent : public UActorComponent, public IFactorySimulated, public IFactoryItemReceiver
{
	GENERATED_BODY()

public:

	/** Distance between two items in mm (0.5 m) */
	static constexpr int32 ItemSpacing = 500;

	/** Highest belt tier (Mk5) */
	static constexpr int32 MaxTier = 5;

	/** Returns the belt speed of a tier in mm per simulation tick. Mk1 = 25 (60 items/min), every tier doubles */
	static int32 GetSpeedForTier(int32 Tier);

	/** Returns the throughput of a tier in items per minute */
	static int32 GetItemsPerMinute(int32 Tier);

	UFactoryBeltComponent();

	// IFactorySimulated
	virtual void SimTick(int64 Tick) override;

	// IFactoryItemReceiver: puts the item at the belt start (if there is room)
	virtual bool ReceiveItem(FName ItemId, int32 Offset) override;

	/** Sets the belt length in mm. Items that don't fit anymore are removed and added to OutRemoved */
	void SetLength(int32 NewLengthMm, TArray<FName>& OutRemoved);

	int32 GetLength() const { return LengthMm; }

	void SetTier(int32 NewTier) { Tier = FMath::Clamp(NewTier, 1, MaxTier); }

	int32 GetTier() const { return Tier; }

	/** Returns the speed in mm per simulation tick */
	int32 GetSpeed() const { return GetSpeedForTier(Tier); }

	/** Machine the belt takes items from at its start (None: only fed by other belts or by hand) */
	void SetSource(UFactoryProductionComponent* NewSource) { Source = NewSource; }

	UFactoryProductionComponent* GetSource() const { return Source.Get(); }

	/** Receiver at the belt end. Must implement IFactoryItemReceiver */
	void SetTarget(UObject* NewTarget);

	UObject* GetTarget() const { return Target.Get(); }

	/** Items from front (belt end) to back (belt start) */
	const TArray<FFactoryBeltItem>& GetItems() const { return Items; }

	/** True if the front item is waiting at the belt end because the receiver had no room */
	bool IsBlocked() const { return bBlocked; }

	/** Returns true if an item could be put onto the belt start right now */
	bool HasRoomAtStart() const;

	/** Removes all items and adds them to OutRemoved, e.g. to give them back to the player */
	void ClearItems(TArray<FName>& OutRemoved);

	/** Returns the number of items that left the belt at its end since it was built */
	int64 GetItemsDelivered() const { return ItemsDelivered; }

	/**
	 *  Returns the position of every item, in cm from the belt start, in the same order as GetItems.
	 *  Estimated between two simulation ticks so items move smoothly. Only for visuals, never use it inside SimTick
	 */
	void GetDisplayPositions(TArray<float>& OutPositions) const;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Belt tier 1-5 (Mk1-Mk5) */
	UPROPERTY(EditAnywhere, SaveGame, Category="Belt", meta=(ClampMin=1, ClampMax=5))
	int32 Tier = 1;

	/** Belt length in mm */
	UPROPERTY(EditAnywhere, SaveGame, Category="Belt", meta=(ClampMin=500))
	int32 LengthMm = 4000;

private:

	/** Moves all items forward by one tick and hands the front item over at the end */
	void MoveItems();

	/** Takes one item from the source machine if there is room at the start */
	void PullFromSource();

	/** Returns the distance from the belt start to the last item in mm (the length if the belt is empty) */
	int32 GetFreeSpaceAtStart() const;

	/** Recalculates TotalGap from the items, e.g. after loading */
	void RecalculateTotalGap();

	UPROPERTY(VisibleInstanceOnly, SaveGame, Category="Belt")
	TArray<FFactoryBeltItem> Items;

	UPROPERTY(VisibleInstanceOnly, SaveGame, Category="Belt")
	int64 ItemsDelivered = 0;

	UPROPERTY()
	TWeakObjectPtr<UFactoryProductionComponent> Source;

	UPROPERTY()
	TWeakObjectPtr<UObject> Target;

	/** Sum of all gaps. Distance from the belt end to the last item = TotalGap + (items - 1) x spacing */
	int32 TotalGap = 0;

	/**
	 *  Number of items at the front that are packed tightly (gap 0). Never more than the real number,
	 *  so it can be used to skip them: a jammed belt costs almost nothing
	 */
	int32 NumCompressed = 0;

	bool bBlocked = false;
};
