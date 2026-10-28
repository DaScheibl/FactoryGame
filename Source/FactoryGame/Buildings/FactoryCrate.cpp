// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryCrate.h"
#include "FactoryGamePlayerController.h"
#include "FactoryLocalization.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AFactoryCrate::AFactoryCrate()
{
	// der Actor-Tick dreht nur den Text zur Kamera
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	Mesh->SetStaticMesh(CubeFinder.Object);
	Mesh->SetMaterial(0, MaterialFinder.Object);

	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Mesh);
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetVerticalAlignment(EVRTA_TextBottom);
	Label->SetWorldSize(20.0f);
	Label->SetRelativeLocation(FVector(0.0, 0.0, 70.0));
	Label->SetTextRenderColor(FColor::White);
}

AFactoryCrate* AFactoryCrate::SpawnCrate(UWorld* World, const FVector& Location, const TArray<FFactoryItemStack>& Stacks)
{
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// der Würfel ist mittig, also 0,5 m hoch setzen, damit er auf dem Boden steht
	AFactoryCrate* Crate = World->SpawnActor<AFactoryCrate>(Location + FVector(0.0, 0.0, 50.0), FRotator::ZeroRotator, SpawnParams);

	if (Crate)
	{
		if (UMaterialInstanceDynamic* Material = Crate->Mesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.35f, 0.18f, 0.06f));
		}

		Crate->AddItems(Stacks);
	}

	return Crate;
}

void AFactoryCrate::AddItems(const TArray<FFactoryItemStack>& Stacks)
{
	for (const FFactoryItemStack& Stack : Stacks)
	{
		if (Stack.IsEmpty())
		{
			continue;
		}

		if (FFactoryItemStack* Existing = Items.FindByPredicate([&Stack](const FFactoryItemStack& Item) { return Item.ItemId == Stack.ItemId; }))
		{
			Existing->Count += Stack.Count;
		}
		else
		{
			Items.Add(Stack);
		}
	}

	UpdateLabel();
}

void AFactoryCrate::TakeItems(AFactoryGamePlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	// GiveItems legt nur so viel ab, wie passt, und meldet "Inventar voll"; der Rest bleibt in der Kiste
	for (FFactoryItemStack& Item : Items)
	{
		Item.Count -= PlayerController->GiveItems(Item.ItemId, Item.Count);
	}

	Items.RemoveAll([](const FFactoryItemStack& Item) { return Item.IsEmpty(); });

	if (Items.IsEmpty())
	{
		Destroy();
		return;
	}

	UpdateLabel();
}

void AFactoryCrate::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Text zur Kamera drehen (nur um die Hochachse)
	if (const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		const FVector ToCamera = Camera->GetCameraLocation() - Label->GetComponentLocation();
		Label->SetWorldRotation(FRotator(0.0, ToCamera.Rotation().Yaw, 0.0));
	}
}

void AFactoryCrate::UpdateLabel()
{
	int32 Total = 0;

	for (const FFactoryItemStack& Item : Items)
	{
		Total += Item.Count;
	}

	Label->SetText(FactoryLoc::Format(TEXT("Crate_Label"), { Total }));
}
