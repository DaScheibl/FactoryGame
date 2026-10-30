// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryBuildComponent.h"
#include "FactoryBuildingDatabase.h"
#include "FactoryHologram.h"
#include "FactoryMachine.h"
#include "FactoryBelt.h"
#include "FactoryBeltPole.h"
#include "FactoryBeltComponent.h"
#include "FactoryCrate.h"
#include "FactoryGamePlayerController.h"
#include "FactoryGameCharacter.h"
#include "FactoryInventoryComponent.h"
#include "FactoryItemDatabase.h"
#include "FactoryHUDWidget.h"
#include "FactoryLocalization.h"
#include "FactoryGame.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"

const FName UFactoryBuildComponent::PoleId(TEXT("LOG_BeltPole"));

namespace
{
	// Würfel-Ecken leicht nach innen, damit die Boden-Messung nicht schon die Nachbarkachel trifft
	constexpr double GroundSampleInset = 50.0;

	// Gebäude-Actors, die sich nie überlappen dürfen
	bool IsFactoryActor(const AActor* Actor)
	{
		return Actor && (Actor->IsA<AFactoryMachine>() || Actor->IsA<AFactoryBelt>() || Actor->IsA<AFactoryBeltPole>() || Actor->IsA<AFactoryCrate>() || Actor->IsA<APawn>());
	}

	// gleiche Items zusammenfassen, damit die Meldungen "+10 Eisenplatte" statt zweimal "+5" zeigen
	void AddStacks(TArray<FFactoryItemStack>& Target, const TArray<FFactoryItemStack>& Stacks)
	{
		for (const FFactoryItemStack& Stack : Stacks)
		{
			if (Stack.IsEmpty())
			{
				continue;
			}

			if (FFactoryItemStack* Existing = Target.FindByPredicate([&Stack](const FFactoryItemStack& Item) { return Item.ItemId == Stack.ItemId; }))
			{
				Existing->Count += Stack.Count;
			}
			else
			{
				Target.Add(Stack);
			}
		}
	}

	FCollisionObjectQueryParams GetBlockingObjectTypes()
	{
		FCollisionObjectQueryParams ObjectTypes;
		ObjectTypes.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjectTypes.AddObjectTypesToQuery(ECC_Pawn);
		ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);
		return ObjectTypes;
	}

	FText GetPathProblemText(EFactoryBeltPathProblem Problem)
	{
		switch (Problem)
		{
		case EFactoryBeltPathProblem::TooShort:		return FactoryLoc::Get(TEXT("Build_Error_TooShort"));
		case EFactoryBeltPathProblem::TooLong:		return FactoryLoc::Get(TEXT("Build_Error_TooLong"));
		case EFactoryBeltPathProblem::TooSteep:		return FactoryLoc::Get(TEXT("Build_Error_TooSteep"));
		case EFactoryBeltPathProblem::TooTight:		return FactoryLoc::Get(TEXT("Build_Error_TooTight"));
		case EFactoryBeltPathProblem::Impossible:	return FactoryLoc::Get(TEXT("Build_Error_Impossible"));
		default:									return FText::GetEmpty();
		}
	}

	FText GetBeltModeText(EFactoryBeltMode Mode)
	{
		switch (Mode)
		{
		case EFactoryBeltMode::Straight:	return FactoryLoc::Get(TEXT("Belt_Mode_Straight"));
		case EFactoryBeltMode::Curve:		return FactoryLoc::Get(TEXT("Belt_Mode_Curve"));
		default:							return FactoryLoc::Get(TEXT("Belt_Mode_Standard"));
		}
	}
}

bool FFactoryBeltEnd::IsSameAs(const FFactoryBeltEnd& Other) const
{
	if (Machine.IsValid() && Machine == Other.Machine)
	{
		return true;
	}

	if (Pole.IsValid() && Pole == Other.Pole)
	{
		return true;
	}

	return bNewPole && Other.bNewPole && PoleFoot.Equals(Other.PoleFoot, 1.0);
}

UFactoryBuildComponent::UFactoryBuildComponent()
{
	// jedes Bild: Hologramm ans Fadenkreuz, Abriss-Ziel suchen, Hinweise aktualisieren
	PrimaryComponentTick.bCanEverTick = true;
}

void UFactoryBuildComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopBuilding();
	Super::EndPlay(EndPlayReason);
}

AFactoryGamePlayerController* UFactoryBuildComponent::GetPlayerController() const
{
	return Cast<AFactoryGamePlayerController>(GetOwner());
}

const FFactoryBuildingDefinition* UFactoryBuildComponent::GetBuilding() const
{
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	return Buildings ? Buildings->FindBuilding(BuildingId) : nullptr;
}

AFactoryHologram* UFactoryBuildComponent::GetHologram()
{
	if (!Hologram)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Hologram = GetWorld()->SpawnActor<AFactoryHologram>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	return Hologram;
}

void UFactoryBuildComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (Mode)
	{
	case EFactoryBuildMode::Place:		UpdatePlacement(); break;
	case EFactoryBuildMode::Pole:		UpdatePole(); break;
	case EFactoryBuildMode::Belt:		UpdateBelt(); break;
	case EFactoryBuildMode::Demolish:	UpdateDemolish(DeltaTime); break;
	default:							UpdateIdle(); break;
	}
}

void UFactoryBuildComponent::BeginPlacing(FName NewBuildingId)
{
	StopBuilding();

	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(NewBuildingId) : nullptr;

	if (!Building || !GetHologram())
	{
		return;
	}

	BuildingId = NewBuildingId;

	// Gebäude zeigen anfangs vom Spieler weg (Ausgang vorn), in 90°-Schritten
	if (const AFactoryGamePlayerController* PlayerController = GetPlayerController())
	{
		PlaceYaw = FMath::GridSnap(PlayerController->GetControlRotation().Yaw, 90.0);
	}

	switch (Building->Type)
	{
	case EFactoryBuildingType::Pole:
		Mode = EFactoryBuildMode::Pole;
		Hologram->SetPole(PoleHeight);
		break;

	case EFactoryBuildingType::Belt:
		Mode = EFactoryBuildMode::Belt;
		bHasBeltStart = false;
		break;

	default:
		Mode = EFactoryBuildMode::Place;
		Hologram->SetBuilding(*Building);
		break;
	}
}

void UFactoryBuildComponent::ToggleDemolish()
{
	if (Mode == EFactoryBuildMode::Demolish)
	{
		StopBuilding();
		return;
	}

	StopBuilding();
	Mode = EFactoryBuildMode::Demolish;
}

void UFactoryBuildComponent::StopBuilding()
{
	Mode = EFactoryBuildMode::None;
	BuildingId = NAME_None;
	bHasBeltStart = false;
	bHoldingPrimary = false;
	HoldTime = 0.0f;
	SetDemolishTarget(nullptr);

	if (Hologram)
	{
		Hologram->Destroy();
		Hologram = nullptr;
	}

	HidePanel();
}

bool UFactoryBuildComponent::Cancel()
{
	// beim Band zuerst nur den gewählten Start verwerfen
	if (Mode == EFactoryBuildMode::Belt && bHasBeltStart)
	{
		bHasBeltStart = false;
		return false;
	}

	StopBuilding();
	return true;
}

void UFactoryBuildComponent::PrimaryPressed()
{
	switch (Mode)
	{
	case EFactoryBuildMode::Place:		PlaceBuilding(); break;
	case EFactoryBuildMode::Pole:		PlacePole(); break;
	case EFactoryBuildMode::Belt:		PlaceBelt(); break;

	case EFactoryBuildMode::Demolish:
		bHoldingPrimary = true;
		HoldTime = 0.0f;
		break;

	default:
		break;
	}
}

void UFactoryBuildComponent::PrimaryReleased()
{
	bHoldingPrimary = false;
	HoldTime = 0.0f;
}

void UFactoryBuildComponent::Scroll(int32 Direction, bool bFine)
{
	// Stützen und Bänder: Mausrad = Höhe neuer Stützen, Strg + Mausrad dreht die Stütze
	if ((Mode == EFactoryBuildMode::Pole && !bFine) || Mode == EFactoryBuildMode::Belt)
	{
		PoleHeight = FMath::Clamp(PoleHeight + Direction, AFactoryBeltPole::MinHeight, AFactoryBeltPole::MaxHeight);

		if (Mode == EFactoryBuildMode::Pole && Hologram)
		{
			Hologram->SetPole(PoleHeight);
		}

		return;
	}

	if (Mode == EFactoryBuildMode::Place || Mode == EFactoryBuildMode::Pole)
	{
		PlaceYaw = FRotator::NormalizeAxis(PlaceYaw + Direction * (bFine ? 15.0 : 90.0));
	}
}

void UFactoryBuildComponent::CycleMode()
{
	if (Mode == EFactoryBuildMode::Belt)
	{
		BeltMode = static_cast<EFactoryBeltMode>((static_cast<uint8>(BeltMode) + 1) % static_cast<uint8>(EFactoryBeltMode::Count));
	}
}

bool UFactoryBuildComponent::TraceView(double Range, FHitResult& OutHit) const
{
	const AFactoryGamePlayerController* PlayerController = GetPlayerController();
	const UWorld* World = GetWorld();

	if (!PlayerController || !World)
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FCollisionQueryParams Params(TEXT("BuildTrace"), false, PlayerController->GetPawn());

	if (Hologram)
	{
		Params.AddIgnoredActor(Hologram);
	}

	return World->LineTraceSingleByChannel(OutHit, ViewLocation, ViewLocation + ViewRotation.Vector() * Range, ECC_Visibility, Params);
}

bool UFactoryBuildComponent::IsFree(const FVector& Center, const FQuat& Rotation, const FVector& HalfSize) const
{
	FCollisionQueryParams Params(TEXT("BuildOverlap"), false);

	if (Hologram)
	{
		Params.AddIgnoredActor(Hologram);
	}

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, Center, Rotation, GetBlockingObjectTypes(), FCollisionShape::MakeBox(HalfSize), Params);
	return !Overlaps.ContainsByPredicate([](const FOverlapResult& Overlap) { return IsFactoryActor(Overlap.GetActor()); });
}

bool UFactoryBuildComponent::IsBeltPathFree(const FFactoryBeltPath& Path) const
{
	// die Enden sitzen an Port-Kästen bzw. auf Stützen, dort darf das Band anliegen
	constexpr double EndMargin = 75.0;
	constexpr double BoxSpacing = 50.0;
	constexpr int32 CollisionSamplesPerSegment = 16;

	// Bandkörper unter der Oberfläche, etwas schmaler als das Band (80 × 10 cm), damit Nachbarn anliegen dürfen
	const FVector HalfSize(BoxSpacing * 0.5, 36.0, 4.0);

	UWorld* World = GetWorld();

	if (!World || Path.Points.Num() < 2)
	{
		return true;
	}

	TArray<FVector> Samples;

	for (int32 Segment = 0; Segment + 1 < Path.Points.Num(); ++Segment)
	{
		for (int32 Step = (Segment == 0 ? 0 : 1); Step <= CollisionSamplesPerSegment; ++Step)
		{
			Samples.Add(Path.Evaluate(Segment + double(Step) / CollisionSamplesPerSegment));
		}
	}

	double TotalLength = 0.0;

	for (int32 Index = 1; Index < Samples.Num(); ++Index)
	{
		TotalLength += FVector::Dist(Samples[Index - 1], Samples[Index]);
	}

	FCollisionQueryParams Params(TEXT("BeltPathOverlap"), false);

	if (Hologram)
	{
		Params.AddIgnoredActor(Hologram);
	}

	// alle 50 cm eine Box entlang der Kurve, gedreht in Bandrichtung (mit Steigung)
	double Walked = 0.0;
	double NextBox = EndMargin;

	for (int32 Index = 1; Index < Samples.Num() && NextBox <= TotalLength - EndMargin; ++Index)
	{
		const FVector A = Samples[Index - 1];
		const FVector B = Samples[Index];
		const double StepLength = FVector::Dist(A, B);

		while (StepLength > UE_KINDA_SMALL_NUMBER && NextBox <= Walked + StepLength && NextBox <= TotalLength - EndMargin)
		{
			const FVector Direction = (B - A) / StepLength;
			const FVector Center = FMath::Lerp(A, B, (NextBox - Walked) / StepLength) - FVector(0.0, 0.0, HalfSize.Z + 1.0);
			const FQuat Rotation = Direction.Rotation().Quaternion();

			TArray<FOverlapResult> Overlaps;
			World->OverlapMultiByObjectType(Overlaps, Center, Rotation, GetBlockingObjectTypes(), FCollisionShape::MakeBox(HalfSize), Params);

			// Bänder und Stützen dürfen gekreuzt werden, der Spieler stört auch nicht
			const bool bBlocked = Overlaps.ContainsByPredicate([](const FOverlapResult& Overlap)
			{
				const AActor* Actor = Overlap.GetActor();
				return !(Actor && (Actor->IsA<AFactoryBelt>() || Actor->IsA<AFactoryBeltPole>() || Actor->IsA<APawn>()));
			});

			if (bBlocked)
			{
				return false;
			}

			NextBox += BoxSpacing;
		}

		Walked += StepLength;
	}

	return true;
}

void UFactoryBuildComponent::UpdatePlacement()
{
	const FFactoryBuildingDefinition* Building = GetBuilding();
	const AFactoryGamePlayerController* PlayerController = GetPlayerController();
	UWorld* World = GetWorld();

	if (!Building || !Hologram || !PlayerController || !World)
	{
		StopBuilding();
		return;
	}

	TArray<TPair<FText, bool>> CostLines;
	const bool bEnoughMaterial = CheckCosts(Building->GetCosts(), CostLines);
	const FText Controls = FactoryLoc::Get(TEXT("Build_Controls_Place"));
	bPlaceValid = false;

	FHitResult Hit;

	if (!TraceView(BuildRange, Hit))
	{
		Hologram->SetActorHiddenInGame(true);
		ShowPanel(Building->GetDisplayName(), CostLines, FactoryLoc::Get(TEXT("Build_Error_NoGround")), Controls);
		return;
	}

	Hologram->SetActorHiddenInGame(false);

	const FVector Size = Building->GetSize();
	const FQuat Rotation = FRotator(0.0, PlaceYaw, 0.0).Quaternion();

	// aufs 1-m-Raster rasten. Bei ungerader Kantenlänge liegt die Mitte auf einer Zellmitte statt auf einer
	// Rasterlinie, damit die Kanten trotzdem auf dem Raster liegen (nur bei 90°-Drehungen sinnvoll)
	FVector Location = Hit.ImpactPoint;
	const bool bRightAngle = FMath::IsNearlyZero(FMath::Fmod(FMath::Abs(PlaceYaw), 90.0), 0.01);
	const bool bSideways = bRightAngle && FMath::IsNearlyEqual(FMath::Abs(FMath::Fmod(PlaceYaw, 180.0)), 90.0, 0.01);
	const int32 WorldSizeX = bSideways ? Building->SizeY : Building->SizeX;
	const int32 WorldSizeY = bSideways ? Building->SizeX : Building->SizeY;
	const double OffsetX = bRightAngle && WorldSizeX % 2 == 1 ? 50.0 : 0.0;
	const double OffsetY = bRightAngle && WorldSizeY % 2 == 1 ? 50.0 : 0.0;
	Location.X = FMath::GridSnap(Location.X - OffsetX, 100.0) + OffsetX;
	Location.Y = FMath::GridSnap(Location.Y - OffsetY, 100.0) + OffsetY;

	// Boden unter Mitte und Ecken messen; das Gebäude steht auf dem tiefsten Punkt und sinkt in den Rest ein
	FCollisionQueryParams GroundParams(TEXT("BuildGround"), false, PlayerController->GetPawn());
	GroundParams.AddIgnoredActor(Hologram);

	const double HalfX = FMath::Max(Size.X * 0.5 - GroundSampleInset, 0.0);
	const double HalfY = FMath::Max(Size.Y * 0.5 - GroundSampleInset, 0.0);
	const FVector Samples[] = { FVector(0.0, 0.0, 0.0), FVector(HalfX, HalfY, 0.0), FVector(HalfX, -HalfY, 0.0), FVector(-HalfX, HalfY, 0.0), FVector(-HalfX, -HalfY, 0.0) };

	double MinZ = TNumericLimits<double>::Max();
	double MaxZ = TNumericLimits<double>::Lowest();
	bool bGroundEverywhere = true;

	for (const FVector& Sample : Samples)
	{
		const FVector Point = Location + Rotation.RotateVector(Sample);
		FHitResult GroundHit;

		if (World->LineTraceSingleByChannel(GroundHit, FVector(Point.X, Point.Y, Hit.ImpactPoint.Z + 300.0), FVector(Point.X, Point.Y, Hit.ImpactPoint.Z - 500.0), ECC_Visibility, GroundParams))
		{
			MinZ = FMath::Min(MinZ, GroundHit.ImpactPoint.Z);
			MaxZ = FMath::Max(MaxZ, GroundHit.ImpactPoint.Z);
		}
		else
		{
			bGroundEverywhere = false;
		}
	}

	if (bGroundEverywhere)
	{
		Location.Z = MinZ;
	}

	PlaceTransform = FTransform(Rotation, Location);
	Hologram->SetActorTransform(PlaceTransform);

	FText Error;

	if (!bGroundEverywhere || MaxZ - MinZ > MaxGroundDifference)
	{
		Error = FactoryLoc::Get(TEXT("Build_Error_Uneven"));
	}
	else
	{
		// 1. kein anderes Gebäude, Band oder Spieler im ganzen Footprint (5 cm Luft, damit Nachbarn anliegen dürfen)
		const FVector HalfSize = Size * 0.5 - FVector(5.0);
		bool bBlocked = !IsFree(Location + FVector(0.0, 0.0, Size.Z * 0.5), Rotation, HalfSize);

		// 2. oberhalb der Einsink-Zone darf gar nichts sein (Felsen, Wände, Gelände)
		const double FreeBottom = MaxGroundDifference + 5.0;

		if (!bBlocked && Size.Z > FreeBottom + 10.0)
		{
			FCollisionQueryParams OverlapParams(TEXT("BuildOverlap"), false);
			OverlapParams.AddIgnoredActor(Hologram);

			const double FreeHalfHeight = (Size.Z - FreeBottom) * 0.5 - 1.0;
			bBlocked = World->OverlapAnyTestByObjectType(Location + FVector(0.0, 0.0, FreeBottom + FreeHalfHeight), Rotation, GetBlockingObjectTypes(),
				FCollisionShape::MakeBox(FVector(HalfSize.X, HalfSize.Y, FreeHalfHeight)), OverlapParams);
		}

		if (bBlocked)
		{
			Error = FactoryLoc::Get(TEXT("Build_Error_Blocked"));
		}
		else if (!bEnoughMaterial)
		{
			Error = FactoryLoc::Get(TEXT("Build_Error_Materials"));
		}
	}

	bPlaceValid = Error.IsEmpty();
	Hologram->SetValid(bPlaceValid);
	ShowPanel(Building->GetDisplayName(), CostLines, Error, Controls);
}

bool UFactoryBuildComponent::PlaceBuilding()
{
	// noch einmal prüfen: zwischen Tick und Klick kann sich etwas geändert haben
	UpdatePlacement();

	const FFactoryBuildingDefinition* Building = GetBuilding();

	if (!Building || !bPlaceValid)
	{
		return false;
	}

	PayCosts(Building->GetCosts());

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AFactoryMachine* Machine = GetWorld()->SpawnActor<AFactoryMachine>(PlaceTransform.GetLocation(), PlaceTransform.Rotator(), SpawnParams))
	{
		Machine->InitBuilding(BuildingId);
	}

	return true;
}

void UFactoryBuildComponent::UpdatePole()
{
	const FFactoryBuildingDefinition* Building = GetBuilding();

	if (!Building || !Hologram)
	{
		StopBuilding();
		return;
	}

	TArray<TPair<FText, bool>> CostLines;
	const bool bEnoughMaterial = CheckCosts(Building->GetCosts(), CostLines);
	const FText Controls = FactoryLoc::Format(TEXT("Build_Controls_Pole"), { PoleHeight });
	bPlaceValid = false;

	FHitResult Hit;

	if (!TraceView(BuildRange, Hit))
	{
		Hologram->SetActorHiddenInGame(true);
		ShowPanel(Building->GetDisplayName(), CostLines, FactoryLoc::Get(TEXT("Build_Error_NoGround")), Controls);
		return;
	}

	Hologram->SetActorHiddenInGame(false);

	// Stützen stehen auf 1-m-Rasterpunkten, wie die Ports der Maschinen
	FVector Foot = Hit.ImpactPoint;
	Foot.X = FMath::GridSnap(Foot.X, 100.0);
	Foot.Y = FMath::GridSnap(Foot.Y, 100.0);

	const FQuat Rotation = FRotator(0.0, PlaceYaw, 0.0).Quaternion();
	PlaceTransform = FTransform(Rotation, Foot);
	Hologram->SetActorTransform(PlaceTransform);

	FText Error;

	if (IsFactoryActor(Hit.GetActor()) || !IsFree(Foot + FVector(0.0, 0.0, PoleHeight * 50.0 + 5.0), Rotation, FVector(35.0, 35.0, PoleHeight * 50.0 - 10.0)))
	{
		Error = FactoryLoc::Get(TEXT("Build_Error_Blocked"));
	}
	else if (!bEnoughMaterial)
	{
		Error = FactoryLoc::Get(TEXT("Build_Error_Materials"));
	}

	bPlaceValid = Error.IsEmpty();
	Hologram->SetValid(bPlaceValid);
	ShowPanel(Building->GetDisplayName(), CostLines, Error, Controls);
}

bool UFactoryBuildComponent::PlacePole()
{
	UpdatePole();

	const FFactoryBuildingDefinition* Building = GetBuilding();

	if (!Building || !bPlaceValid)
	{
		return false;
	}

	PayCosts(Building->GetCosts());
	SpawnPole(PlaceTransform.GetLocation(), PoleHeight, PlaceTransform.GetRotation().GetForwardVector());
	return true;
}

AFactoryBeltPole* UFactoryBuildComponent::SpawnPole(const FVector& Foot, int32 Height, const FVector& Direction) const
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFactoryBeltPole* Pole = GetWorld()->SpawnActor<AFactoryBeltPole>(Foot, FRotator(0.0, Direction.Rotation().Yaw, 0.0), SpawnParams);

	if (Pole)
	{
		Pole->SetHeight(Height);
	}

	return Pole;
}

bool UFactoryBuildComponent::FindBeltEnd(EFactoryBeltEndWant Want, FFactoryBeltEnd& OutEnd, FText& OutError) const
{
	OutEnd = FFactoryBeltEnd();
	FHitResult Hit;

	if (!TraceView(BuildRange, Hit))
	{
		OutError = FactoryLoc::Get(TEXT("Build_Error_NoGround"));
		return false;
	}

	// Maschine: nächster freier Ausgang bzw. Eingang; bei "egal" gewinnt der nähere
	auto TryMachine = [&OutEnd, Want](AFactoryMachine* Machine, const FVector& Near, double MaxDistance)
	{
		const int32 Output = Want != EFactoryBeltEndWant::Input ? Machine->FindFreePort(false, Near, MaxDistance) : INDEX_NONE;
		const int32 Input = Want != EFactoryBeltEndWant::Output ? Machine->FindFreePort(true, Near, MaxDistance) : INDEX_NONE;
		const TArray<FFactoryPort> Ports = Machine->GetWorldPorts();

		int32 PortIndex = Output;

		if (Input != INDEX_NONE && (Output == INDEX_NONE || FVector::Dist(Ports[Input].Location, Near) < FVector::Dist(Ports[Output].Location, Near)))
		{
			PortIndex = Input;
		}

		if (PortIndex == INDEX_NONE)
		{
			return false;
		}

		OutEnd.Machine = Machine;
		OutEnd.Port = PortIndex;
		OutEnd.Location = Ports[PortIndex].Location;
		OutEnd.Direction = Ports[PortIndex].Direction;
		OutEnd.bBeltEndsHere = Ports[PortIndex].bInput;
		return true;
	};

	// Stütze: hat einen Eingang (Band endet hier) und einen Ausgang (Band beginnt hier)
	auto TryPole = [&OutEnd, Want](AFactoryBeltPole* Pole)
	{
		const bool bOutputFree = !Pole->GetOutputBelt();
		const bool bInputFree = !Pole->GetInputBelt();
		bool bEndsHere = false;

		switch (Want)
		{
		case EFactoryBeltEndWant::Output:	if (!bOutputFree) { return false; } break;
		case EFactoryBeltEndWant::Input:	if (!bInputFree) { return false; } bEndsHere = true; break;

		default:
			// frei lassen, was schon belegt ist: hängt schon ein ankommendes Band dran, geht es weiter (Ausgang)
			if (!bOutputFree && !bInputFree)
			{
				return false;
			}

			bEndsHere = !bOutputFree;
			break;
		}

		OutEnd.Pole = Pole;
		OutEnd.Location = Pole->GetConnectionLocation();
		// ist schon ein Band angeschlossen, ist die Fluss-Richtung der Stütze festgelegt
		OutEnd.Direction = Pole->HasAnyBelt() ? Pole->GetActorForwardVector() : FVector::ZeroVector;
		OutEnd.bBeltEndsHere = bEndsHere;
		return true;
	};

	// direkt auf eine Maschine gezielt: irgendein passender freier Port, egal wie weit
	if (AFactoryMachine* Machine = Cast<AFactoryMachine>(Hit.GetActor()))
	{
		if (TryMachine(Machine, Hit.ImpactPoint, UE_BIG_NUMBER))
		{
			return true;
		}

		OutError = FactoryLoc::Get(TEXT("Build_Error_NoPort"));
		return false;
	}

	// Stützen in der Nähe haben Vorrang, auch wenn ein angeschlossenes Band vor ihnen getroffen wurde
	AFactoryBeltPole* NearestPole = Cast<AFactoryBeltPole>(Hit.GetActor());
	double NearestPoleDistance = SnapDistance;

	if (!NearestPole)
	{
		for (TActorIterator<AFactoryBeltPole> It(GetWorld()); It; ++It)
		{
			const double Distance = FMath::Min(FVector::Dist(It->GetConnectionLocation(), Hit.ImpactPoint), FVector::Dist2D(It->GetActorLocation(), Hit.ImpactPoint));

			if (Distance < NearestPoleDistance)
			{
				NearestPoleDistance = Distance;
				NearestPole = *It;
			}
		}
	}

	if (NearestPole)
	{
		if (TryPole(NearestPole))
		{
			return true;
		}

		OutError = FactoryLoc::Get(TEXT("Build_Error_NoPort"));
		return false;
	}

	// freie Ports in der Nähe (Boden davor oder Bandende davor getroffen)
	for (TActorIterator<AFactoryMachine> It(GetWorld()); It; ++It)
	{
		if (FVector::DistSquared2D(It->GetActorLocation(), Hit.ImpactPoint) < FMath::Square(2000.0))
		{
			const FVector Near = IsFactoryActor(Hit.GetActor()) ? Hit.ImpactPoint : Hit.ImpactPoint + FVector(0.0, 0.0, 100.0);

			if (TryMachine(*It, Near, SnapDistance))
			{
				return true;
			}
		}
	}

	if (IsFactoryActor(Hit.GetActor()))
	{
		OutError = FactoryLoc::Get(TEXT("Build_Error_NoPort"));
		return false;
	}

	// sonst entsteht hier eine neue Stütze (1-m-Raster, Höhe per Mausrad)
	OutEnd.bNewPole = true;
	OutEnd.bBeltEndsHere = Want == EFactoryBeltEndWant::Input;
	OutEnd.PoleFoot = FVector(FMath::GridSnap(Hit.ImpactPoint.X, 100.0), FMath::GridSnap(Hit.ImpactPoint.Y, 100.0), Hit.ImpactPoint.Z);
	OutEnd.PoleHeight = PoleHeight;
	OutEnd.Location = OutEnd.PoleFoot + FVector(0.0, 0.0, PoleHeight * 100.0);

	if (!IsFree(OutEnd.PoleFoot + FVector(0.0, 0.0, PoleHeight * 50.0 + 5.0), FQuat::Identity, FVector(35.0, 35.0, PoleHeight * 50.0 - 10.0)))
	{
		OutError = FactoryLoc::Get(TEXT("Build_Error_Blocked"));
		return false;
	}

	return true;
}

void UFactoryBuildComponent::UpdateBelt()
{
	const FFactoryBuildingDefinition* Building = GetBuilding();
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	const FFactoryBuildingDefinition* PoleBuilding = Buildings ? Buildings->FindBuilding(PoleId) : nullptr;

	if (!Building || !Hologram)
	{
		StopBuilding();
		return;
	}

	// der Start (Maschine/Stütze) kann inzwischen abgerissen worden sein
	if (bHasBeltStart && !BeltStart.bNewPole && !BeltStart.Machine.IsValid() && !BeltStart.Pole.IsValid())
	{
		bHasBeltStart = false;
	}

	const FText Title = FactoryLoc::Format(TEXT("Build_BeltTitle"), { Building->GetDisplayName(), GetBeltModeText(BeltMode) });
	const FText Controls = FactoryLoc::Format(bHasBeltStart ? TEXT("Build_Controls_BeltEnd") : TEXT("Build_Controls_BeltStart"), { PoleHeight });

	// Hologramm steht im Ursprung, Kurve und Stützen werden in Weltkoordinaten gezeigt
	Hologram->SetActorTransform(FTransform::Identity);
	Hologram->SetActorHiddenInGame(false);
	bPlaceValid = false;

	FText Error;
	TArray<TPair<FText, bool>> CostLines;
	TArray<FFactoryItemStack> Costs;
	TArray<FTransform> NewPoles;

	auto AddNewPole = [&Costs, &NewPoles, PoleBuilding](const FFactoryBeltEnd& End, const FVector& Direction)
	{
		if (End.bNewPole)
		{
			NewPoles.Emplace(FRotator(0.0, Direction.Rotation().Yaw, 0.0), End.PoleFoot, FVector(1.0, 1.0, End.PoleHeight));
			AddStacks(Costs, PoleBuilding ? PoleBuilding->GetCosts() : TArray<FFactoryItemStack>());
		}
	};

	if (!bHasBeltStart)
	{
		// 1. Schritt: Start suchen. Ein Eingang geht auch, dann wird das Band rückwärts gezogen
		FFactoryBeltEnd Start;

		if (FindBeltEnd(EFactoryBeltEndWant::Either, Start, Error))
		{
			AddNewPole(Start, Start.Direction.IsZero() ? FVector::ForwardVector : Start.Direction);
		}

		CheckCosts(Costs, CostLines);
		CostLines.Insert(TPair<FText, bool>(FactoryLoc::Get(TEXT("Build_BeltPickStart")), true), 0);
		Hologram->SetBelt(nullptr, NewPoles);
		Hologram->SetValid(Error.IsEmpty());
		ShowPanel(Title, CostLines, Error, Controls);
		return;
	}

	// 2. Schritt: das andere Ende suchen. Begann es an einem Eingang, braucht das andere Ende einen Ausgang
	const bool bReversed = BeltStart.bBeltEndsHere;
	const bool bFoundEnd = FindBeltEnd(bReversed ? EFactoryBeltEndWant::Output : EFactoryBeltEndWant::Input, BeltEnd, Error);

	if (bFoundEnd && BeltEnd.IsSameAs(BeltStart))
	{
		Error = FactoryLoc::Get(TEXT("Build_Error_TooShort"));
	}

	// gebaut wird immer in Fluss-Richtung: vom Ausgang (From) zum Eingang (To)
	const FFactoryBeltEnd& From = bReversed ? BeltEnd : BeltStart;
	const FFactoryBeltEnd& To = bReversed ? BeltStart : BeltEnd;
	EFactoryBeltPathProblem Problem = EFactoryBeltPathProblem::None;

	if (Error.IsEmpty())
	{
		Problem = FactoryBeltPath::Build(BeltMode, From.Location, From.Direction, To.Location, To.Direction, BeltPath);
		Error = GetPathProblemText(Problem);

		if (Error.IsEmpty() && !IsBeltPathFree(BeltPath))
		{
			Error = FactoryLoc::Get(TEXT("Build_Error_Blocked"));
		}

		AddStacks(Costs, AFactoryBelt::GetBuildCostsForLength(this, BeltPath.Length, 1));
		AddNewPole(From, BeltPath.StartDirection);
		AddNewPole(To, BeltPath.EndDirection);
	}

	const bool bEnoughMaterial = CheckCosts(Costs, CostLines);

	if (Error.IsEmpty() && !bEnoughMaterial)
	{
		Error = FactoryLoc::Get(TEXT("Build_Error_Materials"));
	}

	// die Kurve auch zeigen, wenn sie nur zu steil oder zu eng ist, damit man sieht, was nicht passt
	const bool bShowPath = bFoundEnd && BeltPath.Points.Num() >= 2 && Problem != EFactoryBeltPathProblem::Impossible;
	Hologram->SetBelt(bShowPath ? &BeltPath : nullptr, NewPoles);

	bPlaceValid = Error.IsEmpty();
	Hologram->SetValid(bPlaceValid);
	ShowPanel(Title, CostLines, Error, Controls);
}

bool UFactoryBuildComponent::PlaceBelt()
{
	// 1. Klick: Start merken (gebaut wird erst mit dem zweiten)
	if (!bHasBeltStart)
	{
		FText Error;

		if (FindBeltEnd(EFactoryBeltEndWant::Either, BeltStart, Error))
		{
			bHasBeltStart = true;
		}

		return false;
	}

	UpdateBelt();

	if (!bPlaceValid)
	{
		return false;
	}

	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	const FFactoryBuildingDefinition* PoleBuilding = Buildings ? Buildings->FindBuilding(PoleId) : nullptr;

	const bool bReversed = BeltStart.bBeltEndsHere;
	const FFactoryBeltEnd From = bReversed ? BeltEnd : BeltStart;
	const FFactoryBeltEnd To = bReversed ? BeltStart : BeltEnd;

	TArray<FFactoryItemStack> Costs = AFactoryBelt::GetBuildCostsForLength(this, BeltPath.Length, 1);

	// neue Stützen an den Enden mitbauen, ausgerichtet in Fluss-Richtung
	AFactoryBeltPole* FromPole = From.Pole.Get();
	AFactoryBeltPole* ToPole = To.Pole.Get();

	if (From.bNewPole)
	{
		FromPole = SpawnPole(From.PoleFoot, From.PoleHeight, BeltPath.StartDirection);
		AddStacks(Costs, PoleBuilding ? PoleBuilding->GetCosts() : TArray<FFactoryItemStack>());
	}

	if (To.bNewPole)
	{
		ToPole = SpawnPole(To.PoleFoot, To.PoleHeight, BeltPath.EndDirection);
		AddStacks(Costs, PoleBuilding ? PoleBuilding->GetCosts() : TArray<FFactoryItemStack>());
	}

	// eine bisher freie Stütze übernimmt die Richtung des ersten Bandes
	if (FromPole && !FromPole->HasAnyBelt())
	{
		FromPole->SetActorRotation(FRotator(0.0, BeltPath.StartDirection.Rotation().Yaw, 0.0));
	}

	if (ToPole && !ToPole->HasAnyBelt())
	{
		ToPole->SetActorRotation(FRotator(0.0, BeltPath.EndDirection.Rotation().Yaw, 0.0));
	}

	PayCosts(Costs);

	AFactoryBelt* NewBelt = AFactoryBelt::SpawnBelt(GetWorld(), BeltPath, 1);

	if (!NewBelt)
	{
		return false;
	}

	if (From.Machine.IsValid())
	{
		NewBelt->ConnectStartToMachine(From.Machine.Get(), From.Port);
	}
	else
	{
		NewBelt->ConnectStartToPole(FromPole);
	}

	if (To.Machine.IsValid())
	{
		NewBelt->ConnectEndToMachine(To.Machine.Get(), To.Port);
	}
	else
	{
		NewBelt->ConnectEndToPole(ToPole);
	}

	// Kette: am zweiten Klick geht es weiter, wenn dort eine Stütze mit freier Seite steht
	// (vorwärts: ihr Ausgang, rückwärts: ihr Eingang). An einer Maschine ist die Kette zu Ende
	AFactoryBeltPole* ChainPole = bReversed ? FromPole : ToPole;
	const bool bChainSideFree = ChainPole && (bReversed ? !ChainPole->GetInputBelt() : !ChainPole->GetOutputBelt());

	if (!bChainSideFree)
	{
		bHasBeltStart = false;
		return true;
	}

	BeltStart = FFactoryBeltEnd();
	BeltStart.Pole = ChainPole;
	BeltStart.Location = ChainPole->GetConnectionLocation();
	BeltStart.Direction = bReversed ? BeltPath.StartDirection : BeltPath.EndDirection;
	BeltStart.bBeltEndsHere = bReversed;
	return true;
}

void UFactoryBuildComponent::PayCosts(const TArray<FFactoryItemStack>& Costs) const
{
	const AFactoryGamePlayerController* PlayerController = GetPlayerController();
	const AFactoryGameCharacter* Character = PlayerController ? Cast<AFactoryGameCharacter>(PlayerController->GetPawn()) : nullptr;
	UFactoryInventoryComponent* Inventory = Character ? Character->GetInventory() : nullptr;

	if (!Inventory)
	{
		return;
	}

	for (const FFactoryItemStack& Cost : Costs)
	{
		Inventory->RemoveItem(Cost.ItemId, Cost.Count);

		if (UFactoryHUDWidget* HUD = PlayerController->GetHUDWidget())
		{
			HUD->ShowItemNotification(Cost.ItemId, -Cost.Count);
		}
	}
}

void UFactoryBuildComponent::UpdateDemolish(float DeltaTime)
{
	FHitResult Hit;
	AActor* Target = nullptr;

	if (TraceView(BuildRange, Hit) && (Hit.GetActor() && (Hit.GetActor()->IsA<AFactoryMachine>() || Hit.GetActor()->IsA<AFactoryBelt>() || Hit.GetActor()->IsA<AFactoryBeltPole>())))
	{
		Target = Hit.GetActor();
	}

	// neues Ziel: Halten beginnt von vorn
	if (Target != DemolishTarget.Get())
	{
		SetDemolishTarget(Target);
		HoldTime = 0.0f;
	}

	const FText Controls = FactoryLoc::Get(TEXT("Build_Controls_Demolish"));

	if (!Target)
	{
		ShowPanel(FactoryLoc::Get(TEXT("Build_DemolishTitle")), {}, FactoryLoc::Get(TEXT("Build_Error_NoTarget")), Controls);
		return;
	}

	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	FText TargetName;

	if (const AFactoryMachine* Machine = Cast<AFactoryMachine>(Target))
	{
		TargetName = Buildings ? Buildings->GetDisplayName(Machine->GetProduction()->GetBuildingId()) : FText::GetEmpty();
	}
	else if (const AFactoryBelt* Belt = Cast<AFactoryBelt>(Target))
	{
		TargetName = FactoryLoc::Format(TEXT("Belt_Name"), { Belt->GetBelt()->GetTier() });
	}
	else
	{
		TargetName = Buildings ? Buildings->GetDisplayName(PoleId) : FText::GetEmpty();
	}

	TArray<TPair<FText, bool>> Lines;

	if (bHoldingPrimary)
	{
		HoldTime += DeltaTime;

		if (HoldTime >= DemolishHoldTime)
		{
			// erneut halten für das nächste Gebäude, damit man nicht versehentlich eine ganze Reihe abreißt
			bHoldingPrimary = false;
			HoldTime = 0.0f;
			SetDemolishTarget(nullptr);
			Demolish(Target);
			return;
		}

		// in 10-%-Schritten, damit das HUD nicht jedes Bild neu gebaut wird
		const int32 Percent = FMath::FloorToInt32(HoldTime / DemolishHoldTime * 10.0f) * 10;
		Lines.Emplace(FactoryLoc::Format(TEXT("Build_DemolishProgress"), { Percent }), true);
	}

	ShowPanel(FactoryLoc::Format(TEXT("Build_DemolishTarget"), { TargetName }), Lines, FText::GetEmpty(), Controls);
}

void UFactoryBuildComponent::Demolish(AActor* Target)
{
	AFactoryGamePlayerController* PlayerController = GetPlayerController();
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);

	if (!Target || !PlayerController)
	{
		return;
	}

	// alles zurück: Baukosten + Inhalt
	TArray<FFactoryItemStack> Refund;
	TArray<AActor*> ToDestroy = { Target };

	auto RefundBelt = [&Refund](AFactoryBelt* Belt)
	{
		AddStacks(Refund, Belt->GetBuildCosts());

		TArray<FName> Items;
		Belt->GetBelt()->ClearItems(Items);

		for (const FName& ItemId : Items)
		{
			AddStacks(Refund, { FFactoryItemStack(ItemId, 1) });
		}
	};

	if (AFactoryMachine* Machine = Cast<AFactoryMachine>(Target))
	{
		if (const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(Machine->GetProduction()->GetBuildingId()) : nullptr)
		{
			AddStacks(Refund, Building->GetCosts());
		}

		// Rezept abwählen gibt Puffer und die Zutaten eines laufenden Durchlaufs heraus.
		// Angeschlossene Bänder bleiben stehen, ihr Ende ist dann offen
		TArray<FFactoryItemStack> Contents;
		Machine->GetProduction()->SetRecipe(NAME_None, Contents);
		AddStacks(Refund, Contents);
	}
	else if (AFactoryBelt* Belt = Cast<AFactoryBelt>(Target))
	{
		RefundBelt(Belt);
	}
	else if (AFactoryBeltPole* Pole = Cast<AFactoryBeltPole>(Target))
	{
		if (const FFactoryBuildingDefinition* PoleBuilding = Buildings ? Buildings->FindBuilding(PoleId) : nullptr)
		{
			AddStacks(Refund, PoleBuilding->GetCosts());
		}

		// eine Stütze trägt ihre Bänder: die werden mit abgerissen
		for (AFactoryBelt* PoleBelt : { Pole->GetInputBelt(), Pole->GetOutputBelt() })
		{
			if (PoleBelt)
			{
				RefundBelt(PoleBelt);
				ToDestroy.Add(PoleBelt);
			}
		}
	}

	TArray<FFactoryItemStack> Leftovers;

	for (const FFactoryItemStack& Stack : Refund)
	{
		const int32 Added = PlayerController->GiveItems(Stack.ItemId, Stack.Count);

		if (Added < Stack.Count)
		{
			Leftovers.Emplace(Stack.ItemId, Stack.Count - Added);
		}
	}

	// was nicht ins Inventar passt, geht nicht verloren, sondern landet in einer Abriss-Kiste
	if (Leftovers.Num() > 0)
	{
		AFactoryCrate::SpawnCrate(GetWorld(), Target->GetActorLocation(), Leftovers);

		if (UFactoryHUDWidget* HUD = PlayerController->GetHUDWidget())
		{
			HUD->ShowNotification(FactoryLoc::Get(TEXT("Notify_CrateSpawned")), EFactoryNotificationType::Warning);
		}
	}

	for (AActor* Actor : ToDestroy)
	{
		Actor->Destroy();
	}
}

void UFactoryBuildComponent::SetDemolishTarget(AActor* NewTarget)
{
	auto Highlight = [](AActor* Actor, bool bOn)
	{
		if (AFactoryMachine* Machine = Cast<AFactoryMachine>(Actor))
		{
			Machine->SetDemolishHighlight(bOn);
		}
		else if (AFactoryBelt* Belt = Cast<AFactoryBelt>(Actor))
		{
			Belt->SetDemolishHighlight(bOn);
		}
		else if (AFactoryBeltPole* Pole = Cast<AFactoryBeltPole>(Actor))
		{
			Pole->SetDemolishHighlight(bOn);

			// die Bänder an der Stütze werden mit abgerissen, also auch markieren
			for (AFactoryBelt* PoleBelt : { Pole->GetInputBelt(), Pole->GetOutputBelt() })
			{
				if (PoleBelt)
				{
					PoleBelt->SetDemolishHighlight(bOn);
				}
			}
		}
	};

	Highlight(DemolishTarget.Get(), false);
	DemolishTarget = NewTarget;
	Highlight(NewTarget, true);
}

void UFactoryBuildComponent::UpdateIdle()
{
	FHitResult Hit;

	if (TraceView(InteractRange, Hit) && Cast<AFactoryCrate>(Hit.GetActor()))
	{
		ShowPanel(FText::GetEmpty(), {}, FText::GetEmpty(), FactoryLoc::Get(TEXT("Crate_Prompt")));
	}
	else if (const AFactoryMachine* Machine = Hit.bBlockingHit ? Cast<AFactoryMachine>(Hit.GetActor()) : nullptr)
	{
		const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
		const FText Name = Buildings ? Buildings->GetDisplayName(Machine->GetProduction()->GetBuildingId()) : FText::GetEmpty();
		ShowPanel(FText::GetEmpty(), {}, FText::GetEmpty(), FactoryLoc::Format(TEXT("Machine_Prompt"), { Name }));
	}
	else
	{
		HidePanel();
	}
}

void UFactoryBuildComponent::Interact()
{
	FHitResult Hit;

	if (TraceView(InteractRange, Hit))
	{
		if (AFactoryCrate* Crate = Cast<AFactoryCrate>(Hit.GetActor()))
		{
			Crate->TakeItems(GetPlayerController());
		}
		else if (AFactoryMachine* Machine = Cast<AFactoryMachine>(Hit.GetActor()))
		{
			if (AFactoryGamePlayerController* PlayerController = GetPlayerController())
			{
				PlayerController->OpenMachinePanel(Machine);
			}
		}
	}
}

bool UFactoryBuildComponent::CheckCosts(const TArray<FFactoryItemStack>& Costs, TArray<TPair<FText, bool>>& OutLines) const
{
	const AFactoryGamePlayerController* PlayerController = GetPlayerController();
	const AFactoryGameCharacter* Character = PlayerController ? Cast<AFactoryGameCharacter>(PlayerController->GetPawn()) : nullptr;
	const UFactoryInventoryComponent* Inventory = Character ? Character->GetInventory() : nullptr;
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);

	bool bEnough = true;

	// eine Zeile pro Kostenposten: "8 / 10 Eisenplatte", rot wenn es nicht reicht
	for (const FFactoryItemStack& Cost : Costs)
	{
		const int32 Have = Inventory ? Inventory->GetItemCount(Cost.ItemId) : 0;
		const bool bHasEnough = Have >= Cost.Count;
		bEnough &= bHasEnough;

		const FText Name = Items ? Items->GetDisplayName(Cost.ItemId) : FText::FromName(Cost.ItemId);
		OutLines.Emplace(FactoryLoc::Format(TEXT("Build_Cost"), { Have, Cost.Count, Name }), bHasEnough);
	}

	return bEnough;
}

void UFactoryBuildComponent::ShowPanel(const FText& Title, const TArray<TPair<FText, bool>>& Lines, const FText& Error, const FText& Controls) const
{
	if (const AFactoryGamePlayerController* PlayerController = GetPlayerController())
	{
		if (UFactoryHUDWidget* HUD = PlayerController->GetHUDWidget())
		{
			HUD->SetBuildPanel(Title, Lines, Error, Controls);
		}
	}
}

void UFactoryBuildComponent::HidePanel() const
{
	ShowPanel(FText::GetEmpty(), {}, FText::GetEmpty(), FText::GetEmpty());
}
