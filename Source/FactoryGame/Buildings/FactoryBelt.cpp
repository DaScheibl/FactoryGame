// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryBelt.h"
#include "FactoryBeltComponent.h"
#include "FactoryBeltPole.h"
#include "FactoryMachine.h"
#include "FactoryProductionComponent.h"
#include "FactoryItemDatabase.h"
#include "FactoryBuildingDatabase.h"
#include "FactoryLocalization.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/CollisionProfile.h"
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

	// Länge eines Körper-Stücks; Spline-Meshes verformen sich nur gut, wenn sie nicht zu lang sind
	constexpr double BodyPieceLength = 100.0;

	const FLinearColor BodyColor(0.04f, 0.04f, 0.045f);
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

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(Root);

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
	BodyMaterial = UMaterialInstanceDynamic::Create(ColorMaterial, this);
	SetDemolishHighlight(false);
}

AFactoryBelt* AFactoryBelt::SpawnBelt(UWorld* World, const FFactoryBeltPath& Path, int32 Tier)
{
	if (!World || Path.Points.Num() < 2)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFactoryBelt* NewBelt = World->SpawnActor<AFactoryBelt>(Path.Points[0].Location, FRotator::ZeroRotator, SpawnParams);

	if (NewBelt)
	{
		TArray<FName> Ignored;
		NewBelt->SetPath(Path, Ignored);
		NewBelt->Belt->SetTier(Tier);
	}

	return NewBelt;
}

void AFactoryBelt::SetPath(const FFactoryBeltPath& NewPath, TArray<FName>& OutRemoved)
{
	Path = NewPath;

	// der Actor steht am Bandanfang, der Spline ist relativ dazu
	SetActorLocationAndRotation(Path.Points.Num() > 0 ? Path.Points[0].Location : GetActorLocation(), FRotator::ZeroRotator);
	ApplyPathToSpline(Spline, Path, GetActorLocation());

	Belt->SetLength(FMath::RoundToInt32(Spline->GetSplineLength() * 10.0), OutRemoved);

	if (!BodyMaterial)
	{
		BodyMaterial = UMaterialInstanceDynamic::Create(ColorMaterial, this);
		SetDemolishHighlight(false);
	}

	BuildBodyMeshes(this, Spline, BodyMeshes, CubeMesh, BodyMaterial, &CollisionBoxes);

	const float Middle = Spline->GetSplineLength() * 0.5f;
	Label->SetRelativeLocation(Spline->GetLocationAtDistanceAlongSpline(Middle, ESplineCoordinateSpace::Local) + FVector(0.0, 0.0, 60.0));
}

void AFactoryBelt::ApplyPathToSpline(USplineComponent* Spline, const FFactoryBeltPath& Path, const FVector& Origin)
{
	Spline->ClearSplinePoints(false);

	for (int32 Index = 0; Index < Path.Points.Num(); ++Index)
	{
		const FFactoryBeltPathPoint& Point = Path.Points[Index];
		Spline->AddSplinePoint(Point.Location - Origin, ESplineCoordinateSpace::Local, false);
		Spline->SetSplinePointType(Index, ESplinePointType::CurveCustomTangent, false);
		Spline->SetTangentsAtSplinePoint(Index, Point.ArriveTangent, Point.LeaveTangent, ESplineCoordinateSpace::Local, false);
	}

	Spline->UpdateSpline();
}

void AFactoryBelt::BuildBodyMeshes(AActor* Owner, USplineComponent* Spline, TArray<TObjectPtr<USplineMeshComponent>>& Meshes,
	UStaticMesh* Mesh, UMaterialInterface* Material, TArray<TObjectPtr<UBoxComponent>>* Boxes)
{
	const double Length = Spline->GetSplineLength();
	const int32 NumPieces = FMath::Max(1, FMath::CeilToInt32(Length / BodyPieceLength));
	const double PieceLength = Length / NumPieces;

	// vorhandene Stücke wiederverwenden, überzählige entfernen
	while (Meshes.Num() > NumPieces)
	{
		Meshes.Pop()->DestroyComponent();
	}

	while (Boxes && Boxes->Num() > NumPieces)
	{
		Boxes->Pop()->DestroyComponent();
	}

	while (Boxes && Boxes->Num() < NumPieces)
	{
		UBoxComponent* Box = NewObject<UBoxComponent>(Owner);
		Box->SetupAttachment(Spline);
		Box->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
		Box->RegisterComponent();
		Owner->AddInstanceComponent(Box);
		Boxes->Add(Box);
	}

	while (Meshes.Num() < NumPieces)
	{
		USplineMeshComponent* Piece = NewObject<USplineMeshComponent>(Owner);
		Piece->SetMobility(EComponentMobility::Movable);
		Piece->SetStaticMesh(Mesh);
		Piece->SetupAttachment(Spline);
		Piece->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Piece->SetCastShadow(Boxes != nullptr);
		Piece->RegisterComponent();
		Owner->AddInstanceComponent(Piece);
		Meshes.Add(Piece);
	}

	// der Würfel wird zum flachen Streifen: 0,8 m breit, 0,1 m dick, Oberkante auf der Kurve
	const FVector Down(0.0, 0.0, BeltThickness * 0.5);

	for (int32 Index = 0; Index < NumPieces; ++Index)
	{
		const float From = PieceLength * Index;
		const float To = PieceLength * (Index + 1);
		const FVector StartTangent = Spline->GetTangentAtDistanceAlongSpline(From, ESplineCoordinateSpace::Local).GetSafeNormal() * PieceLength;
		const FVector EndTangent = Spline->GetTangentAtDistanceAlongSpline(To, ESplineCoordinateSpace::Local).GetSafeNormal() * PieceLength;

		USplineMeshComponent* Piece = Meshes[Index];
		Piece->SetMaterial(0, Material);
		Piece->SetStartScale(FVector2D(BeltWidth / 100.0, BeltThickness / 100.0), false);
		Piece->SetEndScale(FVector2D(BeltWidth / 100.0, BeltThickness / 100.0), false);
		Piece->SetStartAndEnd(Spline->GetLocationAtDistanceAlongSpline(From, ESplineCoordinateSpace::Local) - Down, StartTangent,
			Spline->GetLocationAtDistanceAlongSpline(To, ESplineCoordinateSpace::Local) - Down, EndTangent, true);

		// gerade Box über das Stück; bei 1 m Länge weicht sie in Kurven nur wenige Zentimeter ab
		if (Boxes)
		{
			const FTransform MiddleTransform = Spline->GetTransformAtDistanceAlongSpline((From + To) * 0.5f, ESplineCoordinateSpace::Local);
			UBoxComponent* Box = (*Boxes)[Index];
			Box->SetBoxExtent(FVector(PieceLength * 0.5, BeltWidth * 0.5, BeltThickness * 0.5), false);
			Box->SetRelativeLocationAndRotation(MiddleTransform.GetLocation() - Down, MiddleTransform.GetRotation());
		}
	}
}

void AFactoryBelt::ConnectStartToMachine(AFactoryMachine* Machine, int32 PortIndex)
{
	if (!Machine)
	{
		return;
	}

	StartMachine = Machine;
	StartPort = PortIndex;
	Machine->SetPortBelt(PortIndex, this);

	// Ausgang N der Maschine hat den Port-Index NumInputs + N
	Belt->SetSource(Machine->GetProduction(), PortIndex - Machine->GetNumInputPorts());
}

void AFactoryBelt::ConnectEndToMachine(AFactoryMachine* Machine, int32 PortIndex)
{
	if (!Machine)
	{
		return;
	}

	EndMachine = Machine;
	EndPort = PortIndex;
	Machine->SetPortBelt(PortIndex, this);
	Belt->SetTarget(Machine->GetProduction());
}

void AFactoryBelt::ConnectStartToPole(AFactoryBeltPole* Pole)
{
	if (!Pole)
	{
		return;
	}

	StartPole = Pole;
	Pole->SetOutputBelt(this);

	// das Band, das an der Stütze ankommt, liefert ab jetzt in dieses
	if (AFactoryBelt* Incoming = Pole->GetInputBelt())
	{
		Incoming->Belt->SetTarget(Belt);
	}
}

void AFactoryBelt::ConnectEndToPole(AFactoryBeltPole* Pole)
{
	if (!Pole)
	{
		return;
	}

	EndPole = Pole;
	Pole->SetInputBelt(this);

	if (AFactoryBelt* Outgoing = Pole->GetOutputBelt())
	{
		Belt->SetTarget(Outgoing->Belt);
	}
}

AFactoryBelt* AFactoryBelt::SpawnBetween(UWorld* World, AFactoryMachine* SourceMachine, AFactoryMachine* TargetMachine, int32 Tier)
{
	if (!World || !SourceMachine || !TargetMachine || SourceMachine == TargetMachine)
	{
		return nullptr;
	}

	const TArray<FFactoryPort> SourcePorts = SourceMachine->GetWorldPorts();
	const TArray<FFactoryPort> TargetPorts = TargetMachine->GetWorldPorts();

	// erster freier Ausgang der Quelle ...
	const int32 OutputIndex = SourcePorts.IndexOfByPredicate([SourceMachine, &SourcePorts](const FFactoryPort& Port)
	{
		return !Port.bInput && !SourceMachine->GetPortBelt(&Port - SourcePorts.GetData());
	});

	if (OutputIndex == INDEX_NONE)
	{
		return nullptr;
	}

	// ... zum nächstgelegenen freien Eingang des Ziels
	const int32 InputIndex = TargetMachine->FindFreePort(true, SourcePorts[OutputIndex].Location, UE_BIG_NUMBER);

	if (InputIndex == INDEX_NONE)
	{
		return nullptr;
	}

	// der Reihe nach alle Modi probieren, der erste passende gewinnt
	FFactoryBeltPath NewPath;

	for (const EFactoryBeltMode Mode : { EFactoryBeltMode::Standard, EFactoryBeltMode::Curve, EFactoryBeltMode::Straight })
	{
		if (FactoryBeltPath::Build(Mode, SourcePorts[OutputIndex].Location, SourcePorts[OutputIndex].Direction,
			TargetPorts[InputIndex].Location, TargetPorts[InputIndex].Direction, NewPath) == EFactoryBeltPathProblem::None)
		{
			AFactoryBelt* NewBelt = SpawnBelt(World, NewPath, Tier);

			if (NewBelt)
			{
				NewBelt->ConnectStartToMachine(SourceMachine, OutputIndex);
				NewBelt->ConnectEndToMachine(TargetMachine, InputIndex);
			}

			return NewBelt;
		}
	}

	return nullptr;
}

FName AFactoryBelt::GetBuildingIdForTier(int32 Tier)
{
	return FName(FString::Printf(TEXT("LOG_Belt_Mk%d"), Tier));
}

TArray<FFactoryItemStack> AFactoryBelt::GetBuildCostsForLength(const UObject* WorldContext, double LengthCm, int32 Tier)
{
	// die Tabelle enthält die Kosten pro angefangene 2 m; Stufen ohne eigene Zeile kosten wie Mk1
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(WorldContext);
	const FFactoryBuildingDefinition* Row = Buildings ? Buildings->FindBuilding(GetBuildingIdForTier(Tier)) : nullptr;
	Row = Row ? Row : (Buildings ? Buildings->FindBuilding(GetBuildingIdForTier(1)) : nullptr);

	TArray<FFactoryItemStack> Costs = Row ? Row->GetCosts() : TArray<FFactoryItemStack>();
	const int32 Pieces = FMath::Max(1, FMath::CeilToInt32(LengthCm / 200.0 - 0.001));

	for (FFactoryItemStack& Cost : Costs)
	{
		Cost.Count *= Pieces;
	}

	return Costs;
}

TArray<FFactoryItemStack> AFactoryBelt::GetBuildCosts() const
{
	return GetBuildCostsForLength(this, Spline->GetSplineLength(), Belt->GetTier());
}

void AFactoryBelt::SetDemolishHighlight(bool bHighlighted)
{
	if (BodyMaterial)
	{
		BodyMaterial->SetVectorParameterValue(TEXT("Color"), bHighlighted ? FLinearColor(1.0f, 0.02f, 0.02f) : BodyColor);
	}
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
	const float SplineLength = Spline->GetSplineLength();

	// Simulations-Länge (ganze mm) und Spline-Länge können um Bruchteile abweichen, deshalb begrenzen
	for (int32 Index = 0; Index < Items.Num() && Index < PositionsScratch.Num(); ++Index)
	{
		const float Distance = FMath::Clamp(PositionsScratch[Index], 0.0f, SplineLength);
		const FTransform OnSpline = Spline->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
		const FVector Location = OnSpline.GetLocation() + OnSpline.GetRotation().GetUpVector() * ItemCubeSize * 0.5;
		TransformsScratch.FindOrAdd(Items[Index].ItemId).Emplace(OnSpline.GetRotation(), Location, CubeScale);
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
	Mesh->SetupAttachment(Spline);
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
