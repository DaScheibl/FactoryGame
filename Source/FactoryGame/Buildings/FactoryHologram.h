// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryHologram.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
struct FFactoryBuildingDefinition;

/**
 *  Preview of a building while placing it: a box in the building's size, green if it can be built, red if not.
 *
 *  Graybox: deckender Würfel statt echtem Hologramm-Shader (kommt mit MAT_Hologram). Die Ports werden
 *  wie an der Maschine als Platten gezeigt (blau = Eingang, orange = Ausgang), damit man beim Drehen sieht,
 *  wo Bänder andocken. Keine Kollision, Line Traces gehen durch das Hologramm hindurch.
 */
UCLASS()
class FACTORYGAME_API AFactoryHologram : public AActor
{
	GENERATED_BODY()

public:

	AFactoryHologram();

	/** Resizes the box and creates the port plates for a building */
	void SetBuilding(const FFactoryBuildingDefinition& Building);

	/** Green when the building can be placed, red when not */
	void SetValid(bool bValid);

private:

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PortMarkers;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> Material;

	/** Last value passed to SetValid, -1 before the first call */
	int32 ValidState = -1;
};
