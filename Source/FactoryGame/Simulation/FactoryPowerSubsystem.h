// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FactoryPowerSubsystem.generated.h"

class AActor;
class UFactoryProductionComponent;

/** What a power network does right now, for the machine panel */
struct FFactoryPowerStats
{
	/** Output of all generators that have fuel, in MW */
	int32 Capacity = 0;

	/** Output of all generators, with or without fuel, in MW */
	int32 MaxOutput = 0;

	/** Power use of all machines that want to work, in MW */
	int32 Demand = 0;

	/** Share of the capacity in use, in per mille */
	int32 LoadPermille = 0;

	bool bTripped = false;

	int32 NumGenerators = 0;

	int32 NumConsumers = 0;
};

/**
 *  Power networks: everything connected by power cables (poles, machines, generators) forms one network.
 *  Runs first in every simulation tick, before the machines, and decides which machines have power
 *  and how fast the generators burn their fuel.
 *
 *  Ablauf pro Tick und Netz:
 *  1. Leistung = Summe der Generatoren mit Brennstoff, Verbrauch = Summe der Maschinen, die arbeiten könnten.
 *  2. Verbrauch > Leistung (und Leistung > 0): die Sicherung fliegt raus, das ganze Netz steht,
 *     bis sie an einem Generator wieder eingeschaltet wird (ResetFuse, nur mit genug Leistung).
 *  3. Sonst haben alle Maschinen Strom, und die Generatoren verbrennen anteilig zur Last.
 *  Ohne Generator mit Brennstoff hat das Netz einfach keinen Strom (keine Sicherung).
 *  Die Netze werden nur neu bestimmt, wenn ein Kabel, Mast oder eine Maschine dazukommt oder wegfällt.
 *  Regeln: Docs/Standards/17-strom.md
 */
UCLASS()
class FACTORYGAME_API UFactoryPowerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	static UFactoryPowerSubsystem* Get(const UObject* WorldContextObject);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** Networks are rebuilt before the next tick */
	void MarkDirty() { bDirty = true; }

	/** Called by UFactorySimulationSubsystem at the start of every simulation tick */
	void SimTick(int64 Tick);

	/** Returns the network of a pole or machine. False if it has no cable */
	bool GetStats(const AActor* Node, FFactoryPowerStats& OutStats) const;

	/**
	 *  Switches the fuse of a generator's network on again. Fails (returns false) if the network
	 *  still needs more power than its generators with fuel give
	 */
	bool ResetFuse(const AActor* Generator);

	/** Cheat: every machine has power, with or without cables and generators */
	void SetFreePower(bool bInFreePower) { bFreePower = bInFreePower; }

	bool IsFreePower() const { return bFreePower; }

	int32 GetNumNetworks() const { return Networks.Num(); }

	/** Writes all networks to the log */
	void LogNetworks() const;

private:

	struct FNetwork
	{
		TArray<TWeakObjectPtr<UFactoryProductionComponent>> Generators;
		TArray<TWeakObjectPtr<UFactoryProductionComponent>> Consumers;
		int32 NumPoles = 0;
		FFactoryPowerStats Stats;
	};

	/** Finds the networks from the cables */
	void Rebuild();

	/** Shows "fuse tripped" on the HUD */
	void NotifyTripped() const;

	TArray<FNetwork> Networks;

	/** Network index of every pole and machine with at least one cable */
	TMap<TWeakObjectPtr<const AActor>, int32> NetworkOfNode;

	/** Machines without a cable: no power (except with FreePower) */
	TArray<TWeakObjectPtr<UFactoryProductionComponent>> Unconnected;

	bool bDirty = true;

	bool bFreePower = false;
};
