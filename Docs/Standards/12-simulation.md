# 12 – Simulation

Gehört zu `SYS_Sim` und `SYS_BeltSim` (später auch `SYS_Power`).

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

## Förderbänder (`SYS_BeltSim`)

### Entscheidungen

| Thema | Entscheidung |
|---|---|
| Spuren | **1 Spur** pro Band |
| Item-Abstand | **fest 0,5 m**, höhere Stufen laufen schneller |
| Durchsatz | **Mk1 60 · Mk2 120 · Mk3 240 · Mk4 480 · Mk5 960 Items/min** (jede Stufe verdoppelt) |
| Testen | Graybox-Band + Konsolenbefehle, Kurven/Bauen kommen mit dem Bausystem |

| Stufe | Items/min | Tempo | mm pro Tick |
|---|---|---|---|
| Mk1 | 60 | 0,5 m/s | 25 |
| Mk2 | 120 | 1 m/s | 50 |
| Mk3 | 240 | 2 m/s | 100 |
| Mk4 | 480 | 4 m/s | 200 |
| Mk5 | 960 | 8 m/s | 400 |

Ein Mk1-Band reicht also für 2 Schmelzöfen (je 30 Barren/min).

### Prinzip: Lücken statt Positionen

- Ein Band (`UFactoryBeltComponent`) speichert **nicht die Position jedes Items**, sondern nur die **Lücke** zum Item davor (vorne beginnend, das vorderste Item: Abstand zum Bandende). Gerechnet wird in ganzen Millimetern.
- Läuft das Band, wird pro Tick nur **die erste Lücke kleiner**, alle Items dahinter rücken dadurch automatisch mit. Ein Band mit 200 Items kostet so kaum mehr als eins mit 2.
- **Stau:** Nimmt der Empfänger am Ende nichts an, bleibt das vorderste Item stehen, die Lücken dahinter schließen sich von vorn nach hinten. Dicht gepackte Items werden übersprungen, ein volles gestautes Band kostet fast nichts.
- **Übergabe:** Am Ende geht das Item an einen `IFactoryItemReceiver` (Maschinen-Eingang, später nächstes Band, Splitter, Kiste). Was von der Bewegung im Tick übrig ist, wird mitgegeben, damit auch Mk5 (400 mm/Tick bei 500 mm Abstand) seinen vollen Durchsatz schafft.
- **Anfang:** Ist hinten Platz, holt sich das Band ein Item aus dem Ausgang der Quell-Maschine.
- Die Optik (`AFactoryBelt`) liest jedes Bild die Positionen und schiebt die Würfel dazwischen weiter (`GetDisplayPositions`), die Simulation bleibt deterministisch.
- **Später:** Hintereinander gebaute Bänder werden mit dem Bausystem zu **einem Segment** zusammengelegt (dann entfällt die Übergabe zwischen ihnen). Bis dahin kann ein Item beim Wechsel auf ein Band, das im selben Tick erst danach rechnet, einmal doppelt vorrücken. Das bleibt deterministisch.

### Graybox-Band

`AFactoryBelt`: dunkler Streifen (0,8 m breit) entlang einer Kurve, Oberkante auf Port- bzw. Stützenhöhe, Items als 30-cm-Würfel in der Platzhalterfarbe aus `DT_Items` (Instanced Mesh pro Sorte). Über dem Band steht Stufe, Durchsatz, Anzahl Items und ggf. „Gestaut“.
Gebaut werden Bänder mit dem Bauwerkzeug ([13 Bauen](13-bauen.md)), gespeichert mit der Welt.

### Test-Befehle (Konsole)

| Befehl | Wirkung |
|---|---|
| `BeltTest 2` | Schmelzofen (voll Eisenerz) → Mk2-Band → Konstruktor (Eisenplatten), hintereinander vor dir |
| `ConnectBelt [Stufe]` | 1. Aufruf: Maschine im Blick = Quelle, 2. Aufruf: Maschine im Blick = Ziel, Band vom freien Ausgang zum nächsten freien Eingang |
| `FeedBelt IronOre [Anzahl]` | Items aufs Band im Blick legen (von vorn dicht gepackt, ohne Anzahl: bis voll) |
| `SetBeltTier 5` | Stufe des Bands im Blick ändern |
| `BeltInfo` | Stufe, Länge, Items, gestaut?, ausgelieferte Items ins Log |
| `DestroyBelt` | Band im Blick entfernen, Items gehen ins Inventar |

**Schnelltest:** `BeltTest` → Schmelzofen wird grün, alle 2 s läuft ein Eisenbarren übers Band in den Konstruktor → `SimSpeed 10` → der Konstruktor arbeitet bald, Barren stauen sich vor ihm, sobald sein Eingang voll ist (Band zeigt „Gestaut“) → `SimSpeed 1`.

## Code

| Klasse | Datei |
|---|---|
| `UFactorySimulationSubsystem` (World-Subsystem, fester Takt) | `Source/FactoryGame/Simulation/FactorySimulationSubsystem.h` |
| `IFactorySimulated` (Interface für alles Simulierte) | `Source/FactoryGame/Simulation/FactorySimulated.h` |
| `UFactoryProductionComponent` (Rezepte abarbeiten, Puffer) | `Source/FactoryGame/Simulation/FactoryProductionComponent.h` |
| `AFactoryMachine` (Graybox-Maschine) | `Source/FactoryGame/Buildings/FactoryMachine.h` |
| `UFactoryBeltComponent` (Band-Simulation, Lücken-Verfahren) | `Source/FactoryGame/Simulation/FactoryBeltComponent.h` |
| `IFactoryItemReceiver` (alles, was Items vom Band annimmt) | `Source/FactoryGame/Simulation/FactoryItemReceiver.h` |
| `AFactoryBelt` (Graybox-Band) | `Source/FactoryGame/Buildings/FactoryBelt.h` |
