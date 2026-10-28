// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryBeltPole.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class AFactoryBelt;

/**
 *  Conveyor pole (LOG_BeltPole): a connection point between two belts, 1-7 m high.
 *
 *  Graybox: dünne Säule mit einer Platte oben, die Oberkante der Platte ist die Bandhöhe.
 *  Eine Stütze hat genau einen Eingang (Band, das hier endet) und einen Ausgang (Band, das hier beginnt).
 *  Kommt ein Band an, gibt es seine Items direkt an das abgehende Band weiter (AFactoryBelt verknüpft das).
 *  Die Stütze zeigt in Fluss-Richtung (X-Achse); die Richtung wird festgelegt, sobald ein Band angeschlossen wird.
 */
UCLASS()
class FACTORYGAME_API AFactoryBeltPole : public AActor
{
	GENERATED_BODY()

public:

	static constexpr int32 MinHeight = 1;
	static constexpr int32 MaxHeight = 7;

	AFactoryBeltPole();

	/** Sets the height of the belt surface in meters (1-7) */
	void SetHeight(int32 NewHeight);

	int32 GetHeight() const { return Height; }

	/** Returns the belt connection point (top of the plate) in world space */
	FVector GetConnectionLocation() const;

	/** Belt ending here, may be null */
	AFactoryBelt* GetInputBelt() const { return InputBelt.Get(); }

	/** Belt starting here, may be null */
	AFactoryBelt* GetOutputBelt() const { return OutputBelt.Get(); }

	void SetInputBelt(AFactoryBelt* Belt) { InputBelt = Belt; }
	void SetOutputBelt(AFactoryBelt* Belt) { OutputBelt = Belt; }

	/** True if a belt is connected on either side, then the flow direction of the pole is fixed */
	bool HasAnyBelt() const { return InputBelt.IsValid() || OutputBelt.IsValid(); }

	/** Tints the pole red while it is targeted for demolition */
	void SetDemolishHighlight(bool bHighlighted);

protected:

	virtual void BeginPlay() override;

private:

	void UpdateMeshes();

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Pillar;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Plate;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> Material;

	/** Height of the belt surface in meters */
	UPROPERTY(EditAnywhere, Category="Pole", meta=(ClampMin=1, ClampMax=7))
	int32 Height = 1;

	UPROPERTY()
	TWeakObjectPtr<AFactoryBelt> InputBelt;

	UPROPERTY()
	TWeakObjectPtr<AFactoryBelt> OutputBelt;
};
