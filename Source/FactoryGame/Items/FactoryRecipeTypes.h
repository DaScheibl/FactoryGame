// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FactoryItemTypes.h"
#include "FactoryRecipeTypes.generated.h"

/*
 *  Rezepte: eine Zeile in DT_Recipes.csv beschreibt einen Arbeitsschritt einer Maschine,
 *  z. B. "3 Eisenbarren -> 2 Eisenplatten in 6 s im Konstruktor".
 *  Feste Spalten statt Listen, damit die Tabelle in Excel/LibreOffice lesbar bleibt:
 *  bis zu 4 Eingänge (Fertigungsanlage) und 2 Ausgänge (Raffinerie: Produkt + Nebenprodukt).
 *  Format und Werte: Docs/Standards/11-rezepte.md
 */

/**
 *  One row of the recipe table (Content/FactoryGame/Core/Data/DT_Recipes.csv).
 *  The row name is the recipe id, e.g. RCP_IronPlate.
 */
USTRUCT(BlueprintType)
struct FACTORYGAME_API FFactoryRecipeDefinition : public FTableRowBase
{
	GENERATED_BODY()

	/** Display name. Empty: the name of the first output is used (alternate recipes need their own name) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FString NameEN;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FString NameDE;

	/** Tier in which the recipe is unlocked (0-8) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe", meta=(ClampMin=0, ClampMax=8))
	int32 Tier = 0;

	/** Building that runs the recipe, e.g. BLD_Constructor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FName Building;

	/** Can also be crafted by hand at the workbench (BLD_Workbench) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	bool Workbench = false;

	/** Seconds per cycle at 100 % speed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe", meta=(ClampMin=0.1))
	float Duration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FName Input1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	int32 Input1Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FName Input2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	int32 Input2Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FName Input3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	int32 Input3Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FName Input4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	int32 Input4Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FName Output1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	int32 Output1Amount = 0;

	/** Byproduct, e.g. heavy oil residue from the refinery */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	FName Output2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Recipe")
	int32 Output2Amount = 0;

	/** Returns the used input columns as stacks (empty columns are skipped) */
	TArray<FFactoryItemStack> GetInputs() const;

	/** Returns the used output columns as stacks (empty columns are skipped) */
	TArray<FFactoryItemStack> GetOutputs() const;

	/** Returns how many of the given amount per cycle are moved per minute at 100 % speed */
	float GetPerMinute(int32 AmountPerCycle) const;
};
