// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "FactoryPlant.generated.h"

class UStaticMeshComponent;

/** Kind of plant that can be gathered */
UENUM(BlueprintType)
enum class EFactoryPlantType : uint8
{
	/** Quick to gather, gives leaves */
	Bush,

	/** Takes longer, gives wood */
	Tree
};

/** A gathered plant in the world save: which spot and how long until it grows back */
USTRUCT()
struct FFactorySavedPlant
{
	GENERATED_BODY()

	/** Index of the plant spot in UFactoryPlantSubsystem */
	UPROPERTY(SaveGame)
	int32 Spot = INDEX_NONE;

	UPROPERTY(SaveGame)
	float RegrowSeconds = 0.0f;
};

/**
 *  A bush or tree the player gathers by holding the interact key. Afterwards only a stump is left,
 *  the plant grows back after RegrowTime seconds of play time.
 *
 *  Graybox: Busch = grüne Kugel, Baum = brauner Stamm mit grüner Krone, gesammelt = flacher Stumpf ohne Kollision.
 *  Pflanzen stehen fest in der Welt; gespeichert wird nur, welche gerade nachwachsen.
 *  Regeln: Docs/Standards/17-strom.md
 */
UCLASS()
class FACTORYGAME_API AFactoryPlant : public AActor
{
	GENERATED_BODY()

public:

	/** Play time until a gathered plant grows back, in seconds */
	static constexpr float RegrowTime = 300.0f;

	AFactoryPlant();

	/** Spawns a plant on the ground at the location */
	static AFactoryPlant* SpawnPlant(UWorld* World, const FVector& Location, EFactoryPlantType Type, int32 Spot);

	EFactoryPlantType GetPlantType() const { return PlantType; }

	int32 GetSpot() const { return Spot; }

	/** What gathering gives: leaves or wood */
	FName GetItemId() const;

	int32 GetHarvestAmount() const;

	/** How long the interact key has to be held, in seconds */
	float GetHarvestTime() const;

	FText GetDisplayName() const;

	bool IsHarvested() const { return RegrowRemaining > 0.0f; }

	float GetRegrowRemaining() const { return RegrowRemaining; }

	/** Removes the plant until it grows back */
	void Harvest();

	/** Sets the remaining regrow time (0 = grown), e.g. after loading */
	void SetRegrowRemaining(float Seconds);

	virtual void Tick(float DeltaSeconds) override;

private:

	void UpdateVisual();

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USceneComponent> Root;

	/** Bush body or tree crown */
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Crown;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Trunk;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Stump;

	EFactoryPlantType PlantType = EFactoryPlantType::Bush;

	int32 Spot = INDEX_NONE;

	float RegrowRemaining = 0.0f;
};

/**
 *  Places the plants of the flat test world (Lvl_Flat) and keeps their gathered state for the world save.
 *  Feste Liste rund um den Startpunkt wie bei den Erzvorkommen, bis die echte Welt kommt.
 */
UCLASS()
class FACTORYGAME_API UFactoryPlantSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	static UFactoryPlantSubsystem* Get(const UObject* WorldContextObject);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Gathered plants with their remaining regrow time */
	TArray<FFactorySavedPlant> GetSaveData() const;

	/** Applies a loaded state. Works before and after the plants are placed */
	void LoadSaveData(const TArray<FFactorySavedPlant>& Data);

private:

	void ApplySaveData();

	UPROPERTY()
	TArray<TObjectPtr<AFactoryPlant>> Plants;

	TArray<FFactorySavedPlant> PendingData;

	bool bHasPendingData = false;
};
