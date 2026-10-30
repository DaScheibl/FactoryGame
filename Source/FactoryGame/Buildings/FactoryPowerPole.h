// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryPowerPole.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 *  A power pole (Mk1 / Mk2 / Mk3). Power cables run from its top to other poles and machines,
 *  the building table sets how many (PowerConnections: 4 / 8 / 12).
 *
 *  Graybox: schmaler Mast (25 cm) mit Querbalken oben, Ursprung unten mittig wie bei Maschinen.
 *  Regeln: Docs/Standards/17-strom.md
 */
UCLASS()
class FACTORYGAME_API AFactoryPowerPole : public AActor
{
	GENERATED_BODY()

public:

	AFactoryPowerPole();

	/** Sets building id, height and number of connections from the building table */
	void InitBuilding(FName InBuildingId);

	FName GetBuildingId() const { return BuildingId; }

	/** How many cables can be attached */
	int32 GetMaxConnections() const { return MaxConnections; }

	/** Where cables attach, in world space */
	FVector GetConnectionPoint() const;

	/** Tints the pole red while it is targeted for demolition */
	void SetDemolishHighlight(bool bHighlighted);

protected:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mast;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Crossbar;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> Materials;

	FName BuildingId;

	int32 MaxConnections = 4;

	/** Height in cm */
	double Height = 500.0;
};
