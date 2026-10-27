# 12 – Simulation

Gehört zu `SYS_Sim` (später auch `SYS_BeltSim`, `SYS_Power`).

## Entscheidungen

| Thema | Entscheidung |
|---|---|
| Takt | **20 Simulations-Ticks pro Sekunde**, unabhängig von den FPS |
| Ruckler | **Aufholen mit Limit:** verpasste Ticks werden nachgerechnet, höchstens 0,25 s pro Bild. Bei längeren Hängern läuft die Fabrik kurz langsamer |
| Maschinen-Puffer | **1 Stapel** pro Eingang und Ausgang (z. B. 100 Erz). Ausgang voll → Maschine stoppt |
| Pause | Im Pause-Menü steht die Simulation still |

## Prinzip

- Die Fabrik rechnet in einem **festen Takt** (`UFactorySimulationSubsystem`). Grafik, Animation und UI laufen im normalen Bild-Takt und lesen nur den Zustand.
- **Deterministisch:** gerechnet wird nur mit ganzen Ticks und ganzen Zahlen, ohne Zufall und ohne Abhängigkeit von FPS oder Kamera. Gleicher Spielstand + gleiche Eingaben = gleiches Ergebnis. Das ist Voraussetzung für Multiplayer und für reproduzierbare Bugs.
- Alles, was simuliert wird, implementiert `IFactorySimulated` und meldet sich beim Subsystem an. Die Reihenfolge ist fest (Anmelde-Reihenfolge).

## Maschinen-Ablauf

1. Liegen genug Zutaten im Eingangspuffer **und** passt das Ergebnis in den Ausgangspuffer, werden die Zutaten entnommen und ein Durchlauf startet.
2. Pro Tick steigt der Fortschritt um die Geschwindigkeit (100 % = Rezeptzeit; später Overclocking bis 250 %).
3. Durchlauf fertig → Ergebnis in den Ausgangspuffer, nächster Durchlauf.

Rezeptdauer in Ticks = Dauer in Sekunden × 20 (gerundet). Ein Eisenbarren (2 s) braucht also genau 40 Ticks.

Zustände: **Kein Rezept** · **Wartet auf Zutaten** · **Arbeitet** · **Ausgang voll**.
Beim Rezeptwechsel gehen Pufferinhalt und die Zutaten eines laufenden Durchlaufs an den Spieler zurück.

## Graybox-Maschine

`AFactoryMachine`: Würfel (4 × 4 × 4 m) mit Statustext darüber. Die Farbe zeigt den Zustand. Das ist nur Graybox, final kommen Status-Leuchten (`MESH_StatusLight`):

| Farbe | Zustand |
|---|---|
| Grau | Kein Rezept |
| Gelb | Wartet auf Zutaten |
| Grün | Arbeitet |
| Rot | Ausgang voll |

Maschinen werden **noch nicht gespeichert**, das kommt mit dem Bausystem.

## Test-Befehle (Konsole)

Alle Befehle auf einen Blick: [Cheatsheet](../Cheatsheet.md)

| Befehl | Wirkung |
|---|---|
| `SpawnMachine Smelter IronIngot` | Maschine 8 m vor dir aufstellen (Präfixe `BLD_`/`RCP_` optional) |
| `SetMachineRecipe CopperIngot` | Rezept der Maschine im Blick ändern |
| `FeedMachine` | Alle Eingänge der Maschine im Blick bis zum vollen Stapel füllen |
| `FeedMachine IronOre 10` | Bestimmte Menge einfüllen |
| `TakeOutput` | Ausgang ins Inventar holen |
| `MachineInfo` | Zustand, Puffer, Fortschritt ins Output-Log |
| `DestroyMachine` | Maschine im Blick entfernen |
| `SimStats` | Tick-Zähler, Anzahl Objekte, ms pro Tick, verworfene Ticks |
| `SimSpeed 10` | Simulation 10× schneller (zum Testen), `SimSpeed 1` = normal |

„Maschine im Blick“ = unter dem Fadenkreuz (bis 15 m), sonst die nächste im Umkreis von 20 m.

**Schnelltest:** `SpawnMachine Smelter IronIngot` → `FeedMachine` → Würfel wird grün, alle 2 s +1 Eisenbarren → mit `SimSpeed 20` läuft der Ausgang nach ~10 s voll, der Würfel wird rot → `TakeOutput`.

## Code

| Klasse | Datei |
|---|---|
| `UFactorySimulationSubsystem` (World-Subsystem, fester Takt) | `Source/FactoryGame/Simulation/FactorySimulationSubsystem.h` |
| `IFactorySimulated` (Interface für alles Simulierte) | `Source/FactoryGame/Simulation/FactorySimulated.h` |
| `UFactoryProductionComponent` (Rezepte abarbeiten, Puffer) | `Source/FactoryGame/Simulation/FactoryProductionComponent.h` |
| `AFactoryMachine` (Graybox-Maschine) | `Source/FactoryGame/Buildings/FactoryMachine.h` |
