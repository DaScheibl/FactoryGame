// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FactoryItemTypes.h"
#include "FactoryBuildingTypes.generated.h"

/*
 *  Gebäude: eine Zeile in DT_Buildings.csv beschreibt ein baubares Gebäude mit Größe, Ports und Baukosten.
 *  Feste Spalten wie bei den Rezepten (bis zu 4 Kostenposten), damit die Tabelle in Excel lesbar bleibt.
 *  Format und Werte: Docs/Standards/13-bauen.md
 */

/** What kind of thing a building table row places */
UENUM(BlueprintType)
enum class EFactoryBuildingType : uint8
{
	/** Production building with recipe and ports (AFactoryMachine) */
	Machine,

	/** Conveyor belt, drawn between ports and poles. Costs are per started 2 m */
	Belt,

	/** Conveyor pole (AFactoryBeltPole) */
	Pole,

	/** Mining drill: a machine that has to stand on a resource node (AFactoryResourceNode) */
	Drill
};

/** Where in a building a belt port sits */
struct FFactoryPort
{
	/** Position relative to the building origin (bottom center), at belt height */
	FVector Location = FVector::ZeroVector;

	/** Direction items flow through the port (out of the building for outputs, into it for inputs) */
	FVector Direction = FVector::ForwardVector;

	bool bInput = true;
};

/**
 *  One row of the building table (Content/FactoryGame/Core/Data/DT_Buildings.csv).
 *  The row name is the building id, e.g. BLD_Smelter.
 */
USTRUCT(BlueprintType)
struct FACTORYGAME_API FFactoryBuildingDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	FString NameEN;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	FString NameDE;

	/** Tier in which the building is unlocked (0-8) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building", meta=(ClampMin=0, ClampMax=8))
	int32 Tier = 0;

	/** Build menu category, e.g. Production */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	FName Category;

	/** Machine, belt, pole or drill */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	EFactoryBuildingType Type = EFactoryBuildingType::Machine;

	/** Footprint length along the item flow (front-back) in meters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building", meta=(ClampMin=1))
	int32 SizeX = 4;

	/** Footprint width (left-right) in meters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building", meta=(ClampMin=1))
	int32 SizeY = 4;

	/** Height in meters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building", meta=(ClampMin=1))
	int32 SizeZ = 4;

	/** Number of belt inputs (on the back side) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building", meta=(ClampMin=0, ClampMax=4))
	int32 Inputs = 1;

	/** Number of belt outputs (on the front side) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building", meta=(ClampMin=0, ClampMax=2))
	int32 Outputs = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	FName Cost1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	int32 Cost1Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	FName Cost2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	int32 Cost2Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	FName Cost3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	int32 Cost3Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	FName Cost4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building")
	int32 Cost4Amount = 0;

	/** Returns the build costs (only used columns) */
	TArray<FFactoryItemStack> GetCosts() const;

	/** Returns the footprint and height in cm */
	FVector GetSize() const { return FVector(SizeX, SizeY, SizeZ) * 100.0; }

	/**
	 *  Returns all belt ports, inputs first. Inputs sit on the back edge, outputs on the front edge,
	 *  centered on 1 m grid points at belt height (Docs/Standards/03-raster.md)
	 */
	TArray<FFactoryPort> GetPorts() const;

	/** Returns the display name in the current UI language */
	FText GetDisplayName() const;

	/** True for everything that is an AFactoryMachine (machines and drills) */
	bool IsMachine() const { return Type == EFactoryBuildingType::Machine || Type == EFactoryBuildingType::Drill; }

	/** Returns the build menu category: the Category column if it is one of GetMenuCategories(), Misc otherwise */
	FName GetMenuCategory() const;

	/** The fixed build menu categories in menu order: Production, Logistics, Power, Storage, Structure, Misc */
	static const TArray<FName>& GetMenuCategories();

	/** Returns the display name of a build menu category */
	static FText GetCategoryDisplayName(FName MenuCategory);
};
