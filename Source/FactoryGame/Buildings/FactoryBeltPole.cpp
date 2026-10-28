// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryBeltPole.h"
#include "FactoryBelt.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FLinearColor PoleColor(0.25f, 0.25f, 0.27f);
}

AFactoryBeltPole::AFactoryBeltPole()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	Pillar = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pillar"));
	Pillar->SetupAttachment(Root);
	Pillar->SetStaticMesh(CubeFinder.Object);
	Pillar->SetMaterial(0, MaterialFinder.Object);

	Plate = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plate"));
	Plate->SetupAttachment(Root);
	Plate->SetStaticMesh(CubeFinder.Object);
	Plate->SetMaterial(0, MaterialFinder.Object);

	UpdateMeshes();
}

void AFactoryBeltPole::BeginPlay()
{
	Super::BeginPlay();

	// eine Material-Instanz für Säule und Platte, damit beide gemeinsam rot werden können
	Material = Pillar->CreateAndSetMaterialInstanceDynamic(0);
	Plate->SetMaterial(0, Material);
	SetDemolishHighlight(false);
	UpdateMeshes();
}

void AFactoryBeltPole::SetHeight(int32 NewHeight)
{
	Height = FMath::Clamp(NewHeight, MinHeight, MaxHeight);
	UpdateMeshes();
}

void AFactoryBeltPole::UpdateMeshes()
{
	// der Engine-Würfel ist 1 m groß und mittig. Säule 0,3 m dick bis unter die Platte, Platte 0,8 x 0,8 x 0,1 m
	const double Top = Height * 100.0;
	Pillar->SetRelativeScale3D(FVector(0.3, 0.3, (Top - 10.0) / 100.0));
	Pillar->SetRelativeLocation(FVector(0.0, 0.0, (Top - 10.0) * 0.5));
	Plate->SetRelativeScale3D(FVector(0.8, 0.8, 0.1));
	Plate->SetRelativeLocation(FVector(0.0, 0.0, Top - 5.0));
}

FVector AFactoryBeltPole::GetConnectionLocation() const
{
	return GetActorLocation() + FVector(0.0, 0.0, Height * 100.0);
}

void AFactoryBeltPole::SetDemolishHighlight(bool bHighlighted)
{
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), bHighlighted ? FLinearColor(1.0f, 0.02f, 0.02f) : PoleColor);
	}
}
