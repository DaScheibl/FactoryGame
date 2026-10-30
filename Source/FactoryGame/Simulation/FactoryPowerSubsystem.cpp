// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryPowerSubsystem.h"
#include "FactoryPowerCable.h"
#include "FactoryPowerPole.h"
#include "FactoryMachine.h"
#include "FactoryProductionComponent.h"
#include "FactoryGamePlayerController.h"
#include "FactoryHUDWidget.h"
#include "FactoryLocalization.h"
#include "FactoryGame.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UFactoryPowerSubsystem* UFactoryPowerSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UFactoryPowerSubsystem>() : nullptr;
}

bool UFactoryPowerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld() && Super::ShouldCreateSubsystem(Outer);
}

void UFactoryPowerSubsystem::Rebuild()
{
	bDirty = false;
	Networks.Reset();
	NetworkOfNode.Reset();
	Unconnected.Reset();

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	// alle Knoten (Maschinen + Masten) durchnummerieren, dann über die Kabel zusammenfassen (Union-Find)
	TArray<AActor*> Nodes;
	TMap<const AActor*, int32> NodeIndices;

	for (TActorIterator<AFactoryMachine> It(World); It; ++It)
	{
		NodeIndices.Add(*It, Nodes.Add(*It));
	}

	for (TActorIterator<AFactoryPowerPole> It(World); It; ++It)
	{
		NodeIndices.Add(*It, Nodes.Add(*It));
	}

	TArray<int32> Parents;
	TArray<int32> NumCables;
	Parents.SetNum(Nodes.Num());
	NumCables.SetNumZeroed(Nodes.Num());

	for (int32 Index = 0; Index < Nodes.Num(); ++Index)
	{
		Parents[Index] = Index;
	}

	auto FindRoot = [&Parents](int32 Index)
	{
		while (Parents[Index] != Index)
		{
			Parents[Index] = Parents[Parents[Index]];
			Index = Parents[Index];
		}

		return Index;
	};

	for (TActorIterator<AFactoryPowerCable> It(World); It; ++It)
	{
		const int32* A = NodeIndices.Find(It->GetEndA());
		const int32* B = NodeIndices.Find(It->GetEndB());

		if (A && B && *A != *B)
		{
			++NumCables[*A];
			++NumCables[*B];
			Parents[FindRoot(*A)] = FindRoot(*B);
		}
	}

	TMap<int32, int32> NetworkOfRoot;

	for (int32 Index = 0; Index < Nodes.Num(); ++Index)
	{
		AFactoryMachine* Machine = Cast<AFactoryMachine>(Nodes[Index]);

		if (NumCables[Index] == 0)
		{
			if (Machine)
			{
				Unconnected.Add(Machine->GetProduction());
			}

			continue;
		}

		const int32 Root = FindRoot(Index);
		int32* NetworkIndex = NetworkOfRoot.Find(Root);

		if (!NetworkIndex)
		{
			NetworkIndex = &NetworkOfRoot.Add(Root, Networks.AddDefaulted());
		}

		FNetwork& Network = Networks[*NetworkIndex];
		NetworkOfNode.Add(Nodes[Index], *NetworkIndex);

		if (!Machine)
		{
			++Network.NumPoles;
		}
		else if (Machine->GetProduction()->IsGenerator())
		{
			Network.Generators.Add(Machine->GetProduction());
		}
		else
		{
			Network.Consumers.Add(Machine->GetProduction());
		}
	}
}

void UFactoryPowerSubsystem::SimTick(int64 Tick)
{
	if (bDirty)
	{
		Rebuild();
	}

	for (const TWeakObjectPtr<UFactoryProductionComponent>& Machine : Unconnected)
	{
		if (UFactoryProductionComponent* Production = Machine.Get())
		{
			Production->SetPowered(bFreePower);
			Production->SetGeneratorLoad(0);
		}
	}

	bool bAnyTripped = false;

	for (FNetwork& Network : Networks)
	{
		FFactoryPowerStats& Stats = Network.Stats;
		Stats = FFactoryPowerStats();

		for (const TWeakObjectPtr<UFactoryProductionComponent>& Generator : Network.Generators)
		{
			if (const UFactoryProductionComponent* Production = Generator.Get())
			{
				++Stats.NumGenerators;
				Stats.MaxOutput += Production->GetPowerMW();
				Stats.Capacity += Production->CanGenerate() ? Production->GetPowerMW() : 0;
				Stats.bTripped |= Production->IsFuseTripped();
			}
		}

		for (const TWeakObjectPtr<UFactoryProductionComponent>& Consumer : Network.Consumers)
		{
			if (const UFactoryProductionComponent* Production = Consumer.Get())
			{
				++Stats.NumConsumers;
				Stats.Demand += Production->WantsToRun() ? Production->GetPowerMW() : 0;
			}
		}

		// Überlast: die Sicherung fliegt raus, alle Generatoren des Netzes merken sich das (wird gespeichert)
		if (!Stats.bTripped && !bFreePower && Stats.Capacity > 0 && Stats.Demand > Stats.Capacity)
		{
			Stats.bTripped = true;
			bAnyTripped = true;

			for (const TWeakObjectPtr<UFactoryProductionComponent>& Generator : Network.Generators)
			{
				if (UFactoryProductionComponent* Production = Generator.Get())
				{
					Production->SetFuseTripped(true);
				}
			}

			UE_LOG(LogFactoryGame, Log, TEXT("Power: fuse tripped at tick %lld (%d MW needed, %d MW available)."), Tick, Stats.Demand, Stats.Capacity);
		}

		const bool bPowered = bFreePower || (!Stats.bTripped && Stats.Capacity > 0 && Stats.Demand <= Stats.Capacity);

		if (bPowered && !bFreePower && Stats.Demand > 0)
		{
			Stats.LoadPermille = FMath::Max(1, Stats.Demand * 1000 / Stats.Capacity);
		}

		for (const TWeakObjectPtr<UFactoryProductionComponent>& Generator : Network.Generators)
		{
			if (UFactoryProductionComponent* Production = Generator.Get())
			{
				Production->SetGeneratorLoad(Production->CanGenerate() ? Stats.LoadPermille : 0);
			}
		}

		for (const TWeakObjectPtr<UFactoryProductionComponent>& Consumer : Network.Consumers)
		{
			if (UFactoryProductionComponent* Production = Consumer.Get())
			{
				Production->SetPowered(bPowered);
			}
		}
	}

	if (bAnyTripped)
	{
		NotifyTripped();
	}
}

bool UFactoryPowerSubsystem::GetStats(const AActor* Node, FFactoryPowerStats& OutStats) const
{
	const int32* NetworkIndex = NetworkOfNode.Find(Node);

	if (!NetworkIndex || !Networks.IsValidIndex(*NetworkIndex))
	{
		OutStats = FFactoryPowerStats();
		return false;
	}

	OutStats = Networks[*NetworkIndex].Stats;
	return true;
}

bool UFactoryPowerSubsystem::ResetFuse(const AActor* Generator)
{
	const int32* NetworkIndex = NetworkOfNode.Find(Generator);

	if (!NetworkIndex || !Networks.IsValidIndex(*NetworkIndex))
	{
		return false;
	}

	FNetwork& Network = Networks[*NetworkIndex];

	// nur mit genug Leistung, sonst flöge sie im nächsten Tick gleich wieder raus
	if (Network.Stats.Demand > Network.Stats.Capacity)
	{
		return false;
	}

	for (const TWeakObjectPtr<UFactoryProductionComponent>& Production : Network.Generators)
	{
		if (Production.IsValid())
		{
			Production->SetFuseTripped(false);
		}
	}

	Network.Stats.bTripped = false;
	return true;
}

void UFactoryPowerSubsystem::NotifyTripped() const
{
	const AFactoryGamePlayerController* PlayerController = GetWorld() ? Cast<AFactoryGamePlayerController>(GetWorld()->GetFirstPlayerController()) : nullptr;

	if (UFactoryHUDWidget* HUD = PlayerController ? PlayerController->GetHUDWidget() : nullptr)
	{
		HUD->ShowNotification(FactoryLoc::Get(TEXT("Power_FuseTripped")), EFactoryNotificationType::Warning);
	}
}

void UFactoryPowerSubsystem::LogNetworks() const
{
	UE_LOG(LogFactoryGame, Log, TEXT("Power: %d networks, %d machines without cable%s."), Networks.Num(), Unconnected.Num(), bFreePower ? TEXT(", FreePower on") : TEXT(""));

	for (int32 Index = 0; Index < Networks.Num(); ++Index)
	{
		const FNetwork& Network = Networks[Index];
		UE_LOG(LogFactoryGame, Log, TEXT("  #%d: %d generators, %d machines, %d poles · %d / %d MW (max %d) · load %d %%%s"),
			Index, Network.Stats.NumGenerators, Network.Stats.NumConsumers, Network.NumPoles, Network.Stats.Demand, Network.Stats.Capacity,
			Network.Stats.MaxOutput, Network.Stats.LoadPermille / 10, Network.Stats.bTripped ? TEXT(" · FUSE TRIPPED") : TEXT(""));
	}
}
