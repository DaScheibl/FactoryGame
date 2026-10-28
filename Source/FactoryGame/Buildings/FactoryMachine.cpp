// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryMachine.h"
#include "FactoryItemDatabase.h"
#include "FactoryRecipeDatabase.h"
#include "FactoryBuildingDatabase.h"
#include "FactoryBelt.h"
#include "FactoryLocalization.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
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
	// der Actor-Tick ist nur für den Text da (jedes Bild, damit die Prozente flüssig zählen),
	// die Produktion läuft im Simulations-Takt
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	// der Würfel hat standardmäßig das WorldGridMaterial ohne Farbparameter, deshalb BasicShapeMaterial setzen
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ColorMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	if (ColorMaterial.Succeeded())
	{
		Mesh->SetMaterial(0, ColorMaterial.Object);
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

void AFactoryMachine::InitBuilding(FName BuildingId)
{
	Production->SetBuildingId(BuildingId);

	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(BuildingId) : nullptr;

	if (!Building)
	{
		return;
	}

	SetGrayboxSize(FVector(Building->SizeX, Building->SizeY, Building->SizeZ));
	Ports = Building->GetPorts();
	PortBelts.SetNum(Ports.Num());
	CreatePortMarkers();
}

void AFactoryMachine::CreatePortMarkers()
{
	for (UStaticMeshComponent* Marker : PortMarkers)
	{
		Marker->DestroyComponent();
	}

	PortMarkers.Reset();

	// Graybox: eine Platte in Bandbreite (0,8 m), die 0,3 m aus der Kante ragt, Oberkante auf Port-Höhe
	for (const FFactoryPort& Port : Ports)
	{
		UStaticMeshComponent* Marker = NewObject<UStaticMeshComponent>(this);
		Marker->SetStaticMesh(Mesh->GetStaticMesh());
		Marker->SetMaterial(0, Mesh->GetMaterial(0));
		Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Marker->SetupAttachment(Root);
		Marker->SetRelativeScale3D(FVector(0.3, 0.8, 0.1));
		Marker->SetRelativeLocation(Port.Location + Port.Direction * (Port.bInput ? -15.0 : 15.0) - FVector(0.0, 0.0, 5.0));
		Marker->RegisterComponent();
		AddInstanceComponent(Marker);

		// blau = Eingang, orange = Ausgang
		if (UMaterialInstanceDynamic* MarkerMaterial = Marker->CreateAndSetMaterialInstanceDynamic(0))
		{
			MarkerMaterial->SetVectorParameterValue(TEXT("Color"), Port.bInput ? FLinearColor(0.05f, 0.25f, 0.8f) : FLinearColor(0.9f, 0.4f, 0.05f));
		}

		PortMarkers.Add(Marker);
	}
}

TArray<FFactoryPort> AFactoryMachine::GetWorldPorts() const
{
	TArray<FFactoryPort> WorldPorts = Ports;
	const FTransform& Transform = GetActorTransform();

	for (FFactoryPort& Port : WorldPorts)
	{
		Port.Location = Transform.TransformPosition(Port.Location);
		Port.Direction = Transform.TransformVectorNoScale(Port.Direction);
	}

	return WorldPorts;
}

int32 AFactoryMachine::GetNumInputPorts() const
{
	return Ports.FilterByPredicate([](const FFactoryPort& Port) { return Port.bInput; }).Num();
}

AFactoryBelt* AFactoryMachine::GetPortBelt(int32 PortIndex) const
{
	return PortBelts.IsValidIndex(PortIndex) ? PortBelts[PortIndex].Get() : nullptr;
}

void AFactoryMachine::SetPortBelt(int32 PortIndex, AFactoryBelt* Belt)
{
	if (PortBelts.IsValidIndex(PortIndex))
	{
		PortBelts[PortIndex] = Belt;
	}
}

int32 AFactoryMachine::FindFreePort(bool bInput, const FVector& WorldLocation, double MaxDistance) const
{
	const TArray<FFactoryPort> WorldPorts = GetWorldPorts();
	int32 Best = INDEX_NONE;
	double BestDistance = MaxDistance;

	for (int32 Index = 0; Index < WorldPorts.Num(); ++Index)
	{
		const double Distance = FVector::Dist(WorldPorts[Index].Location, WorldLocation);

		if (WorldPorts[Index].bInput == bInput && !GetPortBelt(Index) && Distance <= BestDistance)
		{
			Best = Index;
			BestDistance = Distance;
		}
	}

	return Best;
}

void AFactoryMachine::SetDemolishHighlight(bool bHighlighted)
{
	if (bDemolishHighlight != bHighlighted)
	{
		bDemolishHighlight = bHighlighted;
		UpdateColor();
	}
}

void AFactoryMachine::UpdateColor()
{
	if (Material)
	{
		// Abriss-Markierung hat Vorrang vor der Zustandsfarbe
		Material->SetVectorParameterValue(TEXT("Color"), bDemolishHighlight ? FLinearColor(1.0f, 0.02f, 0.02f) : GetStateColor(Production->GetState()));
	}
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
	UpdateColor();
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
	case EFactoryMachineState::Working:			StateText = FactoryLoc::Format(TEXT("Machine_Working"), { FMath::FloorToInt32(Production->GetDisplayProgress() * 100.0f) }); break;
	case EFactoryMachineState::OutputFull:		StateText = FactoryLoc::Get(TEXT("Machine_OutputFull")); break;
	default:									StateText = FactoryLoc::Get(TEXT("Machine_NoRecipe")); break;
	}

	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	FString Text = (Buildings ? Buildings->GetDisplayName(Production->GetBuildingId()) : FText::FromName(Production->GetBuildingId())).ToString();

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

	// nur neu setzen, wenn sich etwas geändert hat; SetText baut den Text-Mesh jedes Mal neu
	if (Text != LabelText)
	{
		LabelText = Text;
		Label->SetText(FText::FromString(Text));
	}
}
