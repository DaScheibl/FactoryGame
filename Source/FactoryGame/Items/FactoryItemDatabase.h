// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FactoryItemTypes.h"
#include "FactoryItemDatabase.generated.h"

class UDataTable;

/**
 *  Read-only access to all item definitions.
 *  The items are loaded from a CSV file in Content/FactoryGame/Core/Data, so they can be edited in a spreadsheet and diffed in git.
 *
 *  Als Game Instance Subsystem existiert die Datenbank genau einmal, wird beim Spielstart
 *  automatisch erzeugt und überlebt Levelwechsel. Zugriff von überall mit UFactoryItemDatabase::Get(this).
 *  Die CSV wird zur Laufzeit gelesen und nicht als .uasset importiert; DefaultGame.ini packt
 *  den Ordner deshalb mit ins fertige Spiel.
 *  Neues Item: Zeile in DT_Items.csv ergänzen, im laufenden Spiel mit "ReloadItems" neu laden.
 */
UCLASS(config=Game)
class FACTORYGAME_API UFactoryItemDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/** Returns the item database for the given world context */
	static UFactoryItemDatabase* Get(const UObject* WorldContextObject);

	/** Loads the item table */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Returns the definition of the given item, or nullptr if it doesn't exist */
	const FFactoryItemDefinition* FindItem(FName ItemId) const;

	/** Returns the ids of all items, sorted by tier and table order */
	const TArray<FName>& GetAllItemIds() const { return SortedItemIds; }

	/** Returns the maximum stack size of the given item, or 0 if it doesn't exist or can't be carried */
	int32 GetMaxStackSize(FName ItemId) const;

	/** Returns the display name of the given item, or the id if it doesn't exist */
	FText GetDisplayName(FName ItemId) const;

	/** Reloads the CSV file, e.g. after editing it while the editor is running */
	void Reload();

protected:

	/** Item table, relative to the project content directory */
	UPROPERTY(Config)
	FString ItemTableFile = TEXT("FactoryGame/Core/Data/DT_Items.csv");

private:

	UPROPERTY()
	TObjectPtr<UDataTable> ItemTable;

	TArray<FName> SortedItemIds;
};
