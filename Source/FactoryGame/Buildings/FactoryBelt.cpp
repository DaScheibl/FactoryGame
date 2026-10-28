// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryBelt.h"
#include "FactoryBeltComponent.h"
#include "FactoryProductionComponent.h"
#include "FactoryItemDatabase.h"
#include "FactoryLocalization.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// Graybox-Maße in cm: Bandbreite 0,8 m (Docs/Standards/03-raster.md), Item-Würfel 0,3 m
	constexpr double BeltWidth = 80.0;
	constexpr double BeltThickness = 10.0;
	constexpr double ItemCubeSize = 30.0;

	// Abstand von der Maschinenmitte bis zur Außenkante ihrer Bounding-Box in Richtung Dir (nur XY)
	double GetDistanceToEdge(const AActor* Machine, const FVector& Dir)
	{
		FVector Origin;
		FVector Extent;
		Machine->GetActorBounds(true, Origin, Extent);

		const double ToEdgeX = FMath::Abs(Dir.X) > UE_KINDA_SMALL_NUMBER ? Extent.X / FMath::Abs(Dir.X) : UE_BIG_NUMBER;
		const double ToEdgeY = FMath::Abs(Dir.Y) > UE_KINDA_SMALL_NUMBER ? Extent.Y / FMath::Abs(Dir.Y) : UE_BIG_NUMBER;
		return FMath::Min(ToEdgeX, ToEdgeY);
	}
}

AFactoryBelt::AFactoryBelt()
{
	// der Actor-Tick ist nur für die Optik da (Item-Würfel bewegen), die Simulation läuft im Simulations-Takt
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	CubeMesh = CubeFinder.Object;
	ColorMaterial = MaterialFinder.Object;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(Root);
	BodyMesh->SetStaticMesh(CubeMesh);
	BodyMesh->SetMaterial(0, ColorMaterial);

	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Root);
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetVerticalAlignment(EVRTA_TextBottom);
	Label->SetWorldSize(20.0f);
	Label->SetTextRenderColor(FColor::White);

	Belt = CreateDefaultSubobject<UFactoryBeltComponent>(TEXT("Belt"));
}

void AFactoryBelt::BeginPlay()
{
	Super::BeginPlay();

	// Band-Körper dunkelgrau, damit die farbigen Item-Würfel darauf gut zu sehen sind
	if (UMaterialInstanceDynamic* Material = BodyMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.04f, 0.04f, 0.045f));
	}

	TArray<FName> Ignored;
	SetEndpoints(GetActorLocation(), GetActorLocation() + GetActorForwardVector() * Belt->GetLength() * 0.1, Ignored);
}

void AFactoryBelt::SetEndpoints(const FVector& Start, const FVector& End, TArray<FName>& OutRemoved)
{
	const FVector Delta = End - Start;
	const double LengthCm = Delta.Size();

	SetActorLocationAndRotation(Start, Delta.Rotation());
	Belt->SetLength(FMath::RoundToInt32(LengthCm * 10.0), OutRemoved);

	// der Engine-Würfel ist 1 m groß und mittig; so strecken, dass die Oberkante auf SurfaceHeight liegt
	const double ShownLength = Belt->GetLength() * 0.1;
	BodyMesh->SetRelativeScale3D(FVector(ShownLength, BeltWidth, BeltThickness) / 100.0);
	BodyMesh->SetRelativeLocation(FVector(ShownLength * 0.5, 0.0, SurfaceHeight - BeltThickness * 0.5));
	Label->SetRelativeLocation(FVector(ShownLength * 0.5, 0.0, SurfaceHeight + 60.0));
}

AFactoryBelt* AFactoryBelt::SpawnBetween(UWorld* World, AActor* SourceMachine, AActor* TargetMachine, int32 Tier)
{
	if (!World || !SourceMachine || !TargetMachine || SourceMachine == TargetMachine)
	{
		return nullptr;
	}

	// gerade Linie von der Kante der Quelle zur Kante des Ziels (Ports bündig mit dem Footprint)
	const FVector SourceLocation = SourceMachine->GetActorLocation();
	const FVector TargetLocation = TargetMachine->GetActorLocation();
	const FVector Dir = (TargetLocation - SourceLocation).GetSafeNormal2D();

	if (Dir.IsNearlyZero())
	{
		return nullptr;
	}

	const FVector Start = SourceLocation + Dir * GetDistanceToEdge(SourceMachine, Dir);
	const FVector End = TargetLocation - Dir * GetDistanceToEdge(TargetMachine, Dir);

	// die Maschinen müssen mindestens einen Item-Abstand auseinander stehen
	if (FVector::DotProduct(End - Start, Dir) * 10.0 < UFactoryBeltComponent::ItemSpacing)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFactoryBelt* NewBelt = World->SpawnActor<AFactoryBelt>(Start, (End - Start).Rotation(), SpawnParams);

	if (!NewBelt)
	{
		return nullptr;
	}

	TArray<FName> Ignored;
	NewBelt->SetEndpoints(Start, End, Ignored);
	NewBelt->Belt->SetTier(Tier);
	NewBelt->Belt->SetSource(SourceMachine->FindComponentByClass<UFactoryProductionComponent>());
	NewBelt->Belt->SetTarget(TargetMachine->FindComponentByClass<UFactoryProductionComponent>());
	return NewBelt;
}

void AFactoryBelt::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateItemMeshes();
	UpdateLabel();

	// Text zur Kamera drehen (nur um die Hochachse)
	if (const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		const FVector ToCamera = Camera->GetCameraLocation() - Label->GetComponentLocation();
		Label->SetWorldRotation(FRotator(0.0, ToCamera.Rotation().Yaw, 0.0));
	}
}

void AFactoryBelt::UpdateItemMeshes()
{
	Belt->GetDisplayPositions(PositionsScratch);
	const TArray<FFactoryBeltItem>& Items = Belt->GetItems();

	for (TPair<FName, TArray<FTransform>>& Pair : TransformsScratch)
	{
		Pair.Value.Reset();
	}

	const FVector CubeScale(ItemCubeSize / 100.0);

	for (int32 Index = 0; Index < Items.Num() && Index < PositionsScratch.Num(); ++Index)
	{
		const FVector Location(PositionsScratch[Index], 0.0, SurfaceHeight + ItemCubeSize * 0.5);
		TransformsScratch.FindOrAdd(Items[Index].ItemId).Emplace(FQuat::Identity, Location, CubeScale);
	}

	for (const TPair<FName, TArray<FTransform>>& Pair : TransformsScratch)
	{
		// Sorten, die nie auf dem Band lagen, brauchen keine Komponente
		if (Pair.Value.Num() == 0 && !ItemMeshes.Contains(Pair.Key))
		{
			continue;
		}

		UInstancedStaticMeshComponent* Mesh = GetItemMesh(Pair.Key);

		// ändert sich die Anzahl, neu aufbauen; sonst nur die Positionen verschieben (günstiger)
		if (Mesh->GetInstanceCount() != Pair.Value.Num())
		{
			Mesh->ClearInstances();
			Mesh->AddInstances(Pair.Value, false);
		}
		else if (Pair.Value.Num() > 0)
		{
			Mesh->BatchUpdateInstancesTransforms(0, Pair.Value, false, true);
		}
	}
}

UInstancedStaticMeshComponent* AFactoryBelt::GetItemMesh(FName ItemId)
{
	if (const TObjectPtr<UInstancedStaticMeshComponent>* Existing = ItemMeshes.Find(ItemId))
	{
		return *Existing;
	}

	UInstancedStaticMeshComponent* Mesh = NewObject<UInstancedStaticMeshComponent>(this);
	Mesh->SetStaticMesh(CubeMesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(false);
	Mesh->SetupAttachment(Root);
	Mesh->RegisterComponent();
	AddInstanceComponent(Mesh);

	// Graybox: Würfel in der Platzhalterfarbe des Items, später das BeltMesh aus DT_Items
	const UFactoryItemDatabase* Database = UFactoryItemDatabase::Get(this);
	const FFactoryItemDefinition* Item = Database ? Database->FindItem(ItemId) : nullptr;

	if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(ColorMaterial, Mesh))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Item ? FLinearColor(Item->PlaceholderColor) : FLinearColor::White);
		Mesh->SetMaterial(0, Material);
	}

	ItemMeshes.Add(ItemId, Mesh);
	return Mesh;
}

void AFactoryBelt::UpdateLabel()
{
	FString Text = FactoryLoc::Format(TEXT("Belt_Label"), { Belt->GetTier(), UFactoryBeltComponent::GetItemsPerMinute(Belt->GetTier()), Belt->GetItems().Num() }).ToString();

	if (Belt->IsBlocked())
	{
		Text += TEXT("\n") + FactoryLoc::Get(TEXT("Belt_Blocked")).ToString();
	}

	// nur neu setzen, wenn sich etwas geändert hat; SetText baut den Text-Mesh jedes Mal neu
	if (Text != LabelText)
	{
		LabelText = Text;
		Label->SetText(FText::FromString(Text));
	}
}
