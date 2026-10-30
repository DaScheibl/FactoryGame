// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryHologram.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class USplineComponent;
class USplineMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
struct FFactoryBuildingDefinition;
struct FFactoryBeltPath;

/**
 *  Preview while building: green if it can be built, red if not.
 *
 *  Drei Formen, je nachdem was gebaut wird:
 *  - Gebäude: Würfel in Gebäudegröße mit Port-Platten (blau = Eingang, orange = Ausgang)
 *  - Bandstütze: Säule mit Platte in der gewählten Höhe
 *  - Band: Vorschau der Kurve plus die Stützen, die mitgebaut werden
 *  Graybox: deckend statt echtem Hologramm-Shader (kommt mit MAT_Hologram). Keine Kollision,
 *  Line Traces gehen durch das Hologramm hindurch.
 */
UCLASS()
class FACTORYGAME_API AFactoryHologram : public AActor
{
	GENERATED_BODY()

public:

	AFactoryHologram();

	/** Shows a building: box in its size with port plates */
	void SetBuilding(const FFactoryBuildingDefinition& Building);

	/** Shows a single pole with the given height in meters */
	void SetPole(int32 Height);

	/**
	 *  Shows a belt path and the poles that are built with it (world space; pole transforms are at the
	 *  pole foot, Z scale = height in meters). The actor must be at the world origin
	 */
	void SetBelt(const FFactoryBeltPath* Path, const TArray<FTransform>& NewPoles);

	/** Shows unit cubes at the given world transforms (scale in meters), e.g. foundations. The actor must be at the world origin */
	void SetBoxes(const TArray<FTransform>& Boxes);

	/** Green when it can be built, red when not */
	void SetValid(bool bValid);

private:

	/** Hides all parts, the Set... functions then show what they need */
	void HideAll();

	/** Returns the pillar and plate pair with the given index, creating it on first use */
	void ShowPoleMeshes(int32 Index, const FTransform& Foot, int32 Height);

	UStaticMeshComponent* CreatePart();

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PortMarkers;

	/** Pillar and plate of each previewed pole, two entries per pole */
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PoleMeshes;

	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> BeltMeshes;

	/** Boxes shown by SetBoxes */
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> BoxMeshes;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> BaseMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> Material;

	/** Last value passed to SetValid, -1 before the first call */
	int32 ValidState = -1;
};
