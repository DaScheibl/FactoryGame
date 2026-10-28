// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryItemTypes.h"
#include "FactoryBeltPath.h"
#include "FactoryBelt.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UInstancedStaticMeshComponent;
class USplineComponent;
class USplineMeshComponent;
class UBoxComponent;
class UTextRenderComponent;
class UFactoryBeltComponent;
class AFactoryMachine;
class AFactoryBeltPole;

/**
 *  Graybox conveyor belt: a dark strip along a curve with the items shown as small colored cubes.
 *
 *  Die Logik steckt komplett in der UFactoryBeltComponent (Simulations-Takt). Der Actor ist die Optik
 *  und kennt die Anschlüsse:
 *  - Kurve: ein Spline (FFactoryBeltPath) auf Bandhöhe, der Körper besteht aus 1-m-Spline-Mesh-Stücken
 *  - Items: jedes Bild werden die Item-Positionen auf die Kurve gelegt (Instanced Mesh pro Sorte)
 *  - Anfang/Ende hängen an einem Maschinen-Port oder an einer Bandstütze. Zwei Bänder an derselben Stütze
 *    werden verknüpft: das ankommende gibt seine Items an das abgehende weiter.
 *  Gebaut wird über das Bauwerkzeug (UFactoryBuildComponent) oder per Konsole ("ConnectBelt", "BeltTest").
 */
UCLASS()
class FACTORYGAME_API AFactoryBelt : public AActor
{
	GENERATED_BODY()

public:

	AFactoryBelt();

	UFactoryBeltComponent* GetBelt() const { return Belt; }

	/** Sets the curve (belt surface, world space) and the length. Items that don't fit anymore are added to OutRemoved */
	void SetPath(const FFactoryBeltPath& NewPath, TArray<FName>& OutRemoved);

	const FFactoryBeltPath& GetPath() const { return Path; }

	/** Connects the belt start to an output port of a machine (port index as in AFactoryMachine::GetWorldPorts) */
	void ConnectStartToMachine(AFactoryMachine* Machine, int32 PortIndex);

	/** Connects the belt end to an input port of a machine */
	void ConnectEndToMachine(AFactoryMachine* Machine, int32 PortIndex);

	/** Starts the belt at a pole; a belt already ending there now feeds this one */
	void ConnectStartToPole(AFactoryBeltPole* Pole);

	/** Ends the belt at a pole; a belt already starting there gets the items of this one */
	void ConnectEndToPole(AFactoryBeltPole* Pole);

	AFactoryMachine* GetStartMachine() const { return StartMachine.Get(); }
	AFactoryMachine* GetEndMachine() const { return EndMachine.Get(); }
	int32 GetStartPort() const { return StartPort; }
	int32 GetEndPort() const { return EndPort; }
	AFactoryBeltPole* GetStartPole() const { return StartPole.Get(); }
	AFactoryBeltPole* GetEndPole() const { return EndPole.Get(); }

	/** Spawns a belt from the first free output of one machine to the nearest free input of another (console commands) */
	static AFactoryBelt* SpawnBetween(UWorld* World, AFactoryMachine* SourceMachine, AFactoryMachine* TargetMachine, int32 Tier);

	/** Spawns an unconnected belt along a path */
	static AFactoryBelt* SpawnBelt(UWorld* World, const FFactoryBeltPath& Path, int32 Tier);

	/** Returns the build costs: the costs of LOG_Belt_Mk<Tier> per started 2 m (Docs/Standards/13-bauen.md) */
	TArray<FFactoryItemStack> GetBuildCosts() const;

	/** Returns the build costs of a belt with the given length in cm */
	static TArray<FFactoryItemStack> GetBuildCostsForLength(const UObject* WorldContext, double LengthCm, int32 Tier);

	/** Returns the building table id of a belt tier, e.g. LOG_Belt_Mk1 */
	static FName GetBuildingIdForTier(int32 Tier);

	/** Tints the belt red while it is targeted for demolition */
	void SetDemolishHighlight(bool bHighlighted);

	/** Puts a path into a spline component, relative to Origin */
	static void ApplyPathToSpline(USplineComponent* Spline, const FFactoryBeltPath& Path, const FVector& Origin);

	/**
	 *  Rebuilds the 1 m body pieces along a spline (also used by the build hologram).
	 *  With Boxes, every piece also gets a collision box (spline meshes don't bend box collision)
	 */
	static void BuildBodyMeshes(AActor* Owner, USplineComponent* Spline, TArray<TObjectPtr<USplineMeshComponent>>& Meshes,
		UStaticMesh* Mesh, UMaterialInterface* Material, TArray<TObjectPtr<UBoxComponent>>* Boxes);

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
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UTextRenderComponent> Label;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UFactoryBeltComponent> Belt;

	/** Body pieces along the spline */
	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> BodyMeshes;

	/** One collision box per body piece, for traces and the build overlap check */
	UPROPERTY()
	TArray<TObjectPtr<UBoxComponent>> CollisionBoxes;

	/** Engine cube, used for the belt body and the item cubes */
	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	/** Engine material with a "Color" parameter */
	UPROPERTY()
	TObjectPtr<UMaterialInterface> ColorMaterial;

	/** Material instance of the belt body, to change its color */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;

	/** One instanced cube component per item type on this belt */
	UPROPERTY()
	TMap<FName, TObjectPtr<UInstancedStaticMeshComponent>> ItemMeshes;

	FFactoryBeltPath Path;

	UPROPERTY()
	TWeakObjectPtr<AFactoryMachine> StartMachine;

	UPROPERTY()
	TWeakObjectPtr<AFactoryMachine> EndMachine;

	int32 StartPort = INDEX_NONE;
	int32 EndPort = INDEX_NONE;

	UPROPERTY()
	TWeakObjectPtr<AFactoryBeltPole> StartPole;

	UPROPERTY()
	TWeakObjectPtr<AFactoryBeltPole> EndPole;

	/** Text currently shown by the label */
	FString LabelText;

	/** Reused every frame so updating the cubes doesn't allocate */
	TArray<float> PositionsScratch;
	TMap<FName, TArray<FTransform>> TransformsScratch;
};
