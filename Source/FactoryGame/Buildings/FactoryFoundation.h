// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryFoundation.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
struct FFactoryBuildingDefinition;

/**
 *  A foundation (8 x 8 m, 1 / 2 / 4 m high) or a ramp of the same size. Machines on a foundation snap to its 1 m grid,
 *  new foundations snap flush to the edges of the ones nearby.
 *
 *  Graybox: Fundament = Quader, Rampe = schräge Platte (20 cm dick), die hinten (-X) am Boden beginnt
 *  und vorn (+X) die volle Höhe erreicht. Ursprung unten mittig wie bei Maschinen.
 *  Regeln: Docs/Standards/13-bauen.md (Fundamente)
 */
UCLASS()
class FACTORYGAME_API AFactoryFoundation : public AActor
{
	GENERATED_BODY()

public:

	/** Edge length of a foundation tile in cm */
	static constexpr double TileSize = 800.0;

	AFactoryFoundation();

	/** Sets building id and size from the building table */
	void InitBuilding(FName InBuildingId);

	FName GetBuildingId() const { return BuildingId; }

	bool IsRamp() const { return bRamp; }

	/** Height in cm */
	double GetHeight() const { return Height; }

	/** World height of the top surface (for ramps: the high end) */
	double GetTopZ() const { return GetActorLocation().Z + Height; }

	/** Unit cube transforms (relative to the foundation origin) that make up the graybox of a foundation or ramp */
	static TArray<FTransform> GetPieces(const FFactoryBuildingDefinition& Building);

	/** Tints the foundation red while it is targeted for demolition */
	void SetDemolishHighlight(bool bHighlighted);

private:

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> Material;

	FName BuildingId;

	bool bRamp = false;

	double Height = 100.0;
};
