// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryProductionComponent.h"
#include "FactoryMachine.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class UMaterialInstanceDynamic;

/**
 *  Base actor for production buildings. For now a graybox: a cube that changes its color with the machine state and shows its status as text.
 *
 *  Die Logik steckt komplett in der UFactoryProductionComponent (Simulations-Takt).
 *  Der Actor ist nur die Optik: er hört auf OnStateChanged, färbt den Würfel ein und
 *  aktualisiert ein paar Mal pro Sekunde den Text darüber. Farben (Graybox, nicht final):
 *  grau = kein Rezept, gelb = wartet auf Zutaten, grün = arbeitet, rot = Ausgang voll.
 *  Zum Testen: Konsole "SpawnMachine Smelter IronIngot", siehe Docs/Standards/12-simulation.md
 */
UCLASS()
class FACTORYGAME_API AFactoryMachine : public AActor
{
	GENERATED_BODY()

public:

	AFactoryMachine();

	UFactoryProductionComponent* GetProduction() const { return Production; }

	/** Sets size of the graybox cube in meters. The actor origin is the center of the bottom face */
	void SetGrayboxSize(const FVector& SizeInMeters);

	virtual void Tick(float DeltaSeconds) override;

protected:

	virtual void BeginPlay() override;

	/** Size of the graybox cube in meters */
	UPROPERTY(EditAnywhere, Category="Graybox")
	FVector GrayboxSize = FVector(4.0, 4.0, 4.0);

private:

	void HandleStateChanged(EFactoryMachineState NewState);

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

	/** Material instance of the cube, to change its color */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> Material;
};
