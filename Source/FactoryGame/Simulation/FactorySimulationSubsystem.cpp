// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactorySimulationSubsystem.h"
#include "FactorySimulated.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"

UFactorySimulationSubsystem* UFactorySimulationSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UFactorySimulationSubsystem>() : nullptr;
}

int32 UFactorySimulationSubsystem::SecondsToTicks(float Seconds)
{
	return FMath::Max(1, FMath::RoundToInt32(Seconds * TicksPerSecond));
}

bool UFactorySimulationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	// nur im laufenden Spiel (auch Play-in-Editor), nicht im Editor-Viewport oder in Vorschaufenstern
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UFactorySimulationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFactorySimulationSubsystem, STATGROUP_Tickables);
}

void UFactorySimulationSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	constexpr double SecondsPerTick = 1.0 / TicksPerSecond;

	PendingSeconds += DeltaTime * Speed;

	// nach einem Ruckler höchstens MaxCatchUpSeconds nachholen; der Rest verfällt
	const int32 MaxTicks = FMath::Max(1, FMath::CeilToInt32(MaxCatchUpSeconds * TicksPerSecond * FMath::Max(Speed, 1.0f)));
	int32 NumTicks = FMath::FloorToInt32(PendingSeconds / SecondsPerTick);

	if (NumTicks > MaxTicks)
	{
		DroppedTicks += NumTicks - MaxTicks;
		NumTicks = MaxTicks;
		PendingSeconds = 0.0;
	}
	else
	{
		PendingSeconds -= NumTicks * SecondsPerTick;
	}

	for (int32 Index = 0; Index < NumTicks; ++Index)
	{
		const double StartTime = FPlatformTime::Seconds();
		RunTick();

		// gleitender Mittelwert, damit die Anzeige in SimStats nicht springt
		const double TickMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;
		AverageTickMs = FMath::Lerp(AverageTickMs, TickMs, 0.05);
	}
}

void UFactorySimulationSubsystem::RunTick()
{
	++TickCount;
	bTicking = true;

	// Objekte, die während des Ticks dazukommen, sind erst im nächsten Tick dran
	const int32 NumToTick = Simulated.Num();

	for (int32 Index = 0; Index < NumToTick; ++Index)
	{
		if (IFactorySimulated* Object = Cast<IFactorySimulated>(Simulated[Index]))
		{
			Object->SimTick(TickCount);
		}
	}

	bTicking = false;

	// während des Ticks abgemeldete Objekte (auf null gesetzt) entfernen, Reihenfolge bleibt erhalten
	Simulated.RemoveAll([](const TObjectPtr<UObject>& Object) { return Object == nullptr; });
}

void UFactorySimulationSubsystem::Register(UObject* Object)
{
	if (ensureMsgf(Cast<IFactorySimulated>(Object), TEXT("Only IFactorySimulated objects can be registered")))
	{
		Simulated.AddUnique(Object);
	}
}

void UFactorySimulationSubsystem::Unregister(UObject* Object)
{
	const int32 Index = Simulated.Find(Object);

	if (Index == INDEX_NONE)
	{
		return;
	}

	if (bTicking)
	{
		Simulated[Index] = nullptr;
	}
	else
	{
		Simulated.RemoveAt(Index);
	}
}

int32 UFactorySimulationSubsystem::GetNumSimulated() const
{
	return Simulated.Num();
}
