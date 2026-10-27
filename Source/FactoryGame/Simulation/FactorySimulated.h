// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FactorySimulated.generated.h"

UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UFactorySimulated : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Anything that takes part in the factory simulation: machines, later belts, power grids, trains.
 *
 *  Regeln für SimTick (damit die Simulation deterministisch bleibt):
 *  - nur mit ganzen Ticks rechnen, nie mit DeltaTime oder der Uhrzeit
 *  - kein Zufall, keine Abhängigkeit von der Framerate oder der Kamera
 *  - nichts Grafisches darin machen; Optik reagiert auf Events (z. B. OnStateChanged) oder im normalen Actor-Tick
 *  Anmelden mit UFactorySimulationSubsystem::Register, abmelden mit Unregister.
 */
class FACTORYGAME_API IFactorySimulated
{
	GENERATED_BODY()

public:

	/** Advances the object by one simulation tick (1 / UFactorySimulationSubsystem::TicksPerSecond seconds) */
	virtual void SimTick(int64 Tick) = 0;
};
