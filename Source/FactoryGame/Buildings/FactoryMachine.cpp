// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryMachine.h"
#include "FactoryItemDatabase.h"
#include "FactoryRecipeDatabase.h"
#include "FactoryLocalization.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// Graybox-Farben pro Zustand (nicht final, kommt später über Status-Leuchten MESH_StatusLight)
	FLinearColor GetStateColor(EFactoryMachineState State)
	{
		switch (State)
		{
		case EFactoryMachineState::WaitingForInput:	return FLinearColor(0.8f, 0.6f, 0.05f);
		case EFactoryMachineState::Working:			return FLinearColor(0.1f, 0.6f, 0.15f);
		case EFactoryMachineState::OutputFull:		return FLinearColor(0.75f, 0.1f, 0.08f);
		default:									return FLinearColor(0.35f, 0.35f, 0.35f);
		}
	}

	// "12 Eisenerz, 3 Kohle"
	FString DescribeBuffers(const TArray<FFactoryItemStack>& Buffers, const UFactoryItemDatabase* Items)
	{
		TArray<FString> Parts;

		for (const FFactoryItemStack& Buffer : Buffers)
		{
			Parts.Add(FString::Printf(TEXT("%d %s"), Buffer.Count, *(Items ? Items->GetDisplayName(Buffer.ItemId) : FText::FromName(Buffer.ItemId)).ToString()));
		}

		return FString::Join(Parts, TEXT(", "));
	}
}

AFactoryMachine::AFactoryMachine()
{
	// der Actor-Tick ist nur für den Text da, die Produktion läuft im Simulations-Takt
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.2f;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Root);
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetVerticalAlignment(EVRTA_TextBottom);
	Label->SetWorldSize(30.0f);
	Label->SetTextRenderColor(FColor::White);

	Production = CreateDefaultSubobject<UFactoryProductionComponent>(TEXT("Production"));

	SetGrayboxSize(GrayboxSize);
}

void AFactoryMachine::SetGrayboxSize(const FVector& SizeInMeters)
{
	GrayboxSize = SizeInMeters;

	// der Engine-Würfel ist 1 m groß und mittig; nach oben schieben, damit der Actor auf dem Boden steht
	Mesh->SetRelativeScale3D(GrayboxSize);
	Mesh->SetRelativeLocation(FVector(0.0, 0.0, GrayboxSize.Z * 50.0));
	Label->SetRelativeLocation(FVector(0.0, 0.0, GrayboxSize.Z * 100.0 + 30.0));
}

void AFactoryMachine::BeginPlay()
{
	Super::BeginPlay();

	SetGrayboxSize(GrayboxSize);

	// BasicShapeMaterial hat einen Farbparameter "Color"
	Material = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	Production->OnStateChanged.AddUObject(this, &AFactoryMachine::HandleStateChanged);
	HandleStateChanged(Production->GetState());
}

void AFactoryMachine::HandleStateChanged(EFactoryMachineState NewState)
{
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), GetStateColor(NewState));
	}

	UpdateLabel();
}

void AFactoryMachine::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateLabel();

	// Text zur Kamera drehen (nur um die Hochachse)
	if (const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		const FVector ToCamera = Camera->GetCameraLocation() - Label->GetComponentLocation();
		Label->SetWorldRotation(FRotator(0.0, ToCamera.Rotation().Yaw, 0.0));
	}
}

void AFactoryMachine::UpdateLabel()
{
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);
	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);

	FText StateText;

	switch (Production->GetState())
	{
	case EFactoryMachineState::WaitingForInput:	StateText = FactoryLoc::Get(TEXT("Machine_WaitingForInput")); break;
	case EFactoryMachineState::Working:			StateText = FactoryLoc::Format(TEXT("Machine_Working"), { FMath::FloorToInt32(Production->GetProgress() * 100.0f) }); break;
	case EFactoryMachineState::OutputFull:		StateText = FactoryLoc::Get(TEXT("Machine_OutputFull")); break;
	default:									StateText = FactoryLoc::Get(TEXT("Machine_NoRecipe")); break;
	}

	// Gebäude haben noch keine eigene Tabelle, deshalb steht hier vorerst die ID
	FString Text = Production->GetBuildingId().ToString();

	if (!Production->GetRecipeId().IsNone())
	{
		Text += TEXT("\n") + (Recipes ? Recipes->GetDisplayName(Production->GetRecipeId()) : FText::FromName(Production->GetRecipeId())).ToString();
		Text += TEXT("\n") + StateText.ToString();
		Text += TEXT("\n") + DescribeBuffers(Production->GetInputs(), Items) + TEXT("  >  ") + DescribeBuffers(Production->GetOutputs(), Items);
	}
	else
	{
		Text += TEXT("\n") + StateText.ToString();
	}

	Label->SetText(FText::FromString(Text));
}
