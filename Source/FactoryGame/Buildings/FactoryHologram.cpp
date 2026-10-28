// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryHologram.h"
#include "FactoryBuildingTypes.h"
#include "FactoryBeltPath.h"
#include "FactoryBelt.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AFactoryHologram::AFactoryHologram()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	CubeMesh = CubeFinder.Object;
	BaseMaterial = MaterialFinder.Object;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetStaticMesh(CubeMesh);
	Mesh->SetMaterial(0, BaseMaterial);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(false);

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(Root);
}

UStaticMeshComponent* AFactoryHologram::CreatePart()
{
	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
	Part->SetStaticMesh(CubeMesh);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetCastShadow(false);
	Part->SetupAttachment(Root);
	Part->RegisterComponent();
	return Part;
}

void AFactoryHologram::HideAll()
{
	if (!Material)
	{
		Material = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		Mesh->SetMaterial(0, Material);
		ValidState = -1;
	}

	Mesh->SetVisibility(false);

	for (UStaticMeshComponent* Marker : PortMarkers)
	{
		Marker->DestroyComponent();
	}

	PortMarkers.Reset();

	for (UStaticMeshComponent* Part : PoleMeshes)
	{
		Part->SetVisibility(false);
	}

	for (USplineMeshComponent* Piece : BeltMeshes)
	{
		Piece->SetVisibility(false);
	}
}

void AFactoryHologram::SetBuilding(const FFactoryBuildingDefinition& Building)
{
	HideAll();

	// der Engine-Würfel ist 1 m groß und mittig; nach oben schieben, damit der Ursprung unten in der Mitte liegt
	const FVector Size = Building.GetSize();
	Mesh->SetVisibility(true);
	Mesh->SetRelativeScale3D(Size / 100.0);
	Mesh->SetRelativeLocation(FVector(0.0, 0.0, Size.Z * 0.5));

	// gleiche Platten wie an der fertigen Maschine (AFactoryMachine::CreatePortMarkers)
	for (const FFactoryPort& Port : Building.GetPorts())
	{
		UStaticMeshComponent* Marker = CreatePart();
		Marker->SetRelativeScale3D(FVector(0.3, 0.8, 0.1));
		Marker->SetRelativeLocation(Port.Location + Port.Direction * (Port.bInput ? -15.0 : 15.0) - FVector(0.0, 0.0, 5.0));

		if (UMaterialInstanceDynamic* MarkerMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Marker))
		{
			MarkerMaterial->SetVectorParameterValue(TEXT("Color"), Port.bInput ? FLinearColor(0.05f, 0.25f, 0.8f) : FLinearColor(0.9f, 0.4f, 0.05f));
			Marker->SetMaterial(0, MarkerMaterial);
		}

		PortMarkers.Add(Marker);
	}
}

void AFactoryHologram::SetPole(int32 Height)
{
	HideAll();
	ShowPoleMeshes(0, FTransform::Identity, Height);
}

void AFactoryHologram::SetBelt(const FFactoryBeltPath* Path, const TArray<FTransform>& NewPoles)
{
	HideAll();

	for (int32 Index = 0; Index < NewPoles.Num(); ++Index)
	{
		ShowPoleMeshes(Index, FTransform(NewPoles[Index].GetRotation(), NewPoles[Index].GetLocation()), FMath::RoundToInt32(NewPoles[Index].GetScale3D().Z));
	}

	if (Path && Path->Points.Num() >= 2)
	{
		AFactoryBelt::ApplyPathToSpline(Spline, *Path, GetActorLocation());
		AFactoryBelt::BuildBodyMeshes(this, Spline, BeltMeshes, CubeMesh, Material, nullptr);

		for (USplineMeshComponent* Piece : BeltMeshes)
		{
			Piece->SetVisibility(true);
		}
	}
}

void AFactoryHologram::ShowPoleMeshes(int32 Index, const FTransform& Foot, int32 Height)
{
	while (PoleMeshes.Num() < (Index + 1) * 2)
	{
		UStaticMeshComponent* Part = CreatePart();
		Part->SetMaterial(0, Material);
		PoleMeshes.Add(Part);
	}

	// gleiche Maße wie AFactoryBeltPole: Säule 0,3 m, Platte 0,8 x 0,8 x 0,1 m, Oberkante = Bandhöhe
	const double Top = Height * 100.0;
	UStaticMeshComponent* Pillar = PoleMeshes[Index * 2];
	UStaticMeshComponent* Plate = PoleMeshes[Index * 2 + 1];

	Pillar->SetRelativeTransform(FTransform(Foot.GetRotation(), Foot.TransformPosition(FVector(0.0, 0.0, (Top - 10.0) * 0.5)), FVector(0.3, 0.3, (Top - 10.0) / 100.0)));
	Plate->SetRelativeTransform(FTransform(Foot.GetRotation(), Foot.TransformPosition(FVector(0.0, 0.0, Top - 5.0)), FVector(0.8, 0.8, 0.1)));
	Pillar->SetVisibility(true);
	Plate->SetVisibility(true);
}

void AFactoryHologram::SetValid(bool bValid)
{
	if (ValidState == int32(bValid) || !Material)
	{
		return;
	}

	ValidState = bValid;
	Material->SetVectorParameterValue(TEXT("Color"), bValid ? FLinearColor(0.1f, 0.7f, 0.25f) : FLinearColor(0.8f, 0.05f, 0.05f));
}
