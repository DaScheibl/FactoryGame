// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryBuildComponent.h"
#include "FactoryBuildingDatabase.h"
#include "FactoryHologram.h"
#include "FactoryMachine.h"
#include "FactoryBelt.h"
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
#include "GameFramework/Pawn.h"

namespace
{
	// Würfel-Ecken leicht nach innen, damit die Boden-Messung nicht schon die Nachbarkachel trifft
	constexpr double GroundSampleInset = 50.0;

	// Gebäude-Actors, die sich nie überlappen dürfen
	bool IsFactoryActor(const AActor* Actor)
	{
		return Actor && (Actor->IsA<AFactoryMachine>() || Actor->IsA<AFactoryBelt>() || Actor->IsA<AFactoryCrate>() || Actor->IsA<APawn>());
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

void UFactoryBuildComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (Mode)
	{
	case EFactoryBuildMode::Place:		UpdatePlacement(); break;
	case EFactoryBuildMode::Demolish:	UpdateDemolish(DeltaTime); break;
	default:							UpdateIdle(); break;
	}
}

void UFactoryBuildComponent::BeginPlacing(FName NewBuildingId)
{
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(NewBuildingId) : nullptr;

	if (!Building)
	{
		StopBuilding();
		return;
	}

	SetDemolishTarget(nullptr);
	Mode = EFactoryBuildMode::Place;
	BuildingId = NewBuildingId;

	// das Gebäude zeigt anfangs vom Spieler weg (Ausgang vorn), in 90°-Schritten
	if (const AFactoryGamePlayerController* PlayerController = GetPlayerController())
	{
		PlaceYaw = FMath::GridSnap(PlayerController->GetControlRotation().Yaw, 90.0);
	}

	if (!Hologram)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Hologram = GetWorld()->SpawnActor<AFactoryHologram>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	if (Hologram)
	{
		Hologram->SetBuilding(*Building);
	}

	UpdatePlacement();
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

void UFactoryBuildComponent::PrimaryPressed()
{
	if (Mode == EFactoryBuildMode::Place)
	{
		PlaceBuilding();
	}
	else if (Mode == EFactoryBuildMode::Demolish)
	{
		bHoldingPrimary = true;
		HoldTime = 0.0f;
	}
}

void UFactoryBuildComponent::PrimaryReleased()
{
	bHoldingPrimary = false;
	HoldTime = 0.0f;
}

void UFactoryBuildComponent::Rotate(int32 Direction, bool bFine)
{
	if (Mode == EFactoryBuildMode::Place)
	{
		PlaceYaw = FRotator::NormalizeAxis(PlaceYaw + Direction * (bFine ? 15.0 : 90.0));
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

void UFactoryBuildComponent::UpdatePlacement()
{
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(BuildingId) : nullptr;
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
		FCollisionObjectQueryParams ObjectTypes;
		ObjectTypes.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjectTypes.AddObjectTypesToQuery(ECC_Pawn);
		ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);

		FCollisionQueryParams OverlapParams(TEXT("BuildOverlap"), false);
		OverlapParams.AddIgnoredActor(Hologram);

		// 1. kein anderes Gebäude, Band oder Spieler im ganzen Footprint (5 cm Luft, damit Nachbarn anliegen dürfen)
		TArray<FOverlapResult> Overlaps;
		const FVector HalfSize = Size * 0.5 - FVector(5.0);
		World->OverlapMultiByObjectType(Overlaps, Location + FVector(0.0, 0.0, Size.Z * 0.5), Rotation, ObjectTypes, FCollisionShape::MakeBox(HalfSize), OverlapParams);

		bool bBlocked = Overlaps.ContainsByPredicate([](const FOverlapResult& Overlap) { return IsFactoryActor(Overlap.GetActor()); });

		// 2. oberhalb der Einsink-Zone darf gar nichts sein (Felsen, Wände, Gelände)
		const double FreeBottom = MaxGroundDifference + 5.0;

		if (!bBlocked && Size.Z > FreeBottom + 10.0)
		{
			const double FreeHalfHeight = (Size.Z - FreeBottom) * 0.5 - 1.0;
			bBlocked = World->OverlapAnyTestByObjectType(Location + FVector(0.0, 0.0, FreeBottom + FreeHalfHeight), Rotation, ObjectTypes,
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
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(BuildingId) : nullptr;
	AFactoryGamePlayerController* PlayerController = GetPlayerController();
	const AFactoryGameCharacter* Character = PlayerController ? Cast<AFactoryGameCharacter>(PlayerController->GetPawn()) : nullptr;
	UFactoryInventoryComponent* Inventory = Character ? Character->GetInventory() : nullptr;

	// noch einmal prüfen: zwischen Tick und Klick kann sich etwas geändert haben
	UpdatePlacement();

	if (!Building || !Inventory || !bPlaceValid)
	{
		return false;
	}

	for (const FFactoryItemStack& Cost : Building->GetCosts())
	{
		Inventory->RemoveItem(Cost.ItemId, Cost.Count);

		if (UFactoryHUDWidget* HUD = PlayerController->GetHUDWidget())
		{
			HUD->ShowItemNotification(Cost.ItemId, -Cost.Count);
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AFactoryMachine* Machine = GetWorld()->SpawnActor<AFactoryMachine>(PlaceTransform.GetLocation(), PlaceTransform.Rotator(), SpawnParams))
	{
		Machine->InitBuilding(BuildingId);
	}

	return true;
}

void UFactoryBuildComponent::UpdateDemolish(float DeltaTime)
{
	FHitResult Hit;
	AActor* Target = nullptr;

	if (TraceView(BuildRange, Hit) && (Cast<AFactoryMachine>(Hit.GetActor()) || Cast<AFactoryBelt>(Hit.GetActor())))
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

	FText TargetName;

	if (const AFactoryMachine* Machine = Cast<AFactoryMachine>(Target))
	{
		const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
		TargetName = Buildings ? Buildings->GetDisplayName(Machine->GetProduction()->GetBuildingId()) : FText::GetEmpty();
	}
	else if (const AFactoryBelt* Belt = Cast<AFactoryBelt>(Target))
	{
		TargetName = FactoryLoc::Format(TEXT("Belt_Name"), { Belt->GetBelt()->GetTier() });
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

	if (!Target || !PlayerController)
	{
		return;
	}

	// alles zurück: Baukosten + Inhalt
	TArray<FFactoryItemStack> Refund;

	if (AFactoryMachine* Machine = Cast<AFactoryMachine>(Target))
	{
		const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);

		if (const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(Machine->GetProduction()->GetBuildingId()) : nullptr)
		{
			AddStacks(Refund, Building->GetCosts());
		}

		// Rezept abwählen gibt Puffer und die Zutaten eines laufenden Durchlaufs heraus
		TArray<FFactoryItemStack> Contents;
		Machine->GetProduction()->SetRecipe(NAME_None, Contents);
		AddStacks(Refund, Contents);
	}
	else if (AFactoryBelt* Belt = Cast<AFactoryBelt>(Target))
	{
		AddStacks(Refund, Belt->GetBuildCosts());

		TArray<FName> Items;
		Belt->GetBelt()->ClearItems(Items);

		for (const FName& ItemId : Items)
		{
			AddStacks(Refund, { FFactoryItemStack(ItemId, 1) });
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

	Target->Destroy();
}

void UFactoryBuildComponent::SetDemolishTarget(AActor* NewTarget)
{
	if (AActor* Old = DemolishTarget.Get())
	{
		if (AFactoryMachine* Machine = Cast<AFactoryMachine>(Old))
		{
			Machine->SetDemolishHighlight(false);
		}
		else if (AFactoryBelt* Belt = Cast<AFactoryBelt>(Old))
		{
			Belt->SetDemolishHighlight(false);
		}
	}

	DemolishTarget = NewTarget;

	if (AFactoryMachine* Machine = Cast<AFactoryMachine>(NewTarget))
	{
		Machine->SetDemolishHighlight(true);
	}
	else if (AFactoryBelt* Belt = Cast<AFactoryBelt>(NewTarget))
	{
		Belt->SetDemolishHighlight(true);
	}
}

void UFactoryBuildComponent::UpdateIdle()
{
	FHitResult Hit;

	if (TraceView(InteractRange, Hit) && Cast<AFactoryCrate>(Hit.GetActor()))
	{
		ShowPanel(FText::GetEmpty(), {}, FText::GetEmpty(), FactoryLoc::Get(TEXT("Crate_Prompt")));
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
