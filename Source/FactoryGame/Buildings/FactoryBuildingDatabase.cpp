// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryBuildingDatabase.h"
#include "FactoryItemDatabase.h"
#include "FactoryGame.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UFactoryBuildingDatabase* UFactoryBuildingDatabase::Get(const UObject* WorldContextObject)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	return GameInstance ? GameInstance->GetSubsystem<UFactoryBuildingDatabase>() : nullptr;
}

void UFactoryBuildingDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	// die Item-Datenbank muss zuerst geladen sein, sonst schlägt die Prüfung der Baukosten fehl
	Collection.InitializeDependency<UFactoryItemDatabase>();

	Super::Initialize(Collection);

	Reload();
}

int32 UFactoryBuildingDatabase::Reload()
{
	// gleiches Vorgehen wie bei Items und Rezepten: DataTable im Speicher, Zeilenformat FFactoryBuildingDefinition
	BuildingTable = NewObject<UDataTable>(this, TEXT("DT_Buildings"));
	BuildingTable->RowStruct = FFactoryBuildingDefinition::StaticStruct();
	SortedBuildingIds.Reset();

	const FString FilePath = FPaths::ProjectContentDir() / BuildingTableFile;
	FString Csv;

	if (!FFileHelper::LoadFileToString(Csv, *FilePath))
	{
		UE_LOG(LogFactoryGame, Error, TEXT("Could not read the building table '%s'."), *FilePath);
		return 1;
	}

	int32 NumProblems = 0;

	for (const FString& Problem : BuildingTable->CreateTableFromCSVString(Csv))
	{
		UE_LOG(LogFactoryGame, Warning, TEXT("Building table: %s"), *Problem);
		++NumProblems;
	}

	// stable sort keeps the table order inside a tier
	SortedBuildingIds = BuildingTable->GetRowNames();
	SortedBuildingIds.StableSort([this](const FName& A, const FName& B)
	{
		return FindBuilding(A)->Tier < FindBuilding(B)->Tier;
	});

	NumProblems += Validate();

	UE_LOG(LogFactoryGame, Log, TEXT("Loaded %d buildings from '%s' (%d problems)."), SortedBuildingIds.Num(), *BuildingTableFile, NumProblems);
	return NumProblems;
}

int32 UFactoryBuildingDatabase::Validate() const
{
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);
	int32 NumProblems = 0;

	auto Report = [&NumProblems](FName BuildingId, const FString& Message)
	{
		UE_LOG(LogFactoryGame, Warning, TEXT("Building %s: %s"), *BuildingId.ToString(), *Message);
		++NumProblems;
	};

	for (const FName& BuildingId : SortedBuildingIds)
	{
		const FFactoryBuildingDefinition* Building = FindBuilding(BuildingId);

		// Gebäude heißen BLD_, Logistik-Teile (Bänder, Stützen) LOG_
		if (!BuildingId.ToString().StartsWith(TEXT("BLD_")) && !BuildingId.ToString().StartsWith(TEXT("LOG_")))
		{
			Report(BuildingId, TEXT("id should start with BLD_ or LOG_"));
		}

		if (Building->SizeX <= 0 || Building->SizeY <= 0 || Building->SizeZ <= 0)
		{
			Report(BuildingId, TEXT("size must be at least 1 m in every direction"));
		}

		// Ports müssen nebeneinander auf die Kante passen (mindestens 1 m pro Port)
		if (Building->Type == EFactoryBuildingType::Machine && (Building->Inputs > Building->SizeY || Building->Outputs > Building->SizeY))
		{
			Report(BuildingId, TEXT("more ports than meters of edge"));
		}

		const TPair<FName, int32> Columns[] = {
			{ Building->Cost1, Building->Cost1Amount }, { Building->Cost2, Building->Cost2Amount },
			{ Building->Cost3, Building->Cost3Amount }, { Building->Cost4, Building->Cost4Amount } };

		for (const TPair<FName, int32>& Column : Columns)
		{
			// eine Spalte mit ID, aber ohne Menge (oder umgekehrt) ist fast immer ein Tippfehler
			if (Column.Key.IsNone() != (Column.Value <= 0))
			{
				Report(BuildingId, FString::Printf(TEXT("cost item '%s' with amount %d"), *Column.Key.ToString(), Column.Value));
			}

			if (!Column.Key.IsNone() && Items && Items->GetMaxStackSize(Column.Key) <= 0)
			{
				Report(BuildingId, FString::Printf(TEXT("cost item '%s' doesn't exist or can't be carried"), *Column.Key.ToString()));
			}
		}
	}

	return NumProblems;
}

const FFactoryBuildingDefinition* UFactoryBuildingDatabase::FindBuilding(FName BuildingId) const
{
	return BuildingTable && !BuildingId.IsNone() ? BuildingTable->FindRow<FFactoryBuildingDefinition>(BuildingId, TEXT("FindBuilding"), false) : nullptr;
}

FText UFactoryBuildingDatabase::GetDisplayName(FName BuildingId) const
{
	const FFactoryBuildingDefinition* Building = FindBuilding(BuildingId);
	return Building ? Building->GetDisplayName() : FText::FromName(BuildingId);
}
