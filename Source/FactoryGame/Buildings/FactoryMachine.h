// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryProductionComponent.h"
#include "FactoryBuildingTypes.h"
#include "FactoryMachine.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class UMaterialInstanceDynamic;
class AFactoryBelt;

/**
 *  Base actor for production buildings. For now a graybox: a cube that changes its color with the machine state and shows its status as text.
 *
 *  Die Logik steckt komplett in der UFactoryProductionComponent (Simulations-Takt).
 *  Der Actor ist nur die Optik: er hört auf OnStateChanged, färbt den Würfel ein und
 *  aktualisiert jedes Bild den Text darüber. Farben (Graybox, nicht final):
 *  grau = kein Rezept, gelb = wartet auf Zutaten, grün = arbeitet, rot = Ausgang voll.
 *  Größe und Ports kommen aus DT_Buildings (InitBuilding). Ports sind kleine Platten an der Kante:
 *  blau = Eingang (hinten), orange = Ausgang (vorn).
 *  Zum Testen: Konsole "SpawnMachine Smelter IronIngot", siehe Docs/Standards/12-simulation.md
 */
UCLASS()
class FACTORYGAME_API AFactoryMachine : public AActor
{
	GENERATED_BODY()

public:

	AFactoryMachine();

	UFactoryProductionComponent* GetProduction() const { return Production; }

	/** Sets building id, size and ports from the building table. Unknown buildings keep the default 4 x 4 x 4 m cube */
	void InitBuilding(FName BuildingId);

	/** Sets size of the graybox cube in meters. The actor origin is the center of the bottom face */
	void SetGrayboxSize(const FVector& SizeInMeters);

	/** Returns the belt ports in world space, inputs first */
	TArray<FFactoryPort> GetWorldPorts() const;

	/** Returns the number of input ports; output port N has the index NumInputs + N */
	int32 GetNumInputPorts() const;

	/** Returns the belt connected to a port, or null if the port is free */
	AFactoryBelt* GetPortBelt(int32 PortIndex) const;

	/** Remembers which belt is connected to a port */
	void SetPortBelt(int32 PortIndex, AFactoryBelt* Belt);

	/** Returns the free input or output port closest to a location within MaxDistance (cm), or INDEX_NONE */
	int32 FindFreePort(bool bInput, const FVector& WorldLocation, double MaxDistance) const;

	/** Tints the machine red while it is targeted for demolition */
	void SetDemolishHighlight(bool bHighlighted);

	virtual void Tick(float DeltaSeconds) override;

protected:

	virtual void BeginPlay() override;

	/** Size of the graybox cube in meters */
	UPROPERTY(EditAnywhere, Category="Graybox")
	FVector GrayboxSize = FVector(4.0, 4.0, 4.0);

private:

	void HandleStateChanged(EFactoryMachineState NewState);

	/** Sets the cube color from the state and the demolish highlight */
	void UpdateColor();

	/** Creates the port plates from the building table */
	void CreatePortMarkers();

	/** Updates the status text above the cube */
	void UpdateLabel();

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UTextRenderComponent> Label;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UFactoryProductionComponent> Production;

	/** Port plates, created by InitBuilding */
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PortMarkers;

	/** Belt ports relative to the actor, from the building table */
	TArray<FFactoryPort> Ports;

	/** Belt connected to each port (same order as Ports) */
	TArray<TWeakObjectPtr<AFactoryBelt>> PortBelts;

	/** Text currently shown by the label */
	FString LabelText;

	bool bDemolishHighlight = false;

	/** Material instance of the cube, to change its color */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> Material;
};
