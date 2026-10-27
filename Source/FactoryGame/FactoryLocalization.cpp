// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryLocalization.h"
#include "FactoryGameUserSettings.h"

namespace FactoryLoc
{
	struct FEntry
	{
		const TCHAR* English;
		const TCHAR* German;
	};

	// German umlauts are written as unicode escapes to stay independent of the source file encoding
	static const TMap<FName, FEntry>& GetTable()
	{
		static const TMap<FName, FEntry> Table = {
			// main menu
			{ TEXT("Menu_Title"),				{ TEXT("FACTORY GAME"),					TEXT("FACTORY GAME") } },
			{ TEXT("Menu_Continue"),			{ TEXT("Continue"),						TEXT("Fortsetzen") } },
			{ TEXT("Menu_ContinueHint"),		{ TEXT("World: {0}"),					TEXT("Welt: {0}") } },
			{ TEXT("Menu_NewWorld"),			{ TEXT("New World"),					TEXT("Neue Welt") } },
			{ TEXT("Menu_LoadWorld"),			{ TEXT("Load World"),					TEXT("Welt laden") } },
			{ TEXT("Menu_Settings"),			{ TEXT("Settings"),						TEXT("Einstellungen") } },
			{ TEXT("Menu_Quit"),				{ TEXT("Quit"),							TEXT("Beenden") } },

			// new world
			{ TEXT("NewWorld_Header"),			{ TEXT("Create a new world"),			TEXT("Neue Welt erstellen") } },
			{ TEXT("NewWorld_NameLabel"),		{ TEXT("World name"),					TEXT("Name der Welt") } },
			{ TEXT("NewWorld_DefaultName"),		{ TEXT("World {0}"),					TEXT("Welt {0}") } },
			{ TEXT("NewWorld_Create"),			{ TEXT("Create"),						TEXT("Erstellen") } },
			{ TEXT("NewWorld_NameEmpty"),		{ TEXT("Please enter a name."),			TEXT("Bitte gib einen Namen ein.") } },
			{ TEXT("NewWorld_Failed"),			{ TEXT("The world could not be created."), TEXT("Die Welt konnte nicht erstellt werden.") } },

			// load world
			{ TEXT("Load_Header"),				{ TEXT("Load world"),					TEXT("Welt laden") } },
			{ TEXT("Load_Empty"),				{ TEXT("No saved worlds yet."),			TEXT("Noch keine gespeicherten Welten.") } },
			{ TEXT("Load_Load"),				{ TEXT("Load"),							TEXT("Laden") } },
			{ TEXT("Load_Delete"),				{ TEXT("Delete"),						TEXT("Löschen") } },
			{ TEXT("Load_ConfirmDelete"),		{ TEXT("Really delete?"),				TEXT("Wirklich löschen?") } },
			{ TEXT("Load_Details"),				{ TEXT("Last played: {0}   ·   Play time: {1}"), TEXT("Zuletzt gespielt: {0}   ·   Spielzeit: {1}") } },
			{ TEXT("Load_Failed"),				{ TEXT("The world could not be loaded."), TEXT("Die Welt konnte nicht geladen werden.") } },

			// settings
			{ TEXT("Settings_Header"),			{ TEXT("Settings"),						TEXT("Einstellungen") } },
			{ TEXT("Settings_Language"),		{ TEXT("Language"),						TEXT("Sprache") } },
			{ TEXT("Settings_WindowMode"),		{ TEXT("Window mode"),					TEXT("Fenstermodus") } },
			{ TEXT("Settings_Resolution"),		{ TEXT("Resolution"),					TEXT("Auflösung") } },
			{ TEXT("Settings_Quality"),			{ TEXT("Graphics quality"),				TEXT("Grafikqualität") } },
			{ TEXT("Settings_VSync"),			{ TEXT("VSync"),						TEXT("VSync") } },
			{ TEXT("Settings_Apply"),			{ TEXT("Apply"),						TEXT("Übernehmen") } },
			{ TEXT("Settings_Applied"),			{ TEXT("Settings applied."),			TEXT("Einstellungen übernommen.") } },
			{ TEXT("WindowMode_Fullscreen"),	{ TEXT("Fullscreen"),					TEXT("Vollbild") } },
			{ TEXT("WindowMode_Borderless"),	{ TEXT("Borderless"),					TEXT("Randlos") } },
			{ TEXT("WindowMode_Windowed"),		{ TEXT("Windowed"),						TEXT("Fenster") } },
			{ TEXT("Quality_0"),				{ TEXT("Low"),							TEXT("Niedrig") } },
			{ TEXT("Quality_1"),				{ TEXT("Medium"),						TEXT("Mittel") } },
			{ TEXT("Quality_2"),				{ TEXT("High"),							TEXT("Hoch") } },
			{ TEXT("Quality_3"),				{ TEXT("Epic"),							TEXT("Episch") } },
			{ TEXT("Quality_4"),				{ TEXT("Cinematic"),					TEXT("Kino") } },
			{ TEXT("Language_en"),				{ TEXT("English"),						TEXT("English") } },
			{ TEXT("Language_de"),				{ TEXT("Deutsch"),						TEXT("Deutsch") } },

			// pause menu
			{ TEXT("Pause_Header"),				{ TEXT("Paused"),						TEXT("Pausiert") } },
			{ TEXT("Pause_Resume"),				{ TEXT("Resume"),						TEXT("Weiterspielen") } },
			{ TEXT("Pause_Save"),				{ TEXT("Save world"),					TEXT("Welt speichern") } },
			{ TEXT("Pause_Saved"),				{ TEXT("World saved."),					TEXT("Welt gespeichert.") } },
			{ TEXT("Pause_SaveFailed"),			{ TEXT("Saving failed!"),				TEXT("Speichern fehlgeschlagen!") } },
			{ TEXT("Pause_MainMenu"),			{ TEXT("Save & main menu"),				TEXT("Speichern & Hauptmenü") } },

			// common
			{ TEXT("Common_On"),				{ TEXT("On"),							TEXT("An") } },
			{ TEXT("Common_Off"),				{ TEXT("Off"),							TEXT("Aus") } },
			{ TEXT("Common_Back"),				{ TEXT("Back"),							TEXT("Zurück") } },
		};
		return Table;
	}

	static bool IsGerman()
	{
		const UFactoryGameUserSettings* Settings = UFactoryGameUserSettings::GetFactoryGameUserSettings();
		return Settings && Settings->GetLanguage() == TEXT("de");
	}

	FText Get(FName Key)
	{
		if (const FEntry* Entry = GetTable().Find(Key))
		{
			return FText::FromString(IsGerman() ? Entry->German : Entry->English);
		}

		// make missing keys visible instead of showing nothing
		return FText::FromString(FString::Printf(TEXT("<%s>"), *Key.ToString()));
	}

	FText Format(FName Key, FFormatOrderedArguments Args)
	{
		return FText::Format(FTextFormat(Get(Key)), MoveTemp(Args));
	}
}
