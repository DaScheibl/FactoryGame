// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryPowerCable.h"
#include "FactoryPowerPole.h"
#include "FactoryPowerSubsystem.h"
#include "FactoryMachine.h"
#include "FactoryBuildingDatabase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

const FName AFactoryPowerCable::CableId(TEXT("LOG_PowerCable"));

namespace
{
	/** Thickness of the graybox wire in cm */
	constexpr double WireThickness = 6.0;

	const FLinearColor CableColor(0.03f, 0.03f, 0.035f);

	const FFactoryBuildingDefinition* FindNodeBuilding(const AActor* Node)
	{
		const UFactoryBuildingDatabase* Buildings = Node ? UFactoryBuildingDatabase::Get(Node) : nullptr;

		if (!Buildings)
		{
			return nullptr;
		}

		if (const AFactoryMachine* Machine = Cast<AFactoryMachine>(Node))
		{
			return Buildings->FindBuilding(Machine->GetProduction()->GetBuildingId());
		}

		if (const AFactoryPowerPole* Pole = Cast<AFactoryPowerPole>(Node))
		{
			return Buildings->FindBuilding(Pole->GetBuildingId());
		}

		return nullptr;
	}
}

AFactoryPowerCable::AFactoryPowerCable()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	Wire = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wire"));
	Wire->SetupAttachment(Root);
	Wire->SetStaticMesh(CubeFinder.Object);
	Wire->SetMaterial(0, MaterialFinder.Object);
	Wire->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Wire->SetCastShadow(false);
}

void AFactoryPowerCable::BeginPlay()
{
	Super::BeginPlay();

	if (UFactoryPowerSubsystem* Power = UFactoryPowerSubsystem::Get(this))
	{
		Power->MarkDirty();
	}
}

void AFactoryPowerCable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFactoryPowerSubsystem* Power = UFactoryPowerSubsystem::Get(this))
	{
		Power->MarkDirty();
	}

	Super::EndPlay(EndPlayReason);
}

void AFactoryPowerCable::Connect(AActor* InEndA, AActor* InEndB)
{
	EndA = InEndA;
	EndB = InEndB;
	UpdateVisual();

	if (UFactoryPowerSubsystem* Power = UFactoryPowerSubsystem::Get(this))
	{
		Power->MarkDirty();
	}
}

AActor* AFactoryPowerCable::GetOtherEnd(const AActor* Node) const
{
	if (Node && EndA.Get() == Node)
	{
		return EndB.Get();
	}

	if (Node && EndB.Get() == Node)
	{
		return EndA.Get();
	}

	return nullptr;
}

double AFactoryPowerCable::GetLength() const
{
	const AActor* A = EndA.Get();
	const AActor* B = EndB.Get();
	return A && B ? FVector::Dist(GetConnectionPoint(A), GetConnectionPoint(B)) : 0.0;
}

FTransform AFactoryPowerCable::GetCableTransform(const FVector& From, const FVector& To, double Thickness)
{
	const FVector Delta = To - From;
	const double Length = FMath::Max(Delta.Size(), 1.0);
	return FTransform(Delta.ToOrientationQuat(), (From + To) * 0.5, FVector(Length, Thickness, Thickness) / 100.0);
}

void AFactoryPowerCable::UpdateVisual()
{
	const AActor* A = EndA.Get();
	const AActor* B = EndB.Get();

	if (!A || !B)
	{
		return;
	}

	// das Kabel liegt in Weltkoordinaten; der Actor steht am Anfang, damit Abriss-Kisten dort landen
	SetActorLocation(GetConnectionPoint(A));
	Wire->SetWorldTransform(GetCableTransform(GetConnectionPoint(A), GetConnectionPoint(B), WireThickness));

	if (!Material)
	{
		Material = Wire->CreateAndSetMaterialInstanceDynamic(0);
	}

	SetDemolishHighlight(false);
}

void AFactoryPowerCable::SetDemolishHighlight(bool bHighlighted)
{
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), bHighlighted ? FLinearColor(1.0f, 0.02f, 0.02f) : CableColor);
	}
}

TArray<FFactoryItemStack> AFactoryPowerCable::GetCosts(const UObject* WorldContextObject, double Length)
{
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(WorldContextObject);
	const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(CableId) : nullptr;
	TArray<FFactoryItemStack> Costs = Building ? Building->GetCosts() : TArray<FFactoryItemStack>();

	// Kosten in der Tabelle gelten pro angefangene 5 m
	const int32 Pieces = FMath::Max(1, FMath::CeilToInt32(Length / CostLength));

	for (FFactoryItemStack& Cost : Costs)
	{
		Cost.Count *= Pieces;
	}

	return Costs;
}

bool AFactoryPowerCable::IsPowerNode(const AActor* Actor)
{
	return GetMaxConnections(Actor) > 0;
}

FVector AFactoryPowerCable::GetConnectionPoint(const AActor* Node)
{
	if (const AFactoryPowerPole* Pole = Cast<AFactoryPowerPole>(Node))
	{
		return Pole->GetConnectionPoint();
	}

	// Maschinen: Anschluss oben in der Mitte des Dachs
	const FFactoryBuildingDefinition* Building = FindNodeBuilding(Node);
	const double Height = Building ? Building->GetSize().Z : 400.0;
	return Node ? Node->GetActorLocation() + Node->GetActorUpVector() * Height : FVector::ZeroVector;
}

int32 AFactoryPowerCable::GetMaxConnections(const AActor* Node)
{
	if (const AFactoryPowerPole* Pole = Cast<AFactoryPowerPole>(Node))
	{
		return Pole->GetMaxConnections();
	}

	const FFactoryBuildingDefinition* Building = FindNodeBuilding(Node);
	return Building ? Building->PowerConnections : 0;
}

TArray<AFactoryPowerCable*> AFactoryPowerCable::GetCables(const AActor* Node)
{
	TArray<AFactoryPowerCable*> Cables;

	if (!Node || !Node->GetWorld())
	{
		return Cables;
	}

	for (TActorIterator<AFactoryPowerCable> It(Node->GetWorld()); It; ++It)
	{
		if (It->GetEndA() == Node || It->GetEndB() == Node)
		{
			Cables.Add(*It);
		}
	}

	return Cables;
}

bool AFactoryPowerCable::AreConnected(const AActor* A, const AActor* B)
{
	for (const AFactoryPowerCable* Cable : GetCables(A))
	{
		if (Cable->GetOtherEnd(A) == B)
		{
			return true;
		}
	}

	return false;
}

AFactoryPowerCable* AFactoryPowerCable::FindCableNearRay(const UWorld* World, const FVector& Start, const FVector& Direction, double MaxDistance, double Tolerance, double& OutDistance)
{
	AFactoryPowerCable* Best = nullptr;
	double BestGap = Tolerance;
	OutDistance = MaxDistance;

	if (!World)
	{
		return nullptr;
	}

	const FVector RayEnd = Start + Direction.GetSafeNormal() * MaxDistance;

	for (TActorIterator<AFactoryPowerCable> It(World); It; ++It)
	{
		const AActor* A = It->GetEndA();
		const AActor* B = It->GetEndB();

		if (!A || !B)
		{
			continue;
		}

		// kürzester Abstand zwischen Blickstrahl und Kabelstück
		FVector OnRay;
		FVector OnCable;
		FMath::SegmentDistToSegmentSafe(Start, RayEnd, GetConnectionPoint(A), GetConnectionPoint(B), OnRay, OnCable);
		const double Gap = FVector::Dist(OnRay, OnCable);

		if (Gap <= BestGap)
		{
			Best = *It;
			BestGap = Gap;
			OutDistance = FVector::Dist(Start, OnRay);
		}
	}

	return Best;
}
