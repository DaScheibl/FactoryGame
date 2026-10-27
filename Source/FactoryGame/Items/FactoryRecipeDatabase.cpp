// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryRecipeDatabase.h"
#include "FactoryItemDatabase.h"
#include "FactoryLocalization.h"
#include "FactoryGame.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

const FName UFactoryRecipeDatabase::WorkbenchId(TEXT("BLD_Workbench"));

UFactoryRecipeDatabase* UFactoryRecipeDatabase::Get(const UObject* WorldContextObject)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	return GameInstance ? GameInstance->GetSubsystem<UFactoryRecipeDatabase>() : nullptr;
}

void UFactoryRecipeDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	// die Item-Datenbank muss zuerst geladen sein, sonst schlägt die Prüfung jedes Rezept fehl
	Collection.InitializeDependency<UFactoryItemDatabase>();

	Super::Initialize(Collection);

	Reload();
}

int32 UFactoryRecipeDatabase::Reload()
{
	// gleiches Vorgehen wie bei den Items: DataTable im Speicher, Zeilenformat FFactoryRecipeDefinition
	RecipeTable = NewObject<UDataTable>(this, TEXT("DT_Recipes"));
	RecipeTable->RowStruct = FFactoryRecipeDefinition::StaticStruct();
	SortedRecipeIds.Reset();

	const FString FilePath = FPaths::ProjectContentDir() / RecipeTableFile;
	FString Csv;

	if (!FFileHelper::LoadFileToString(Csv, *FilePath))
	{
		UE_LOG(LogFactoryGame, Error, TEXT("Could not read the recipe table '%s'."), *FilePath);
		return 1;
	}

	int32 NumProblems = 0;

	for (const FString& Problem : RecipeTable->CreateTableFromCSVString(Csv))
	{
		UE_LOG(LogFactoryGame, Warning, TEXT("Recipe table: %s"), *Problem);
		++NumProblems;
	}

	// stable sort keeps the table order inside a tier
	SortedRecipeIds = RecipeTable->GetRowNames();
	SortedRecipeIds.StableSort([this](const FName& A, const FName& B)
	{
		return FindRecipe(A)->Tier < FindRecipe(B)->Tier;
	});

	NumProblems += Validate();

	UE_LOG(LogFactoryGame, Log, TEXT("Loaded %d recipes from '%s' (%d problems)."), SortedRecipeIds.Num(), *RecipeTableFile, NumProblems);
	return NumProblems;
}

int32 UFactoryRecipeDatabase::Validate() const
{
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);

	if (!Items)
	{
		return 0;
	}

	int32 NumProblems = 0;

	auto Report = [&NumProblems](FName RecipeId, const FString& Message)
	{
		UE_LOG(LogFactoryGame, Warning, TEXT("Recipe %s: %s"), *RecipeId.ToString(), *Message);
		++NumProblems;
	};

	for (const FName& RecipeId : SortedRecipeIds)
	{
		const FFactoryRecipeDefinition* Recipe = FindRecipe(RecipeId);

		if (!RecipeId.ToString().StartsWith(TEXT("RCP_")))
		{
			Report(RecipeId, TEXT("id should start with RCP_"));
		}

		if (!Recipe->Building.ToString().StartsWith(TEXT("BLD_")))
		{
			Report(RecipeId, FString::Printf(TEXT("building '%s' should start with BLD_"), *Recipe->Building.ToString()));
		}

		if (Recipe->Duration <= 0.0f)
		{
			Report(RecipeId, TEXT("duration must be greater than 0"));
		}

		// eine Spalte mit ID, aber ohne Menge (oder umgekehrt) ist fast immer ein Tippfehler
		const TPair<FName, int32> Columns[] = {
			{ Recipe->Input1, Recipe->Input1Amount }, { Recipe->Input2, Recipe->Input2Amount },
			{ Recipe->Input3, Recipe->Input3Amount }, { Recipe->Input4, Recipe->Input4Amount },
			{ Recipe->Output1, Recipe->Output1Amount }, { Recipe->Output2, Recipe->Output2Amount } };

		for (const TPair<FName, int32>& Column : Columns)
		{
			if (Column.Key.IsNone() != (Column.Value <= 0))
			{
				Report(RecipeId, FString::Printf(TEXT("item '%s' with amount %d"), *Column.Key.ToString(), Column.Value));
			}
		}

		const TArray<FFactoryItemStack> Inputs = Recipe->GetInputs();
		const TArray<FFactoryItemStack> Outputs = Recipe->GetOutputs();

		if (Outputs.IsEmpty())
		{
			Report(RecipeId, TEXT("has no output"));
		}

		TSet<FName> SeenInputs;

		for (const FFactoryItemStack& Input : Inputs)
		{
			bool bDuplicate = false;
			SeenInputs.Add(Input.ItemId, &bDuplicate);

			if (bDuplicate)
			{
				Report(RecipeId, FString::Printf(TEXT("input '%s' is listed twice"), *Input.ItemId.ToString()));
			}
		}

		for (const TArray<FFactoryItemStack>* Stacks : { &Inputs, &Outputs })
		{
			for (const FFactoryItemStack& Stack : *Stacks)
			{
				const FFactoryItemDefinition* Item = Items->FindItem(Stack.ItemId);

				if (!Item)
				{
					Report(RecipeId, FString::Printf(TEXT("unknown item '%s'"), *Stack.ItemId.ToString()));
				}
				else if (Recipe->Workbench && Item->Form == EFactoryItemForm::Fluid)
				{
					// an der Werkbank wird aus dem Inventar gebaut, und Fluide passen nicht ins Inventar
					Report(RecipeId, FString::Printf(TEXT("fluid '%s' can't be crafted at the workbench"), *Stack.ItemId.ToString()));
				}
				else if (Item->Tier > Recipe->Tier && Stacks == &Inputs)
				{
					Report(RecipeId, FString::Printf(TEXT("input '%s' is tier %d, recipe is tier %d"), *Stack.ItemId.ToString(), Item->Tier, Recipe->Tier));
				}
			}
		}
	}

	return NumProblems;
}

const FFactoryRecipeDefinition* UFactoryRecipeDatabase::FindRecipe(FName RecipeId) const
{
	return RecipeTable && !RecipeId.IsNone() ? RecipeTable->FindRow<FFactoryRecipeDefinition>(RecipeId, TEXT("FindRecipe"), false) : nullptr;
}

TArray<FName> UFactoryRecipeDatabase::GetRecipesForBuilding(FName BuildingId) const
{
	const bool bWorkbench = BuildingId == WorkbenchId;

	return SortedRecipeIds.FilterByPredicate([this, BuildingId, bWorkbench](const FName& RecipeId)
	{
		const FFactoryRecipeDefinition* Recipe = FindRecipe(RecipeId);
		return Recipe->Building == BuildingId || (bWorkbench && Recipe->Workbench);
	});
}

TArray<FName> UFactoryRecipeDatabase::GetRecipesProducing(FName ItemId) const
{
	return SortedRecipeIds.FilterByPredicate([this, ItemId](const FName& RecipeId)
	{
		return FindRecipe(RecipeId)->GetOutputs().ContainsByPredicate([ItemId](const FFactoryItemStack& Stack) { return Stack.ItemId == ItemId; });
	});
}

TArray<FName> UFactoryRecipeDatabase::GetRecipesUsing(FName ItemId) const
{
	return SortedRecipeIds.FilterByPredicate([this, ItemId](const FName& RecipeId)
	{
		return FindRecipe(RecipeId)->GetInputs().ContainsByPredicate([ItemId](const FFactoryItemStack& Stack) { return Stack.ItemId == ItemId; });
	});
}

FText UFactoryRecipeDatabase::GetDisplayName(FName RecipeId) const
{
	const FFactoryRecipeDefinition* Recipe = FindRecipe(RecipeId);

	if (!Recipe)
	{
		return FText::FromName(RecipeId);
	}

	// eigener Name nur für Alternativrezepte nötig; sonst heißt das Rezept wie sein Produkt
	const FString& OwnName = FactoryLoc::IsGerman() && !Recipe->NameDE.IsEmpty() ? Recipe->NameDE : Recipe->NameEN;

	if (!OwnName.IsEmpty())
	{
		return FText::FromString(OwnName);
	}

	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);
	return Items ? Items->GetDisplayName(Recipe->Output1) : FText::FromName(Recipe->Output1);
}
