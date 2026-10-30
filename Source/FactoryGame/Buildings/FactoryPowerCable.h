// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryItemTypes.h"
#include "FactoryPowerCable.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 *  A power cable between two nodes of a power network: power poles and machines (generators included).
 *  Poles take 4 / 8 / 12 cables, machines one (PowerConnections in the building table).
 *
 *  Graybox: gerade, dünne Leitung ohne Kollision (Durchhang kommt später, LOG_PowerCable).
 *  Weil sie keine Kollision hat, sucht der Abriss sie über den Abstand zur Blickrichtung (FindCableNearRay).
 *  Regeln: Docs/Standards/17-strom.md
 */
UCLASS()
class FACTORYGAME_API AFactoryPowerCable : public AActor
{
	GENERATED_BODY()

public:

	/** Building table id of the cable */
	static const FName CableId;

	/** Longest cable between two connection points, in cm */
	static constexpr double MaxLength = 2500.0;

	/** The cable costs are charged per started piece of this length, in cm */
	static constexpr double CostLength = 500.0;

	AFactoryPowerCable();

	/** Connects both ends and builds the visual */
	void Connect(AActor* InEndA, AActor* InEndB);

	AActor* GetEndA() const { return EndA.Get(); }
	AActor* GetEndB() const { return EndB.Get(); }

	/** Returns the other end, or nullptr if Node isn't one of the ends */
	AActor* GetOtherEnd(const AActor* Node) const;

	double GetLength() const;

	/** What this cable cost, for the refund */
	TArray<FFactoryItemStack> GetBuildCosts() const { return GetCosts(this, GetLength()); }

	void SetDemolishHighlight(bool bHighlighted);

	/** Costs of a cable of the given length (cm) */
	static TArray<FFactoryItemStack> GetCosts(const UObject* WorldContextObject, double Length);

	/** True for actors a cable can attach to: power poles and machines with a power connection */
	static bool IsPowerNode(const AActor* Actor);

	/** Where cables attach to a pole or machine, in world space */
	static FVector GetConnectionPoint(const AActor* Node);

	/** How many cables a pole or machine takes */
	static int32 GetMaxConnections(const AActor* Node);

	/** All cables attached to a pole or machine */
	static TArray<AFactoryPowerCable*> GetCables(const AActor* Node);

	/** True if a cable already runs between both nodes */
	static bool AreConnected(const AActor* A, const AActor* B);

	/** Finds the cable closest to a view ray (within Tolerance cm) and returns it with its distance along the ray */
	static AFactoryPowerCable* FindCableNearRay(const UWorld* World, const FVector& Start, const FVector& Direction, double MaxDistance, double Tolerance, double& OutDistance);

	/** Unit cube transform of a straight cable piece between two points (also used by the build hologram) */
	static FTransform GetCableTransform(const FVector& From, const FVector& To, double Thickness);

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	void UpdateVisual();

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Wire;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> Material;

	TWeakObjectPtr<AActor> EndA;
	TWeakObjectPtr<AActor> EndB;
};
