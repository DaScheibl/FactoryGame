// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FactorySimulationSubsystem.generated.h"

class IFactorySimulated;

/**
 *  Runs the factory simulation at a fixed rate, independent of the frame rate.
 *
 *  So funktioniert's:
 *  - Jedes Bild wird die vergangene Spielzeit in einem Topf gesammelt. Sobald genug für einen
 *    Simulations-Tick (1/20 s) drin ist, wird ein Tick gerechnet, bei 144 FPS also nur jedes ~7. Bild,
 *    bei 10 FPS zwei pro Bild.
 *  - Nach einem Ruckler werden verpasste Ticks nachgeholt, aber höchstens 0,25 s pro Bild.
 *    Bei längeren Hängern läuft die Fabrik kurz langsamer, statt das Spiel weiter einzufrieren.
 *  - Angemeldete Objekte (IFactorySimulated) werden immer in derselben Reihenfolge abgearbeitet.
 *  - Im Pause-Menü steht die Simulation still (Subsystem tickt nicht, wenn das Spiel pausiert ist).
 *  Werte: Docs/Standards/12-simulation.md
 */
UCLASS()
class FACTORYGAME_API UFactorySimulationSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	/** Simulation ticks per second */
	static constexpr int32 TicksPerSecond = 20;

	/** Most game time that is caught up in one frame after a hitch */
	static constexpr float MaxCatchUpSeconds = 0.25f;

	/** Returns the simulation of the given world, or nullptr outside of game worlds */
	static UFactorySimulationSubsystem* Get(const UObject* WorldContextObject);

	/** Converts seconds into whole simulation ticks (at least 1) */
	static int32 SecondsToTicks(float Seconds);

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Adds an object to the simulation. It is ticked after all objects registered before it */
	void Register(UObject* Object);

	/** Removes an object from the simulation. Safe to call during a simulation tick */
	void Unregister(UObject* Object);

	/** Returns the number of simulation ticks since the world started */
	int64 GetTickCount() const { return TickCount; }

	/**
	 *  Returns how far the game time is between the last and the next simulation tick (0-1).
	 *  Only for visuals, e.g. smooth progress bars. Never use it inside SimTick
	 */
	float GetTickAlpha() const { return FMath::Clamp(float(PendingSeconds * TicksPerSecond), 0.0f, 1.0f); }

	/** Returns the number of registered objects */
	int32 GetNumSimulated() const;

	/** Returns the average time one simulation tick takes, in milliseconds */
	double GetAverageTickMs() const { return AverageTickMs; }

	/** Returns the number of ticks that were skipped because of the catch-up limit */
	int64 GetDroppedTicks() const { return DroppedTicks; }

	/** Speeds up or slows down the simulation for testing (1 = real time). The catch-up limit scales along */
	void SetSpeed(float NewSpeed) { Speed = FMath::Clamp(NewSpeed, 0.0f, 100.0f); }

	float GetSpeed() const { return Speed; }

protected:

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:

	/** Runs one simulation tick on all registered objects */
	void RunTick();

	/** Registered objects in tick order. Unregistered entries are set to null and removed after the tick */
	UPROPERTY()
	TArray<TObjectPtr<UObject>> Simulated;

	/** Game time that has not been simulated yet */
	double PendingSeconds = 0.0;

	int64 TickCount = 0;

	int64 DroppedTicks = 0;

	double AverageTickMs = 0.0;

	float Speed = 1.0f;

	/** True while RunTick is iterating, so Unregister must not shrink the array */
	bool bTicking = false;
};
