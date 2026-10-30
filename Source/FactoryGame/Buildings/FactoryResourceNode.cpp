// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryResourceNode.h"
#include "FactoryMachine.h"
#include "FactoryBuildingDatabase.h"
#include "FactoryItemDatabase.h"
#include "FactoryLocalization.h"
#include "FactoryGame.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Height of the graybox disc in cm */
	constexpr double NodeHeight = 40.0;
}

AFactoryResourceNode::AFactoryResourceNode()
{
	// der Actor-Tick dreht nur den Text zur Kamera
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// Zylinder ist 1 m breit und 1 m hoch mit Mittelpunkt in der Mitte: auf 3 m × 40 cm skalieren und anheben
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	Mesh->SetStaticMesh(CylinderFinder.Object);
	Mesh->SetMaterial(0, MaterialFinder.Object);
	Mesh->SetWorldScale3D(FVector(Radius * 2.0 / 100.0, Radius * 2.0 / 100.0, NodeHeight / 100.0));

	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Mesh);
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetVerticalAlignment(EVRTA_TextBottom);
	Label->SetWorldSize(28.0f);
	Label->SetUsingAbsoluteScale(true);
	Label->SetRelativeLocation(FVector(0.0, 0.0, 250.0));
	Label->SetTextRenderColor(FColor::White);
	Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

AFactoryResourceNode* AFactoryResourceNode::SpawnNode(UWorld* World, const FVector& Location, FName ItemId, EFactoryNodePurity Purity)
{
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// der Zylinder ist mittig, also um die halbe Höhe anheben, damit er auf dem Boden liegt
	AFactoryResourceNode* Node = World->SpawnActor<AFactoryResourceNode>(Location + FVector(0.0, 0.0, NodeHeight * 0.5), FRotator::ZeroRotator, SpawnParams);

	if (Node)
	{
		Node->Setup(ItemId, Purity);
	}

	return Node;
}

void AFactoryResourceNode::Setup(FName InItemId, EFactoryNodePurity InPurity)
{
	ItemId = InItemId;
	Purity = InPurity;

	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);
	const FFactoryItemDefinition* Item = Items ? Items->FindItem(ItemId) : nullptr;

	if (UMaterialInstanceDynamic* Material = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Item ? FLinearColor(Item->PlaceholderColor) : FLinearColor::Gray);
	}

	Label->SetText(FText::Format(FText::FromString(TEXT("{0}\n{1}")), Items ? Items->GetDisplayName(ItemId) : FText::FromName(ItemId), GetPurityText(Purity)));
}

AFactoryResourceNode* AFactoryResourceNode::FindNodeNear(const UWorld* World, const FVector& Location, double MaxDistance)
{
	AFactoryResourceNode* Best = nullptr;
	double BestDistance = MaxDistance;

	if (!World)
	{
		return nullptr;
	}

	// nur eine Handvoll Vorkommen in der Welt, deshalb einfach alle durchgehen
	for (TActorIterator<AFactoryResourceNode> It(World); It; ++It)
	{
		const double Distance = FVector::Dist2D(It->GetActorLocation(), Location);

		if (Distance <= BestDistance)
		{
			Best = *It;
			BestDistance = Distance;
		}
	}

	return Best;
}

int32 AFactoryResourceNode::GetSpeedPercent(EFactoryNodePurity Purity)
{
	switch (Purity)
	{
	case EFactoryNodePurity::Impure:	return 50;
	case EFactoryNodePurity::Pure:		return 200;
	default:							return 100;
	}
}

FText AFactoryResourceNode::GetPurityText(EFactoryNodePurity Purity)
{
	switch (Purity)
	{
	case EFactoryNodePurity::Impure:	return FactoryLoc::Get(TEXT("Node_Impure"));
	case EFactoryNodePurity::Pure:		return FactoryLoc::Get(TEXT("Node_Pure"));
	default:							return FactoryLoc::Get(TEXT("Node_Normal"));
	}
}

FName AFactoryResourceNode::GetMiningRecipe() const
{
	FString Name = ItemId.ToString();
	Name.RemoveFromStart(TEXT("ITM_"));
	return FName(*(TEXT("RCP_Mine_") + Name));
}

AFactoryMachine* AFactoryResourceNode::FindDrill() const
{
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);

	for (TActorIterator<AFactoryMachine> It(GetWorld()); It; ++It)
	{
		const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(It->GetProduction()->GetBuildingId()) : nullptr;

		if (Building && Building->Type == EFactoryBuildingType::Drill && FVector::Dist2D(It->GetActorLocation(), GetActorLocation()) < Radius)
		{
			return *It;
		}
	}

	return nullptr;
}

void AFactoryResourceNode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Text zur Kamera drehen (nur um die Hochachse)
	if (const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		const FVector ToCamera = Camera->GetCameraLocation() - Label->GetComponentLocation();
		Label->SetWorldRotation(FRotator(0.0, ToCamera.Rotation().Yaw, 0.0));
	}
}

bool UFactoryResourceNodeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld() && Super::ShouldCreateSubsystem(Outer);
}

void UFactoryResourceNodeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!InWorld.GetMapName().Contains(TEXT("Lvl_Flat")))
	{
		return;
	}

	// feste Testbelegung rund um den Startpunkt (0, 0): Eisen im Osten, Kupfer im Westen, Kalkstein im Norden/Süden.
	// Positionen in Metern, jede Sorte in mehreren Reinheiten (Docs/Standards/15-abbau.md)
	struct FNodeSpot
	{
		double X;
		double Y;
		const TCHAR* ItemId;
		EFactoryNodePurity Purity;
	};

	static const FNodeSpot Spots[] = {
		{ 25.0,   0.0, TEXT("ITM_IronOre"),   EFactoryNodePurity::Normal },
		{ 35.0,  18.0, TEXT("ITM_IronOre"),   EFactoryNodePurity::Pure },
		{ 35.0, -18.0, TEXT("ITM_IronOre"),   EFactoryNodePurity::Impure },
		{ -25.0,  0.0, TEXT("ITM_CopperOre"), EFactoryNodePurity::Normal },
		{ -35.0, 18.0, TEXT("ITM_CopperOre"), EFactoryNodePurity::Pure },
		{ -35.0,-18.0, TEXT("ITM_CopperOre"), EFactoryNodePurity::Impure },
		{ 0.0,   30.0, TEXT("ITM_Limestone"), EFactoryNodePurity::Normal },
		{ 0.0,  -30.0, TEXT("ITM_Limestone"), EFactoryNodePurity::Pure },
	};

	for (const FNodeSpot& Spot : Spots)
	{
		// auf den Boden setzen (die Ebene liegt bei Z = 0, aber lieber messen)
		FVector Location(Spot.X * 100.0, Spot.Y * 100.0, 0.0);
		FHitResult Hit;

		if (InWorld.LineTraceSingleByChannel(Hit, Location + FVector(0.0, 0.0, 5000.0), Location - FVector(0.0, 0.0, 5000.0), ECC_Visibility))
		{
			Location.Z = Hit.ImpactPoint.Z;
		}

		AFactoryResourceNode::SpawnNode(&InWorld, Location, Spot.ItemId, Spot.Purity);
	}

	UE_LOG(LogFactoryGame, Log, TEXT("Placed %d resource nodes in the flat test world."), UE_ARRAY_COUNT(Spots));
}
