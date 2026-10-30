// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "FactoryResourceNode.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;

/** How rich a resource node is. Sets the speed of a drill on it */
UENUM(BlueprintType)
enum class EFactoryNodePurity : uint8
{
	/** 50 % speed: 30 ore per minute with a Mk1 drill */
	Impure,

	/** 100 % speed: 60 ore per minute */
	Normal,

	/** 200 % speed: 120 ore per minute */
	Pure
};

/**
 *  A resource node in the world: an endless deposit of one ore with a purity.
 *  A drill (building type Drill) snaps onto it and mines with the speed of the purity,
 *  the player can mine it by hand by holding the interact key (1 ore per second).
 *
 *  Graybox: flache Scheibe (3 m Durchmesser) in der Erzfarbe, darüber Name und Reinheit.
 *  Vorkommen sind fest in der Welt und werden nicht gespeichert; in der flachen Testwelt setzt sie
 *  UFactoryResourceNodeSubsystem beim Start. Regeln: Docs/Standards/15-abbau.md
 */
UCLASS()
class FACTORYGAME_API AFactoryResourceNode : public AActor
{
	GENERATED_BODY()

public:

	/** Radius of the node in cm. A drill aimed within this distance snaps onto the node */
	static constexpr double Radius = 150.0;

	AFactoryResourceNode();

	/** Spawns a node on the ground at the location */
	static AFactoryResourceNode* SpawnNode(UWorld* World, const FVector& Location, FName ItemId, EFactoryNodePurity Purity);

	/** Returns the node closest to the location within MaxDistance (cm, horizontal), or nullptr */
	static AFactoryResourceNode* FindNodeNear(const UWorld* World, const FVector& Location, double MaxDistance);

	/** Speed of a drill on a node of this purity, in percent */
	static int32 GetSpeedPercent(EFactoryNodePurity Purity);

	static FText GetPurityText(EFactoryNodePurity Purity);

	FName GetItemId() const { return ItemId; }

	EFactoryNodePurity GetPurity() const { return Purity; }

	/** Recipe a drill runs on this node: RCP_Mine_ plus the item name, e.g. RCP_Mine_IronOre */
	FName GetMiningRecipe() const;

	/** Returns the drill standing on this node, or nullptr */
	class AFactoryMachine* FindDrill() const;

	virtual void Tick(float DeltaSeconds) override;

private:

	void Setup(FName InItemId, EFactoryNodePurity InPurity);

	UPROPERTY(VisibleAnywhere, Category="Node")
	FName ItemId;

	UPROPERTY(VisibleAnywhere, Category="Node")
	EFactoryNodePurity Purity = EFactoryNodePurity::Normal;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UTextRenderComponent> Label;
};

/**
 *  Places the resource nodes of the flat test world (Lvl_Flat) when it starts.
 *  Feste Liste rund um den Startpunkt, bis die echte Welt kommt (WLD_Decision). Andere Maps bleiben leer.
 */
UCLASS()
class FACTORYGAME_API UFactoryResourceNodeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};
