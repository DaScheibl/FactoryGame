// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryItemDatabase.h"
#include "FactoryGame.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UFactoryItemDatabase* UFactoryItemDatabase::Get(const UObject* WorldContextObject)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	return GameInstance ? GameInstance->GetSubsystem<UFactoryItemDatabase>() : nullptr;
}

void UFactoryItemDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Reload();
}

void UFactoryItemDatabase::Reload()
{
	// Leere DataTable im Speicher anlegen, Zeilenformat ist FFactoryItemDefinition.
	// Die erste CSV-Spalte ist der Zeilenname (= Item-ID), die übrigen heißen wie die Properties.
	ItemTable = NewObject<UDataTable>(this, TEXT("DT_Items"));
	ItemTable->RowStruct = FFactoryItemDefinition::StaticStruct();
	SortedItemIds.Reset();

	const FString FilePath = FPaths::ProjectContentDir() / ItemTableFile;
	FString Csv;

	if (!FFileHelper::LoadFileToString(Csv, *FilePath))
	{
		UE_LOG(LogFactoryGame, Error, TEXT("Could not read the item table '%s'."), *FilePath);
		return;
	}

	// Fehler in der CSV (falscher Enum-Wert, unbekannte Spalte ...) erscheinen als Warnung im Log
	for (const FString& Problem : ItemTable->CreateTableFromCSVString(Csv))
	{
		UE_LOG(LogFactoryGame, Warning, TEXT("Item table: %s"), *Problem);
	}

	// stable sort keeps the table order inside a tier
	SortedItemIds = ItemTable->GetRowNames();
	SortedItemIds.StableSort([this](const FName& A, const FName& B)
	{
		return FindItem(A)->Tier < FindItem(B)->Tier;
	});

	UE_LOG(LogFactoryGame, Log, TEXT("Loaded %d items from '%s'."), SortedItemIds.Num(), *ItemTableFile);
}

const FFactoryItemDefinition* UFactoryItemDatabase::FindItem(FName ItemId) const
{
	return ItemTable && !ItemId.IsNone() ? ItemTable->FindRow<FFactoryItemDefinition>(ItemId, TEXT("FindItem"), false) : nullptr;
}

int32 UFactoryItemDatabase::GetMaxStackSize(FName ItemId) const
{
	const FFactoryItemDefinition* Item = FindItem(ItemId);
	return Item ? Item->GetMaxStackSize() : 0;
}

FText UFactoryItemDatabase::GetDisplayName(FName ItemId) const
{
	const FFactoryItemDefinition* Item = FindItem(ItemId);
	return Item ? Item->GetDisplayName() : FText::FromName(ItemId);
}
