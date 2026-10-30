// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryPowerPole.h"
#include "FactoryBuildingDatabase.h"
#include "FactoryPowerSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Width of the mast in cm */
	constexpr double MastWidth = 25.0;

	const FLinearColor PowerPoleColor(0.55f, 0.45f, 0.12f);
}

AFactoryPowerPole::AFactoryPowerPole()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	Mast = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mast"));
	Mast->SetupAttachment(Root);
	Mast->SetStaticMesh(CubeFinder.Object);
	Mast->SetMaterial(0, MaterialFinder.Object);

	Crossbar = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Crossbar"));
	Crossbar->SetupAttachment(Root);
	Crossbar->SetStaticMesh(CubeFinder.Object);
	Crossbar->SetMaterial(0, MaterialFinder.Object);
}

void AFactoryPowerPole::InitBuilding(FName InBuildingId)
{
	BuildingId = InBuildingId;

	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);

	if (const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(BuildingId) : nullptr)
	{
		Height = Building->GetSize().Z;
		MaxConnections = Building->PowerConnections;
	}

	// der Engine-Würfel ist 1 m groß und mittig
	Mast->SetRelativeScale3D(FVector(MastWidth, MastWidth, Height) / 100.0);
	Mast->SetRelativeLocation(FVector(0.0, 0.0, Height * 0.5));
	Crossbar->SetRelativeScale3D(FVector(0.15, 0.9, 0.15));
	Crossbar->SetRelativeLocation(FVector(0.0, 0.0, Height - 25.0));

	if (Materials.IsEmpty())
	{
		Materials.Add(Mast->CreateAndSetMaterialInstanceDynamic(0));
		Materials.Add(Crossbar->CreateAndSetMaterialInstanceDynamic(0));
	}

	SetDemolishHighlight(false);
}

FVector AFactoryPowerPole::GetConnectionPoint() const
{
	return GetActorLocation() + GetActorUpVector() * (Height - 25.0);
}

void AFactoryPowerPole::SetDemolishHighlight(bool bHighlighted)
{
	for (UMaterialInstanceDynamic* Material : Materials)
	{
		if (Material)
		{
			Material->SetVectorParameterValue(TEXT("Color"), bHighlighted ? FLinearColor(1.0f, 0.02f, 0.02f) : PowerPoleColor);
		}
	}
}

void AFactoryPowerPole::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFactoryPowerSubsystem* Power = UFactoryPowerSubsystem::Get(this))
	{
		Power->MarkDirty();
	}

	Super::EndPlay(EndPlayReason);
}
