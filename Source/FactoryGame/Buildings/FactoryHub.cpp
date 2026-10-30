// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryHub.h"
#include "FactoryBuildingDatabase.h"
#include "FactoryRecipeDatabase.h"
#include "FactoryInventoryComponent.h"
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

const FName AFactoryHub::HubId(TEXT("BLD_Hub"));

namespace
{
	/** Where a new HUB stands, relative to the world origin (the start point of the flat test world), in cm */
	const FVector HubStartOffset(0.0, 1200.0, 0.0);

	/** Size of the graybox if the building table has no BLD_Hub row */
	const FVector HubDefaultSize(800.0, 800.0, 600.0);
}

AFactoryHub::AFactoryHub()
{
	// der Actor-Tick dreht nur den Text zur Kamera
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetStaticMesh(CubeFinder.Object);
	Mesh->SetMaterial(0, MaterialFinder.Object);

	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Root);
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetVerticalAlignment(EVRTA_TextBottom);
	Label->SetWorldSize(60.0f);
	Label->SetTextRenderColor(FColor::White);
	Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Label->SetText(FText::FromString(TEXT("HUB")));

	Storage = CreateDefaultSubobject<UFactoryInventoryComponent>(TEXT("Storage"));
}

AFactoryHub* AFactoryHub::Get(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AFactoryHub> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

AFactoryHub* AFactoryHub::SpawnHub(UWorld* World, const FTransform& Transform)
{
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFactoryHub* Hub = World->SpawnActor<AFactoryHub>(Transform.GetLocation(), Transform.Rotator(), SpawnParams);

	if (!Hub)
	{
		return nullptr;
	}

	Hub->Storage->SetNumSlots(StorageSlots);

	// Größe aus der Gebäude-Tabelle, Würfel mit Ursprung unten mittig (wie bei Maschinen)
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(World);
	const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(HubId) : nullptr;
	const FVector Size = Building ? Building->GetSize() : HubDefaultSize;

	Hub->Mesh->SetRelativeScale3D(Size / 100.0);
	Hub->Mesh->SetRelativeLocation(FVector(0.0, 0.0, Size.Z * 0.5));
	Hub->Label->SetRelativeLocation(FVector(0.0, 0.0, Size.Z + 40.0));

	if (UMaterialInstanceDynamic* Material = Hub->Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.55f, 0.32f, 0.08f));
	}

	return Hub;
}

AFactoryHub* AFactoryHub::EnsureHub(UWorld* World)
{
	if (AFactoryHub* Existing = Get(World))
	{
		return Existing;
	}

	// neben dem Startpunkt auf den Boden setzen
	FVector Location = HubStartOffset;
	FHitResult Hit;

	if (World && World->LineTraceSingleByChannel(Hit, Location + FVector(0.0, 0.0, 5000.0), Location - FVector(0.0, 0.0, 5000.0), ECC_Visibility))
	{
		Location.Z = Hit.ImpactPoint.Z;
	}

	UE_LOG(LogFactoryGame, Log, TEXT("World has no HUB yet, placing one at %s."), *Location.ToString());
	return SpawnHub(World, FTransform(Location));
}

int32 AFactoryHub::GetUnlockedTier(const UObject* WorldContextObject)
{
	const AFactoryHub* Hub = Get(WorldContextObject);
	return Hub ? Hub->UnlockedTier : 0;
}

bool AFactoryHub::IsBuildingUnlocked(const UObject* WorldContextObject, const FFactoryBuildingDefinition& Building)
{
	return Building.Tier <= GetUnlockedTier(WorldContextObject);
}

bool AFactoryHub::IsRecipeUnlocked(const UObject* WorldContextObject, const FFactoryRecipeDefinition& Recipe, bool bAtWorkbench)
{
	const int32 Tier = GetUnlockedTier(WorldContextObject);

	if (Recipe.Tier > Tier)
	{
		return false;
	}

	if (bAtWorkbench)
	{
		return true;
	}

	// in einer Maschine zählt auch deren Tier (die Maschine selbst muss baubar sein)
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(WorldContextObject);
	const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(Recipe.Building) : nullptr;
	return !Building || Building->Tier <= Tier;
}

const FFactoryMilestone* AFactoryHub::GetMilestone(int32 Tier)
{
	// Tier N wird mit Forschungspaketen freigeschaltet (Docs/Standards/16-hub.md). Mengen = Entwurf bis zum Balancing
	static const TArray<FFactoryMilestone> Milestones = {
		{ 1, { FFactoryItemStack(TEXT("ITM_SciencePack_01"), 20) } },
		{ 2, { FFactoryItemStack(TEXT("ITM_SciencePack_01"), 50), FFactoryItemStack(TEXT("ITM_SciencePack_02"), 20) } },
	};

	return Milestones.FindByPredicate([Tier](const FFactoryMilestone& Milestone) { return Milestone.Tier == Tier; });
}

void AFactoryHub::SetTier(int32 NewTier)
{
	UnlockedTier = FMath::Clamp(NewTier, 0, 8);
	Delivered.Reset();
}

int32 AFactoryHub::GetDelivered(FName ItemId) const
{
	const FFactoryItemStack* Stack = Delivered.FindByPredicate([ItemId](const FFactoryItemStack& Item) { return Item.ItemId == ItemId; });
	return Stack ? Stack->Count : 0;
}

bool AFactoryHub::Deliver(UFactoryInventoryComponent* Inventory)
{
	const FFactoryMilestone* Milestone = GetMilestone(UnlockedTier + 1);

	if (!Milestone || !Inventory)
	{
		return false;
	}

	// Abgeben geht auch in Teilen, gemerkt wird, was schon da ist
	bool bTookAny = false;

	for (const FFactoryItemStack& Cost : Milestone->Costs)
	{
		const int32 Missing = Cost.Count - GetDelivered(Cost.ItemId);
		const int32 Taken = Missing > 0 ? Inventory->RemoveItem(Cost.ItemId, Missing) : 0;

		if (Taken <= 0)
		{
			continue;
		}

		bTookAny = true;

		if (FFactoryItemStack* Stack = Delivered.FindByPredicate([&Cost](const FFactoryItemStack& Item) { return Item.ItemId == Cost.ItemId; }))
		{
			Stack->Count += Taken;
		}
		else
		{
			Delivered.Emplace(Cost.ItemId, Taken);
		}
	}

	return bTookAny;
}

bool AFactoryHub::CanUnlockNext() const
{
	const FFactoryMilestone* Milestone = GetMilestone(UnlockedTier + 1);

	return Milestone && !Milestone->Costs.ContainsByPredicate([this](const FFactoryItemStack& Cost) { return GetDelivered(Cost.ItemId) < Cost.Count; });
}

bool AFactoryHub::UnlockNext()
{
	if (!CanUnlockNext())
	{
		return false;
	}

	SetTier(UnlockedTier + 1);
	UE_LOG(LogFactoryGame, Log, TEXT("Unlocked tier %d."), UnlockedTier);
	return true;
}

void AFactoryHub::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Text zur Kamera drehen (nur um die Hochachse)
	if (const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		const FVector ToCamera = Camera->GetCameraLocation() - Label->GetComponentLocation();
		Label->SetWorldRotation(FRotator(0.0, ToCamera.Rotation().Yaw, 0.0));
	}
}
