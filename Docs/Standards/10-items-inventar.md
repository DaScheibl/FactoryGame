# 10 – Items, Inventar & Hotbar

Gehört zu `SYS_Recipes`, `UI_Inventory`, `UI_HUD` und den Ausrüstungs-Items (`EQP_*`).

## Entscheidungen

| Thema | Entscheidung |
|---|---|
| Item-IDs | **Englisch**, `ITM_<PascalCase>`, z. B. `ITM_IronPlate`. Keine Umlaute. Anzeigenamen DE/EN kommen aus der Tabelle. |
| Forschungspakete | `ITM_SciencePack_01` … `ITM_SciencePack_07` |
| Inventar | **20 Slots** zum Start, **+10 pro Forschungsstufe** bis **60** (4 Stufen). **Nur über Forschung**, kein Rucksack-Item. |
| Inventar-Fenster | **Ausrüstung links, Inventar-Raster (10 Spalten) rechts.** Kisten/Maschinen öffnen später ein eigenes Panel. |
| Ausrüstung | **4 Slots: Kopf, Körper, Rücken, Hand.** Das Bauwerkzeug ist immer da und braucht keinen Slot. |
| Bedienung | **Stapel am Cursor, Rechtsklick (halb / einzeln), Shift-Klick, Strg-Klick, Drag & Drop** (Details unten) |
| Crafting | **Kein Handcrafting im Inventar**, nur an einer Werkbank (Gebäude) |
| Items loswerden | **Recycler-Gebäude** (`BLD_Recycler`), kein Mülleimer-Slot im Inventar |
| Stapelgrößen | **Nach Item-Klasse** (Tabelle unten) |
| Hotbar | **10 Slots mit Bau-Shortcuts** (Tasten 1–9, 0), wie Satisfactory. Ein Gebäude-Slot startet das Platzieren ([13 Bauen](13-bauen.md)). Bis es das Baumenü gibt, bekommt eine leere Hotbar automatisch alle Gebäude. |

## Stapel-Klassen

| Klasse (`StackClass`) | Stapel | Beispiele |
|---|---|---|
| `Small` | 500 | Schraube, Kupferdraht, Kieselerde |
| `Standard` | 100 | Erze, Barren, Platten, Stangen, Kabel, Forschungspakete |
| `Component` | 50 | Verstärkte Eisenplatte, Rotor, Motor, Schaltkreis |
| `Large` | 20 | Computer, Schwerer Modularrahmen, Raketensegment |
| `Single` | 1 | Ausrüstung: Werkzeuge, Jetpack, Atemmaske |

Die Werte stehen an einer einzigen Stelle im Code (`FFactoryItemDefinition::GetStackSize`).

> ✏️ **Entwurf, von dir freizugeben:** die Zuordnung der einzelnen Items zu den Klassen in `DT_Items.csv`.

## Item-Tabelle

**Datei:** `Content/FactoryGame/Core/Data/DT_Items.csv` (UTF-8, mit Excel/LibreOffice oder Rider editierbar, in Git diffbar).
Das Spiel liest die CSV beim Start in eine DataTable (`UFactoryItemDatabase`). Kein Import-Schritt im Editor nötig; beim Packen wird der Ordner automatisch mitkopiert.

| Spalte | Inhalt |
|---|---|
| `---` | Item-ID (Zeilenname) |
| `NameEN` / `NameDE` | Anzeigename |
| `DescriptionEN` / `DescriptionDE` | Kodex-/Tooltip-Text (kommt mit `STY_Codex`) |
| `Tier` | 0–8, Freischalt-Tier |
| `Form` | `Solid` oder `Fluid` |
| `StackClass` | siehe oben |
| `EquipSlot` | `None` für normale Items, sonst `Head`, `Body`, `Back` oder `Hand` |
| `PlaceholderColor` | Farbe im UI, solange es kein Icon gibt: `(B=…,G=…,R=…,A=255)` |
| `Icon` | Soft-Pfad auf die 256-px-Textur (aus dem Icon-Rig, `TOOL_IconRig`) |
| `BeltMesh` | Soft-Pfad auf das Band-Mesh |

Nach dem Bearbeiten im laufenden Spiel neu laden: Konsole → `ReloadItems`.

Rohstoffe, die man tragen kann (Erze, Kohle, Kalkstein, Quarz, Schwefel, Bauxit, Uran, Xenokristall), stehen als `ITM_` in derselben Tabelle. Das Vorkommen in der Welt behält die `RES_`-ID.

> ✏️ **Entwurf, von dir freizugeben:**
> - Englische Namen und Platzhalterfarben aller Items.
> - **Fluide** (`Form = Fluid`: Schwefelsäure, Treibstoff, Schweröl-Rückstand, Raketentreibstoff) passen **nicht** ins Inventar, nur in Rohre/Tanks bzw. später in Kanister (wie Satisfactory).
> - Rohquarz heißt `ITM_Quartz`, das verarbeitete Produkt `ITM_QuartzCrystal`.

## Inventar & HUD (Bedienung)

> ✏️ **Entwurf, von dir freizugeben:** Tastenbelegung (v. a. Gamepad), Fenster-Layout und Slot-Größe (64 px).

**Prinzip:** Ein aufgenommener Stapel bleibt in seinem Slot, bis er abgelegt wird. Schließen, Escape oder ein Klick daneben legt ihn einfach zurück. Dadurch geht nie etwas verloren.

| Aktion | Maus/Tastatur | Gamepad |
|---|---|---|
| Inventar öffnen/schließen | `Tab` oder `I` | View-Taste |
| Ganzen Stapel aufnehmen / ablegen | Linksklick | A |
| Halben Stapel aufnehmen / 1 Stück ablegen | Rechtsklick | X |
| Ziehen & Ablegen | Linke Maustaste gedrückt halten und ziehen | – |
| Schnell verschieben (Inventar ↔ Ausrüstung, später ↔ Kiste) | Shift + Klick | Y |
| Alle Stapel dieser Sorte verschieben | Strg + Klick | – |
| Aufnehmen abbrechen / schließen | Escape oder Klick neben die Slots | B |
| Hotbar-Slot wählen | `1`–`9`, `0` | Steuerkreuz links/rechts |

- Ablegen auf gleiche Items führt zusammen; was nicht mehr passt, bleibt am Cursor. Verschiedene Items werden getauscht (nur ganze Stapel).
- Ausrüstungs-Slots nehmen nur passende Items an. Shift-Klick auf ein Werkzeug rüstet es aus und tauscht ein schon ausgerüstetes zurück ins Inventar.
- Das Spiel läuft weiter, während das Inventar offen ist (keine Pause).
- Button **Sortieren** fasst Teilstapel zusammen und sortiert nach Tier und Tabellen-Reihenfolge.
- Unter dem Raster stehen Name, Tier, Stapelinfo und Beschreibung des Items unter dem Mauszeiger, darunter die Steuerungshinweise (wechseln automatisch zwischen Maus und Gamepad).

> ✏️ **Entwurf, von dir freizugeben:** Ausrüstungs-Items in `DT_Items.csv` (Handbohrer, Stirnlampe, Scanner, Kettensäge, Gleiter, Jetpack, Atemmaske, Hover-Pack) mit Tiers und Farben. Für den **Körper-Slot** gibt es noch kein Item.

- HUD: Punkt-Fadenkreuz in der Mitte, Hotbar unten mittig, Hinweiszeile darüber, Benachrichtigungen oben rechts (z. B. „+50 Eisenplatte“, „Inventar voll“; gleiche Items werden zusammengefasst, 4 s sichtbar, max. 5).
- Ohne Icons zeigt jeder Slot die Platzhalterfarbe, den Namen und die Anzahl.

## Speichern

Inventar, Ausrüstung und Hotbar werden mit der Welt gespeichert (seit Version 2 auch die Ausrüstung; ältere Spielstände mit 18 Slots werden auf 20 angehoben). Der Spielstand hat eine **Versionsnummer** (`EFactoryWorldSaveVersion`). Neue Felder im Spielstand → neue Version anlegen und in `UFactoryWorldSaveGame::MigrateToLatest()` behandeln. Spielstände aus einer neueren Spielversion werden nicht geladen (statt sie zu überschreiben).

## Test-Befehle (Konsole, nur Editor/Development)

Alle Befehle auf einen Blick: [Cheatsheet](../Cheatsheet.md)

Konsole öffnen: Taste `~` (auf deutscher Tastatur meist `Ö` oder `^`).

| Befehl | Wirkung |
|---|---|
| `GiveItem IronPlate 50` | Items geben (`ITM_` kann weggelassen werden) |
| `GiveAllItems 2` | Je einen vollen Stapel aller tragbaren Items bis Tier 2 |
| `ClearInventory` | Inventar leeren |
| `UpgradeInventory` | Eine Forschungsstufe Inventar (+10 Slots, max. 60) |
| `SetInventorySlots 36` | Slot-Anzahl frei setzen |
| `ListItems` | Alle IDs ins Output-Log schreiben |
| `ReloadItems` | `DT_Items.csv` neu laden |
| `SetHotbar 1 BLD_Drill_Mk1` | Shortcut auf Hotbar-Slot legen (HUD-Test) |

## Code

| Klasse | Datei |
|---|---|
| `FFactoryItemDefinition`, `FFactoryItemStack` | `Source/FactoryGame/Items/FactoryItemTypes.h` |
| `UFactoryItemDatabase` (GameInstance-Subsystem) | `Source/FactoryGame/Items/FactoryItemDatabase.h` |
| `UFactoryInventoryComponent` (Inventar, Ausrüstung über Slot-Filter, später Kisten/Maschinen) | `Source/FactoryGame/Items/FactoryInventoryComponent.h` |
| `UFactoryHUDWidget`, `UFactoryInventoryWidget`, `UFactoryInventorySlotWidget` | `Source/FactoryGame/UI/` |
| `UFactoryCheatManager` | `Source/FactoryGame/FactoryCheatManager.h` |
