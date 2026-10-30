// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryFoundation.h"
#include "FactoryBuildingDatabase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Thickness of the ramp slab in cm */
	constexpr double RampThickness = 20.0;

	const FLinearColor FoundationColor(0.32f, 0.33f, 0.36f);
}

AFactoryFoundation::AFactoryFoundation()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetStaticMesh(CubeFinder.Object);
	Mesh->SetMaterial(0, MaterialFinder.Object);
}

TArray<FTransform> AFactoryFoundation::GetPieces(const FFactoryBuildingDefinition& Building)
{
	const FVector Size = Building.GetSize();

	if (Building.Type != EFactoryBuildingType::Ramp)
	{
		return { FTransform(FQuat::Identity, FVector(0.0, 0.0, Size.Z * 0.5), Size / 100.0) };
	}

	// Oberkante läuft von (-X, 0) bis (+X, volle Höhe); die Platte hängt um ihre halbe Dicke darunter
	const double Pitch = FMath::Atan2(Size.Z, Size.X);
	const double Length = FMath::Sqrt(Size.X * Size.X + Size.Z * Size.Z);
	const FQuat Rotation = FRotator(FMath::RadiansToDegrees(Pitch), 0.0, 0.0).Quaternion();
	const FVector TopCenter(0.0, 0.0, Size.Z * 0.5);
	const FVector Center = TopCenter - Rotation.GetUpVector() * RampThickness * 0.5;

	return { FTransform(Rotation, Center, FVector(Length, Size.Y, RampThickness) / 100.0) };
}

void AFactoryFoundation::InitBuilding(FName InBuildingId)
{
	BuildingId = InBuildingId;

	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(BuildingId) : nullptr;

	if (!Building)
	{
		return;
	}

	bRamp = Building->Type == EFactoryBuildingType::Ramp;
	Height = Building->GetSize().Z;

	const TArray<FTransform> Pieces = GetPieces(*Building);

	if (Pieces.Num() > 0)
	{
		Mesh->SetRelativeTransform(Pieces[0]);
	}

	if (!Material)
	{
		Material = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	SetDemolishHighlight(false);
}

void AFactoryFoundation::SetDemolishHighlight(bool bHighlighted)
{
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), bHighlighted ? FLinearColor(1.0f, 0.02f, 0.02f) : FoundationColor);
	}
}
