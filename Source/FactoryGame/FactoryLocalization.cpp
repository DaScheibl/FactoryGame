// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryLocalization.h"
#include "FactoryGameUserSettings.h"

namespace FactoryLoc
{
	// ein Tabelleneintrag: derselbe Text in beiden Sprachen
	struct FEntry
	{
		const TCHAR* English;
		const TCHAR* German;
	};

	// Umlaute stehen direkt im Quelltext, die Datei muss deshalb als UTF-8 gespeichert bleiben.
	// Spalten: Schlüssel, Englisch, Deutsch. {0}, {1} ... sind Platzhalter für FactoryLoc::Format.
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

			// inventory
			{ TEXT("Inv_Header"),				{ TEXT("Inventory"),					TEXT("Inventar") } },
			{ TEXT("Inv_SlotsUsed"),			{ TEXT("{0} / {1} slots used"),			TEXT("{0} / {1} Slots belegt") } },
			{ TEXT("Inv_Sort"),					{ TEXT("Sort"),							TEXT("Sortieren") } },
			{ TEXT("Inv_Close"),				{ TEXT("Close"),						TEXT("Schließen") } },
			{ TEXT("Inv_Equipment"),			{ TEXT("Equipment"),					TEXT("Ausrüstung") } },
			{ TEXT("Inv_HintMouse"),			{ TEXT("Click: pick up / put down   ·   Right click: half stack / put down one   ·   Drag & drop   ·   Shift+click: quick move   ·   Ctrl+click: move all of this item"), TEXT("Klick: aufnehmen / ablegen   ·   Rechtsklick: halber Stapel / einzeln ablegen   ·   Ziehen & Ablegen   ·   Shift+Klick: schnell verschieben   ·   Strg+Klick: alle dieser Sorte verschieben") } },
			{ TEXT("Inv_HintGamepad"),			{ TEXT("A: pick up / put down   ·   X: half stack / put down one   ·   Y: quick move   ·   B: cancel / close"), TEXT("A: aufnehmen / ablegen   ·   X: halber Stapel / einzeln ablegen   ·   Y: schnell verschieben   ·   B: abbrechen / schließen") } },
			{ TEXT("Equip_Head"),				{ TEXT("Head"),							TEXT("Kopf") } },
			{ TEXT("Equip_Body"),				{ TEXT("Body"),							TEXT("Körper") } },
			{ TEXT("Equip_Back"),				{ TEXT("Back"),							TEXT("Rücken") } },
			{ TEXT("Equip_Hand"),				{ TEXT("Hand"),							TEXT("Hand") } },
			{ TEXT("Inv_StackInfo"),			{ TEXT("{0} / {1} per stack   ·   {2} in total"), TEXT("{0} / {1} pro Stapel   ·   {2} insgesamt") } },
			{ TEXT("Inv_Tier"),					{ TEXT("Tier {0}"),						TEXT("Tier {0}") } },
			{ TEXT("Inv_Unknown"),				{ TEXT("Unknown item"),					TEXT("Unbekanntes Item") } },

			// hud
			{ TEXT("Hotbar_Empty"),				{ TEXT("Hotbar slot {0} is empty. Buildings from the build menu can be placed here."), TEXT("Hotbar-Slot {0} ist leer. Hier kommen Gebäude aus dem Baumenü hin.") } },
			{ TEXT("Notify_InventoryFull"),		{ TEXT("Inventory full"),				TEXT("Inventar voll") } },
			{ TEXT("Notify_ItemsAdded"),		{ TEXT("+{0} {1}"),						TEXT("+{0} {1}") } },
			{ TEXT("Notify_ItemsRemoved"),		{ TEXT("-{0} {1}"),						TEXT("-{0} {1}") } },

			// machines
			{ TEXT("Machine_NoRecipe"),			{ TEXT("No recipe"),					TEXT("Kein Rezept") } },
			{ TEXT("Machine_WaitingForInput"),	{ TEXT("Waiting for input"),			TEXT("Wartet auf Zutaten") } },
			{ TEXT("Machine_Working"),			{ TEXT("Working {0} %"),				TEXT("Arbeitet {0} %") } },
			{ TEXT("Machine_OutputFull"),		{ TEXT("Output full"),					TEXT("Ausgang voll") } },

			// belts
			{ TEXT("Belt_Label"),				{ TEXT("Conveyor Mk{0}  {1}/min  {2} items"), TEXT("Förderband Mk{0}  {1}/min  {2} Items") } },
			{ TEXT("Belt_Blocked"),				{ TEXT("Jammed"),						TEXT("Gestaut") } },

			// common
			{ TEXT("Common_On"),				{ TEXT("On"),							TEXT("An") } },
			{ TEXT("Common_Off"),				{ TEXT("Off"),							TEXT("Aus") } },
			{ TEXT("Common_Back"),				{ TEXT("Back"),							TEXT("Zurück") } },
		};
		return Table;
	}

	bool IsGerman()
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
