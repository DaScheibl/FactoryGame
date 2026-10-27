// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FactoryRecipeTypes.h"
#include "FactoryRecipeDatabase.generated.h"

class UDataTable;

/**
 *  Read-only access to all recipes.
 *  The recipes are loaded from a CSV file in Content/FactoryGame/Core/Data, like the items.
 *
 *  Funktioniert wie UFactoryItemDatabase: ein Game Instance Subsystem, Zugriff mit
 *  UFactoryRecipeDatabase::Get(this). Nach dem Laden wird jedes Rezept gegen die Item-Tabelle
 *  geprüft (unbekannte IDs, Mengen, Dauer); Fehler erscheinen als Warnung im Output Log.
 *  Neues Rezept: Zeile in DT_Recipes.csv ergänzen, im laufenden Spiel mit "ReloadRecipes" neu laden.
 */
UCLASS(config=Game)
class FACTORYGAME_API UFactoryRecipeDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/** Returns the recipe database for the given world context */
	static UFactoryRecipeDatabase* Get(const UObject* WorldContextObject);

	/** Loads the recipe table (after the item database, which is needed for validation) */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Returns the definition of the given recipe, or nullptr if it doesn't exist */
	const FFactoryRecipeDefinition* FindRecipe(FName RecipeId) const;

	/** Returns the ids of all recipes, sorted by tier and table order */
	const TArray<FName>& GetAllRecipeIds() const { return SortedRecipeIds; }

	/** Returns the ids of all recipes the given building can run. BLD_Workbench also gets every recipe marked as Workbench */
	TArray<FName> GetRecipesForBuilding(FName BuildingId) const;

	/** Returns the ids of all recipes that produce the given item */
	TArray<FName> GetRecipesProducing(FName ItemId) const;

	/** Returns the ids of all recipes that consume the given item */
	TArray<FName> GetRecipesUsing(FName ItemId) const;

	/** Returns the display name of the given recipe: its own name, else the name of its first output */
	FText GetDisplayName(FName RecipeId) const;

	/** Reloads the CSV file and validates it against the item table. Returns the number of problems found */
	int32 Reload();

	/** Building id of the workbench, which can craft all recipes marked as Workbench */
	static const FName WorkbenchId;

protected:

	/** Recipe table, relative to the project content directory */
	UPROPERTY(Config)
	FString RecipeTableFile = TEXT("FactoryGame/Core/Data/DT_Recipes.csv");

private:

	/** Checks all recipes for unknown items, missing amounts etc. and logs every problem */
	int32 Validate() const;

	UPROPERTY()
	TObjectPtr<UDataTable> RecipeTable;

	TArray<FName> SortedRecipeIds;
};
