// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FactoryBuildingTypes.h"
#include "FactoryBuildingDatabase.generated.h"

class UDataTable;

/**
 *  Read-only access to all buildable buildings.
 *  The buildings are loaded from a CSV file in Content/FactoryGame/Core/Data, like items and recipes.
 *
 *  Funktioniert wie UFactoryItemDatabase: ein Game Instance Subsystem, Zugriff mit
 *  UFactoryBuildingDatabase::Get(this). Nach dem Laden werden die Baukosten gegen die Item-Tabelle
 *  geprüft; Fehler erscheinen als Warnung im Output Log.
 *  Neues Gebäude: Zeile in DT_Buildings.csv ergänzen, im laufenden Spiel mit "ReloadBuildings" neu laden.
 */
UCLASS(config=Game)
class FACTORYGAME_API UFactoryBuildingDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/** Returns the building database for the given world context */
	static UFactoryBuildingDatabase* Get(const UObject* WorldContextObject);

	/** Loads the building table (after the item database, which is needed for validation) */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Returns the definition of the given building, or nullptr if it doesn't exist */
	const FFactoryBuildingDefinition* FindBuilding(FName BuildingId) const;

	/** Returns the ids of all buildings, sorted by tier and table order */
	const TArray<FName>& GetAllBuildingIds() const { return SortedBuildingIds; }

	/** Returns the display name of the given building, or the id if it doesn't exist */
	FText GetDisplayName(FName BuildingId) const;

	/** Reloads the CSV file and validates it. Returns the number of problems found */
	int32 Reload();

protected:

	/** Building table, relative to the project content directory */
	UPROPERTY(Config)
	FString BuildingTableFile = TEXT("FactoryGame/Core/Data/DT_Buildings.csv");

private:

	/** Checks all buildings for unknown cost items, sizes etc. and logs every problem */
	int32 Validate() const;

	UPROPERTY()
	TObjectPtr<UDataTable> BuildingTable;

	TArray<FName> SortedBuildingIds;
};
