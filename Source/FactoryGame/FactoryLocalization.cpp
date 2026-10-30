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
			{ TEXT("Hotbar_Empty"),				{ TEXT("Hotbar slot {0} is empty. Q opens the build menu, where you can put buildings here."), TEXT("Hotbar-Slot {0} ist leer. Q öffnet das Baumenü, dort kannst du Gebäude hierher legen.") } },

			// build menu
			{ TEXT("BuildMenu_Header"),			{ TEXT("Build"),						TEXT("Bauen") } },
			{ TEXT("BuildMenu_Search"),			{ TEXT("Search …"),						TEXT("Suchen …") } },
			{ TEXT("BuildMenu_Hotbar"),			{ TEXT("Hotbar"),						TEXT("Hotbar") } },
			{ TEXT("BuildMenu_Ports"),			{ TEXT("{0} in → {1} out"),				TEXT("{0} rein → {1} raus") } },
			{ TEXT("BuildMenu_Cost"),			{ TEXT("Costs:"),						TEXT("Kosten:") } },
			{ TEXT("BuildMenu_CostPer2m"),		{ TEXT("Costs per 2 m:"),				TEXT("Kosten pro 2 m:") } },
			{ TEXT("BuildMenu_EmptyCategory"),	{ TEXT("Nothing to build here yet."),	TEXT("Hier gibt es noch nichts zu bauen.") } },
			{ TEXT("BuildMenu_NoResults"),		{ TEXT("No building found."),			TEXT("Kein Gebäude gefunden.") } },
			{ TEXT("BuildMenu_HintMouse"),		{ TEXT("Click: Build  ·  Drag onto the hotbar or press 1–0 on a tile: Put on the hotbar  ·  Right click on the hotbar: Clear  ·  Q / Esc: Close"), TEXT("Klick: Bauen  ·  Auf die Hotbar ziehen oder 1–0 über einer Kachel: Auf die Hotbar legen  ·  Rechtsklick auf die Hotbar: Leeren  ·  Q / Esc: Schließen") } },
			{ TEXT("BuildMenu_HintGamepad"),	{ TEXT("A: Build  ·  Y: Put on the hotbar  ·  X on the hotbar: Clear  ·  LB / RB: Category  ·  B: Close"), TEXT("A: Bauen  ·  Y: Auf die Hotbar legen  ·  X auf der Hotbar: Leeren  ·  LB / RB: Kategorie  ·  B: Schließen") } },
			{ TEXT("BuildMenu_HintHeld"),		{ TEXT("{0}: choose a hotbar slot and press A  ·  B: Cancel"), TEXT("{0}: Hotbar-Slot wählen und A drücken  ·  B: Abbrechen") } },
			{ TEXT("Category_Production"),		{ TEXT("Production"),					TEXT("Produktion") } },
			{ TEXT("Category_Logistics"),		{ TEXT("Logistics"),					TEXT("Logistik") } },
			{ TEXT("Category_Power"),			{ TEXT("Power"),						TEXT("Strom") } },
			{ TEXT("Category_Storage"),			{ TEXT("Storage"),						TEXT("Lager") } },
			{ TEXT("Category_Structure"),		{ TEXT("Structure"),					TEXT("Struktur") } },
			{ TEXT("Category_Misc"),			{ TEXT("Other"),						TEXT("Sonstiges") } },
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
			{ TEXT("Belt_Name"),				{ TEXT("Conveyor Mk{0}"),				TEXT("Förderband Mk{0}") } },

			// building
			{ TEXT("Build_Controls_Place"),		{ TEXT("LMB: Build  ·  Wheel: Rotate (Ctrl: 15°)  ·  RMB: Cancel"), TEXT("Linksklick: Bauen  ·  Mausrad: Drehen (Strg: 15°)  ·  Rechtsklick: Abbrechen") } },
			{ TEXT("Build_Controls_Demolish"),	{ TEXT("Hold LMB: Demolish  ·  F / RMB: Exit"), TEXT("Linksklick halten: Abreißen  ·  F / Rechtsklick: Beenden") } },
			{ TEXT("Build_Cost"),				{ TEXT("{0} / {1}  {2}"),				TEXT("{0} / {1}  {2}") } },
			{ TEXT("Build_Error_NoGround"),		{ TEXT("Aim at the ground"),			TEXT("Auf den Boden zielen") } },
			{ TEXT("Build_Error_Uneven"),		{ TEXT("Ground too uneven"),			TEXT("Boden zu uneben") } },
			{ TEXT("Build_Error_Blocked"),		{ TEXT("Something is in the way"),		TEXT("Etwas ist im Weg") } },
			{ TEXT("Build_Error_Materials"),	{ TEXT("Not enough materials"),			TEXT("Nicht genug Material") } },
			{ TEXT("Build_Error_NoTarget"),		{ TEXT("Aim at a building"),			TEXT("Auf ein Gebäude zielen") } },
			{ TEXT("Build_Error_NoPort"),		{ TEXT("No free connection here"),		TEXT("Kein freier Anschluss") } },
			{ TEXT("Build_Error_TooShort"),		{ TEXT("Belt too short"),				TEXT("Band zu kurz") } },
			{ TEXT("Build_Error_TooLong"),		{ TEXT("Belt too long (max. 56 m)"),	TEXT("Band zu lang (max. 56 m)") } },
			{ TEXT("Build_Error_TooSteep"),		{ TEXT("Too steep (max. 35°)"),			TEXT("Zu steil (max. 35°)") } },
			{ TEXT("Build_Error_TooTight"),		{ TEXT("Curve too tight (min. 2 m radius)"), TEXT("Kurve zu eng (min. 2 m Radius)") } },
			{ TEXT("Build_Error_Impossible"),	{ TEXT("Doesn't fit in this mode – R changes the mode"), TEXT("Passt in diesem Modus nicht – R wechselt den Modus") } },
			{ TEXT("Build_BeltTitle"),			{ TEXT("{0}  ·  Mode: {1}"),			TEXT("{0}  ·  Modus: {1}") } },
			{ TEXT("Build_BeltPickStart"),		{ TEXT("Pick the start: an output, a pole or the ground"), TEXT("Start wählen: Ausgang, Stütze oder Boden") } },
			{ TEXT("Build_Controls_BeltStart"),	{ TEXT("LMB: Start  ·  Wheel: Pole height ({0} m)  ·  R: Mode  ·  RMB: Cancel"), TEXT("Linksklick: Start  ·  Mausrad: Stützenhöhe ({0} m)  ·  R: Modus  ·  Rechtsklick: Abbrechen") } },
			{ TEXT("Build_Controls_BeltEnd"),	{ TEXT("LMB: Build  ·  Wheel: Pole height ({0} m)  ·  R: Mode  ·  RMB: Back"), TEXT("Linksklick: Bauen  ·  Mausrad: Stützenhöhe ({0} m)  ·  R: Modus  ·  Rechtsklick: Zurück") } },
			{ TEXT("Build_Controls_Pole"),		{ TEXT("LMB: Place  ·  Wheel: Height ({0} m)  ·  Ctrl+Wheel: Rotate  ·  RMB: Cancel"), TEXT("Linksklick: Setzen  ·  Mausrad: Höhe ({0} m)  ·  Strg+Mausrad: Drehen  ·  Rechtsklick: Abbrechen") } },
			{ TEXT("Belt_Mode_Standard"),		{ TEXT("Default"),						TEXT("Standard") } },
			{ TEXT("Belt_Mode_Straight"),		{ TEXT("Straight"),						TEXT("Gerade") } },
			{ TEXT("Belt_Mode_Curve"),			{ TEXT("Curve"),						TEXT("Kurve") } },
			{ TEXT("Build_DemolishTitle"),		{ TEXT("Demolish"),						TEXT("Abriss-Modus") } },
			{ TEXT("Build_DemolishTarget"),		{ TEXT("Demolish: {0}"),				TEXT("Abreißen: {0}") } },
			{ TEXT("Build_DemolishProgress"),	{ TEXT("Demolishing … {0} %"),			TEXT("Abreißen … {0} %") } },
			{ TEXT("Crate_Label"),				{ TEXT("Demolition crate: {0} items"),	TEXT("Abriss-Kiste: {0} Items") } },
			{ TEXT("Crate_Prompt"),				{ TEXT("E: Take items"),				TEXT("E: Items nehmen") } },
			{ TEXT("Notify_CrateSpawned"),		{ TEXT("Inventory full – the rest is in a demolition crate"), TEXT("Inventar voll – der Rest liegt in einer Abriss-Kiste") } },

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
