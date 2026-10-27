// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryWorldSaveGame.h"
#include "FactoryGame.h"
#include "FactoryInventoryComponent.h"

// fester Slot für die Weltenliste: Saved/SaveGames/WorldIndex.sav
const FString UFactoryWorldIndexSave::SlotName = TEXT("WorldIndex");

bool UFactoryWorldSaveGame::MigrateToLatest()
{
	const int32 Latest = static_cast<int32>(EFactoryWorldSaveVersion::Latest);

	// Ein neuerer Stand enthält Daten, die diese Version nicht kennt. Laden und erneutes
	// Speichern würde sie löschen, deshalb wird er abgelehnt.
	if (SaveVersion > Latest)
	{
		UE_LOG(LogFactoryGame, Error, TEXT("World '%s' was saved with a newer game version (save version %d, supported %d)."), *DisplayName, SaveVersion, Latest);
		return false;
	}

	// Jeder folgende Block rüstet auf eine Version hoch; ein sehr alter Stand durchläuft alle nacheinander.
	if (SaveVersion < static_cast<int32>(EFactoryWorldSaveVersion::Inventory))
	{
		// nothing to convert, the player starts with the default empty inventory
		InventorySlots.Reset();
		InventoryNumSlots = 0;
		HotbarSlots.Reset();
	}

	if (SaveVersion < static_cast<int32>(EFactoryWorldSaveVersion::Equipment))
	{
		EquipmentSlots.Reset();

		// the inventory used to start with 18 slots
		if (InventoryNumSlots > 0 && InventoryNumSlots < UFactoryInventoryComponent::PlayerStartSlots)
		{
			InventoryNumSlots = UFactoryInventoryComponent::PlayerStartSlots;
		}
	}

	if (SaveVersion < Latest)
	{
		UE_LOG(LogFactoryGame, Log, TEXT("Migrated world '%s' from save version %d to %d."), *DisplayName, SaveVersion, Latest);
	}

	SaveVersion = Latest;
	return true;
}
