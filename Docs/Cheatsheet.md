# Cheatsheet – Konsolenbefehle

Alle Test-Befehle von FactoryGame auf einen Blick. Sie funktionieren im Editor (Play) und in Development-Builds, **nicht** im fertigen Spiel (Shipping).

**Konsole öffnen:** Taste `^` (deutsche Tastatur) bzw. `` ` ``. Ein zweites Drücken öffnet die große Konsole mit Verlauf.
**Ausgaben** landen im Output Log (Editor: *Window → Output Log*, nach `LogFactoryGame` filtern).
**Kurzformen:** Die Präfixe `ITM_`, `RCP_` und `BLD_` können fast überall weggelassen werden (`GiveItem IronPlate` = `GiveItem ITM_IronPlate`). Groß-/Kleinschreibung ist egal.

> Neue Befehle kommen in `Source/FactoryGame/FactoryCheatManager.h` (jede `UFUNCTION(Exec)` ist automatisch ein Befehl) **und hier** in die Liste.

## Items & Inventar

| Befehl | Wirkung | Beispiel |
|---|---|---|
| `GiveItem <Item> [Anzahl]` | Items ins Inventar (Standard: 1) | `GiveItem IronPlate 50` |
| `GiveAllItems [MaxTier]` | Je einen vollen Stapel aller tragbaren Items bis zur Tier (Standard: 8) | `GiveAllItems 2` |
| `ClearInventory` | Inventar leeren | |
| `SetInventorySlots <Anzahl>` | Slot-Anzahl frei setzen | `SetInventorySlots 36` |
| `UpgradeInventory` | Eine Forschungsstufe Inventar (+10 Slots, max. 60) | |
| `ListItems` | Alle Item-IDs mit Tier und Stapelgröße ins Log | |
| `ReloadItems` | `DT_Items.csv` neu laden (prüft danach auch die Rezepte) | |
| `SetHotbar <Slot 1–10> <ID>` | Shortcut auf einen Hotbar-Slot legen (HUD-Test) | `SetHotbar 1 BLD_Drill_Mk1` |

## Rezepte

| Befehl | Wirkung | Beispiel |
|---|---|---|
| `ListRecipes` | Alle Rezepte mit Mengen und Werten pro Minute ins Log | |
| `ListRecipes <Gebäude>` | Nur die Rezepte eines Gebäudes (`Workbench` = alles, was von Hand geht) | `ListRecipes Constructor` |
| `ListRecipes <Item>` | Rezepte, die das Item herstellen oder verbrauchen | `ListRecipes IronPlate` |
| `ReloadRecipes` | `DT_Recipes.csv` neu laden und prüfen | |

## Maschinen

„Maschine im Blick“ = die unter dem Fadenkreuz (bis 15 m), sonst die nächste im Umkreis von 20 m.

| Befehl | Wirkung | Beispiel |
|---|---|---|
| `SpawnMachine <Gebäude> [Rezept]` | Graybox-Maschine 8 m vor dir aufstellen (auf 1-m-Raster) | `SpawnMachine Smelter IronIngot` |
| `SetMachineRecipe <Rezept>` | Rezept der Maschine im Blick ändern, Inhalt geht ins Inventar | `SetMachineRecipe CopperIngot` |
| `FeedMachine` | Alle Eingänge bis zum vollen Stapel füllen | |
| `FeedMachine <Item> [Anzahl]` | Bestimmte Menge einfüllen (ohne Anzahl: bis voll) | `FeedMachine IronOre 10` |
| `TakeOutput` | Ausgang ins Inventar holen (so viel, wie passt) | |
| `MachineInfo` | Zustand, Puffer, Fortschritt und Durchläufe ins Log | |
| `DestroyMachine` | Maschine im Blick entfernen | |

Farben der Graybox: grau = kein Rezept · gelb = wartet auf Zutaten · grün = arbeitet · rot = Ausgang voll.

## Bauen

Steuerung: `1`–`0` Gebäude wählen · Linksklick bauen · Mausrad drehen (`Strg`: 15°) bzw. Stützenhöhe · `R` Band-Modus · Rechtsklick/`Esc` abbrechen · `F` Abriss-Modus (Linksklick halten) · `E` Abriss-Kiste leeren / Maschine öffnen, auf einem Vorkommen halten = Erz abbauen · `Q` Baumenü. Details: [13 Bauen](Standards/13-bauen.md), [15 Abbau](Standards/15-abbau.md)

| Befehl | Wirkung | Beispiel |
|---|---|---|
| `GiveBuildCosts <Gebäude> [Anzahl]` | Material für ein oder mehrere Gebäude ins Inventar | `GiveBuildCosts Smelter 5` |
| `ListBuildings` | Alle Gebäude mit Größe, Ports und Kosten ins Log | |
| `ReloadBuildings` | `DT_Buildings.csv` neu laden und prüfen | |
| `SpawnNode <Erz> [Impure/Normal/Pure]` | Erz-Vorkommen 8 m vor dir setzen (Standard: Normal) | `SpawnNode CopperOre Pure` |

## Förderbänder

„Band im Blick“ = das unter dem Fadenkreuz (bis 15 m), sonst das nächste im Umkreis von 20 m.

| Befehl | Wirkung | Beispiel |
|---|---|---|
| `BeltTest [Stufe]` | Fertiger Aufbau: Schmelzofen (voll Eisenerz) → Band → Konstruktor, hintereinander vor dir | `BeltTest 2` |
| `ConnectBelt [Stufe]` | Zweimal eingeben: erst auf die Quell-, dann auf die Zielmaschine schauen. Baut ein Band vom freien Ausgang zum nächsten freien Eingang (Stufe 1–5, Standard Mk1) | `ConnectBelt 3` |
| `FeedBelt <Item> [Anzahl]` | Items aufs Band legen, von vorn dicht gepackt (ohne Anzahl: bis voll) | `FeedBelt IronOre 10` |
| `SetBeltTier <1–5>` | Stufe ändern (Mk1 60 · Mk2 120 · Mk3 240 · Mk4 480 · Mk5 960 Items/min) | `SetBeltTier 5` |
| `BeltInfo` | Stufe, Tempo, Länge, Items pro Sorte, gestaut?, ausgelieferte Items ins Log | |
| `DestroyBelt` | Band entfernen, Items gehen ins Inventar | |

## Simulation

| Befehl | Wirkung | Beispiel |
|---|---|---|
| `SimStats` | Tick-Zähler, Anzahl Objekte, ms pro Tick, verworfene Ticks, Tempo | |
| `SimSpeed <Faktor>` | Simulation schneller/langsamer (0–100), `1` = normal, `0` = angehalten | `SimSpeed 10` |

## Schnelltests

**Schmelzofen:**
```
SpawnMachine Smelter IronIngot
FeedMachine
SimSpeed 20
MachineInfo
TakeOutput
SimSpeed 1
```

**Bauen:**
```
GiveBuildCosts Smelter 3
GiveBuildCosts Constructor 2
```
Dann `1` (Schmelzofen) bzw. `2` (Konstruktor) drücken, Linksklick baut, `F` + Linksklick halten reißt ab.

**Förderband:**
```
BeltTest
SimSpeed 10
BeltInfo
SetBeltTier 5
SimSpeed 1
```

**Kette Erz → Platte** (zwei Maschinen nebeneinander, Ausgang per Hand umfüllen):
```
SpawnMachine Smelter IronIngot
FeedMachine
TakeOutput
SpawnMachine Constructor IronPlate
FeedMachine IronIngot
```

**Inventar voll machen:** `GiveAllItems 2`, danach `ClearInventory`.

## Nützliche Engine-Befehle

Kommen von Unreal selbst, nicht von uns.

| Befehl | Wirkung |
|---|---|
| `stat fps` / `stat unit` | FPS bzw. Frame-, Game- und Render-Zeit einblenden |
| `stat game` | Zeit pro Tick-Gruppe (die Simulation läuft unter *Tickables*) |
| `fly` / `ghost` / `walk` | Fliegen / Fliegen durch Wände / wieder normal laufen |
| `god` | Unverwundbar |
| `slomo <Faktor>` | Ganze Spielzeit verlangsamen/beschleunigen (betrifft auch die Simulation) |
| `teleport` | Zu dem Punkt unter dem Fadenkreuz springen |
| `t.MaxFPS <Wert>` | FPS begrenzen, z. B. `t.MaxFPS 10` um Ruckler/Aufholen zu testen |
| `showlog` | Log-Fenster öffnen (nur in gepackten Development-Builds) |
