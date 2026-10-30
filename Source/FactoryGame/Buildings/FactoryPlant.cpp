// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryPlant.h"
#include "FactoryItemDatabase.h"
#include "FactoryLocalization.h"
#include "FactoryGame.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FLinearColor LeafColor(0.12f, 0.45f, 0.1f);
	const FLinearColor BarkColor(0.3f, 0.17f, 0.07f);

	void SetPartColor(UStaticMeshComponent* Part, const FLinearColor& Color)
	{
		if (UMaterialInstanceDynamic* Material = Part->CreateAndSetMaterialInstanceDynamic(0))
		{
			Material->SetVectorParameterValue(TEXT("Color"), Color);
		}
	}
}

AFactoryPlant::AFactoryPlant()
{
	// tickt nur, solange die Pflanze nachwächst
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	Crown = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Crown"));
	Crown->SetupAttachment(Root);
	Crown->SetStaticMesh(SphereFinder.Object);
	Crown->SetMaterial(0, MaterialFinder.Object);

	Trunk = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Trunk"));
	Trunk->SetupAttachment(Root);
	Trunk->SetStaticMesh(CylinderFinder.Object);
	Trunk->SetMaterial(0, MaterialFinder.Object);

	Stump = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Stump"));
	Stump->SetupAttachment(Root);
	Stump->SetStaticMesh(CylinderFinder.Object);
	Stump->SetMaterial(0, MaterialFinder.Object);
	Stump->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

AFactoryPlant* AFactoryPlant::SpawnPlant(UWorld* World, const FVector& Location, EFactoryPlantType Type, int32 InSpot)
{
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// jede Pflanze etwas anders gedreht, damit die Reihe nicht wie kopiert aussieht
	AFactoryPlant* Plant = World->SpawnActor<AFactoryPlant>(Location, FRotator(0.0, InSpot * 47.0, 0.0), SpawnParams);

	if (Plant)
	{
		Plant->PlantType = Type;
		Plant->Spot = InSpot;

		// Engine-Kugel und -Zylinder sind 1 m groß und mittig
		if (Type == EFactoryPlantType::Tree)
		{
			Plant->Trunk->SetRelativeScale3D(FVector(0.5, 0.5, 4.0));
			Plant->Trunk->SetRelativeLocation(FVector(0.0, 0.0, 200.0));
			Plant->Crown->SetRelativeScale3D(FVector(3.0, 3.0, 3.5));
			Plant->Crown->SetRelativeLocation(FVector(0.0, 0.0, 520.0));
			Plant->Stump->SetRelativeScale3D(FVector(0.55, 0.55, 0.4));
			SetPartColor(Plant->Stump, BarkColor);
		}
		else
		{
			Plant->Trunk->SetVisibility(false);
			Plant->Trunk->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Plant->Crown->SetRelativeScale3D(FVector(1.8, 1.6, 1.3));
			Plant->Crown->SetRelativeLocation(FVector(0.0, 0.0, 55.0));
			Plant->Stump->SetRelativeScale3D(FVector(0.6, 0.6, 0.15));
			SetPartColor(Plant->Stump, LeafColor * 0.6f);
		}

		Plant->Stump->SetRelativeLocation(FVector(0.0, 0.0, Plant->Stump->GetRelativeScale3D().Z * 50.0));
		SetPartColor(Plant->Crown, LeafColor);
		SetPartColor(Plant->Trunk, BarkColor);
		Plant->UpdateVisual();
	}

	return Plant;
}

FName AFactoryPlant::GetItemId() const
{
	return PlantType == EFactoryPlantType::Tree ? FName(TEXT("ITM_Wood")) : FName(TEXT("ITM_Leaves"));
}

int32 AFactoryPlant::GetHarvestAmount() const
{
	return PlantType == EFactoryPlantType::Tree ? 4 : 5;
}

float AFactoryPlant::GetHarvestTime() const
{
	return PlantType == EFactoryPlantType::Tree ? 4.0f : 1.5f;
}

FText AFactoryPlant::GetDisplayName() const
{
	return FactoryLoc::Get(PlantType == EFactoryPlantType::Tree ? TEXT("Plant_Tree") : TEXT("Plant_Bush"));
}

void AFactoryPlant::Harvest()
{
	SetRegrowRemaining(RegrowTime);
}

void AFactoryPlant::SetRegrowRemaining(float Seconds)
{
	RegrowRemaining = FMath::Max(0.0f, Seconds);
	SetActorTickEnabled(IsHarvested());
	UpdateVisual();
}

void AFactoryPlant::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Spielzeit: im Pause-Menü tickt nichts, also wächst dann auch nichts
	if (IsHarvested())
	{
		SetRegrowRemaining(RegrowRemaining - DeltaSeconds);
	}
}

void AFactoryPlant::UpdateVisual()
{
	const bool bGrown = !IsHarvested();
	const bool bTree = PlantType == EFactoryPlantType::Tree;

	Crown->SetVisibility(bGrown);
	Crown->SetCollisionEnabled(bGrown ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	Trunk->SetVisibility(bGrown && bTree);
	Trunk->SetCollisionEnabled(bGrown && bTree ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	Stump->SetVisibility(!bGrown);
}

UFactoryPlantSubsystem* UFactoryPlantSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UFactoryPlantSubsystem>() : nullptr;
}

bool UFactoryPlantSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld() && Super::ShouldCreateSubsystem(Outer);
}

void UFactoryPlantSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!InWorld.GetMapName().Contains(TEXT("Lvl_Flat")))
	{
		return;
	}

	// feste Testbelegung in Metern: Büsche nah am Start, Bäume weiter draußen, frei von Vorkommen und HUB.
	// Die Reihenfolge ist der Index im Spielstand, neue Pflanzen deshalb nur hinten anhängen
	struct FPlantSpot
	{
		double X;
		double Y;
		EFactoryPlantType Type;
	};

	static const FPlantSpot Spots[] = {
		{ -10.0, -10.0, EFactoryPlantType::Bush }, { -12.0,  -6.0, EFactoryPlantType::Bush }, {  -8.0, -14.0, EFactoryPlantType::Bush },
		{  10.0, -10.0, EFactoryPlantType::Bush }, {  12.0,  -6.0, EFactoryPlantType::Bush }, {   8.0, -14.0, EFactoryPlantType::Bush },
		{ -15.0,  10.0, EFactoryPlantType::Bush }, {  15.0,  10.0, EFactoryPlantType::Bush }, {  -6.0,  24.0, EFactoryPlantType::Bush },
		{   6.0,  24.0, EFactoryPlantType::Bush },
		{ -20.0, -30.0, EFactoryPlantType::Tree }, { -12.0, -36.0, EFactoryPlantType::Tree }, {  12.0, -36.0, EFactoryPlantType::Tree },
		{  20.0, -30.0, EFactoryPlantType::Tree }, { -22.0,  36.0, EFactoryPlantType::Tree }, {  22.0,  36.0, EFactoryPlantType::Tree },
		{ -50.0,   8.0, EFactoryPlantType::Tree }, {  50.0,  -8.0, EFactoryPlantType::Tree },
	};

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Spots); ++Index)
	{
		FVector Location(Spots[Index].X * 100.0, Spots[Index].Y * 100.0, 0.0);
		FHitResult Hit;

		if (InWorld.LineTraceSingleByChannel(Hit, Location + FVector(0.0, 0.0, 5000.0), Location - FVector(0.0, 0.0, 5000.0), ECC_Visibility))
		{
			Location.Z = Hit.ImpactPoint.Z;
		}

		Plants.Add(AFactoryPlant::SpawnPlant(&InWorld, Location, Spots[Index].Type, Index));
	}

	ApplySaveData();
	UE_LOG(LogFactoryGame, Log, TEXT("Placed %d plants in the flat test world."), Plants.Num());
}

TArray<FFactorySavedPlant> UFactoryPlantSubsystem::GetSaveData() const
{
	// noch nicht platziert (Laden läuft gerade): den geladenen Stand unverändert zurückgeben
	if (bHasPendingData)
	{
		return PendingData;
	}

	TArray<FFactorySavedPlant> Data;

	for (const AFactoryPlant* Plant : Plants)
	{
		if (Plant && Plant->IsHarvested())
		{
			FFactorySavedPlant& Saved = Data.AddDefaulted_GetRef();
			Saved.Spot = Plant->GetSpot();
			Saved.RegrowSeconds = Plant->GetRegrowRemaining();
		}
	}

	return Data;
}

void UFactoryPlantSubsystem::LoadSaveData(const TArray<FFactorySavedPlant>& Data)
{
	PendingData = Data;
	bHasPendingData = true;

	if (Plants.Num() > 0)
	{
		ApplySaveData();
	}
}

void UFactoryPlantSubsystem::ApplySaveData()
{
	if (!bHasPendingData)
	{
		return;
	}

	for (const FFactorySavedPlant& Saved : PendingData)
	{
		if (Plants.IsValidIndex(Saved.Spot) && Plants[Saved.Spot])
		{
			Plants[Saved.Spot]->SetRegrowRemaining(Saved.RegrowSeconds);
		}
	}

	PendingData.Reset();
	bHasPendingData = false;
}
