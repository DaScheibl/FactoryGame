// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryHologram.h"
#include "FactoryBuildingTypes.h"
#include "Components/StaticMeshComponent.h"
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

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetStaticMesh(CubeFinder.Object);
	Mesh->SetMaterial(0, MaterialFinder.Object);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(false);
}

void AFactoryHologram::SetBuilding(const FFactoryBuildingDefinition& Building)
{
	// der Engine-Würfel ist 1 m groß und mittig; nach oben schieben, damit der Ursprung unten in der Mitte liegt
	const FVector Size = Building.GetSize();
	Mesh->SetRelativeScale3D(Size / 100.0);
	Mesh->SetRelativeLocation(FVector(0.0, 0.0, Size.Z * 0.5));

	if (!Material)
	{
		Material = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	for (UStaticMeshComponent* Marker : PortMarkers)
	{
		Marker->DestroyComponent();
	}

	PortMarkers.Reset();

	// gleiche Platten wie an der fertigen Maschine (AFactoryMachine::CreatePortMarkers)
	for (const FFactoryPort& Port : Building.GetPorts())
	{
		UStaticMeshComponent* Marker = NewObject<UStaticMeshComponent>(this);
		Marker->SetStaticMesh(Mesh->GetStaticMesh());
		Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Marker->SetCastShadow(false);
		Marker->SetupAttachment(Root);
		Marker->SetRelativeScale3D(FVector(0.3, 0.8, 0.1));
		Marker->SetRelativeLocation(Port.Location + Port.Direction * (Port.bInput ? -15.0 : 15.0) - FVector(0.0, 0.0, 5.0));
		Marker->SetMaterial(0, Mesh->GetMaterial(0)->GetBaseMaterial());
		Marker->RegisterComponent();

		if (UMaterialInstanceDynamic* MarkerMaterial = Marker->CreateAndSetMaterialInstanceDynamic(0))
		{
			MarkerMaterial->SetVectorParameterValue(TEXT("Color"), Port.bInput ? FLinearColor(0.05f, 0.25f, 0.8f) : FLinearColor(0.9f, 0.4f, 0.05f));
		}

		PortMarkers.Add(Marker);
	}

	ValidState = -1;
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
