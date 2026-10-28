// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryBelt.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;
class UInstancedStaticMeshComponent;
class UTextRenderComponent;
class UFactoryBeltComponent;
class UFactoryProductionComponent;

/**
 *  Graybox conveyor belt: a straight dark strip with the items shown as small colored cubes.
 *
 *  Die Logik steckt komplett in der UFactoryBeltComponent (Simulations-Takt). Der Actor ist nur die Optik:
 *  jedes Bild holt er die Item-Positionen und schiebt die Würfel dorthin. Pro Item-Sorte gibt es eine
 *  Instanced-Mesh-Komponente (Farbe = PlaceholderColor aus DT_Items), damit hunderte Items nur wenige Draw Calls kosten.
 *  Der Actor-Ursprung ist der Bandanfang am Boden, das Band zeigt entlang der X-Achse, die Oberkante liegt auf 1 m.
 *  Zum Testen: Konsole "ConnectBelt", siehe Docs/Standards/12-simulation.md
 */
UCLASS()
class FACTORYGAME_API AFactoryBelt : public AActor
{
	GENERATED_BODY()

public:

	/** Height of the belt surface above the actor origin in cm (port height, Docs/Standards/03-raster.md) */
	static constexpr double SurfaceHeight = 100.0;

	AFactoryBelt();

	UFactoryBeltComponent* GetBelt() const { return Belt; }

	/**
	 *  Places the belt between two points (both at floor height) and sets its length. Items that don't fit anymore
	 *  are added to OutRemoved
	 */
	void SetEndpoints(const FVector& Start, const FVector& End, TArray<FName>& OutRemoved);

	/** Spawns a belt from the output of one machine to the input of another and connects both */
	static AFactoryBelt* SpawnBetween(UWorld* World, AActor* SourceMachine, AActor* TargetMachine, int32 Tier);

	virtual void Tick(float DeltaSeconds) override;

protected:

	virtual void BeginPlay() override;

private:

	/** Moves the item cubes to the current item positions */
	void UpdateItemMeshes();

	/** Returns the cube component for an item type, creates it on first use */
	UInstancedStaticMeshComponent* GetItemMesh(FName ItemId);

	/** Updates the text above the belt */
	void UpdateLabel();

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UTextRenderComponent> Label;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UFactoryBeltComponent> Belt;

	/** Engine cube, used for the belt body and the item cubes */
	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	/** Engine material with a "Color" parameter */
	UPROPERTY()
	TObjectPtr<UMaterialInterface> ColorMaterial;

	/** One instanced cube component per item type on this belt */
	UPROPERTY()
	TMap<FName, TObjectPtr<UInstancedStaticMeshComponent>> ItemMeshes;

	/** Text currently shown by the label */
	FString LabelText;

	/** Reused every frame so updating the cubes doesn't allocate */
	TArray<float> PositionsScratch;
	TMap<FName, TArray<FTransform>> TransformsScratch;
};
