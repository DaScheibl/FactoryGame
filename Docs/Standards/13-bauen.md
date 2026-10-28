# 13 – Bauen

Gehört zu `SYS_Build` (Bausystem), `EQP_BuildGun` und später `UI_BuildMenu`. Raster und Ports: [03 Raster](03-raster.md).

Das Bausystem kommt in drei Schritten:

1. **Gebäude platzieren** (Hologramm, Drehen, Kosten, Abriss, Speichern) ✅
2. **Bänder bauen** (zwischen Ports ziehen, Modi, Bandstützen)
3. **Baumenü** (`UI_BuildMenu`, Kategorien & Suche)

## Entscheidungen

| Thema | Entscheidung |
|---|---|
| Baukosten | **Direkt aus dem Inventar** beim Platzieren (kein Gebäude-Item) |
| Abriss | **Alles zurück:** 100 % der Baukosten + Inhalt (Puffer, Items auf dem Band) |
| Abriss-Sicherheit | **Linksklick 0,5 s gedrückt halten**, das Ziel leuchtet vorher rot |
| Inventar voll beim Abriss | Der Rest landet in einer **Abriss-Kiste** am Boden (mit `E` leeren) |
| Untergrund | **Gelände mit Toleranz:** bis 1 m Höhenunterschied unter dem Footprint, das Gebäude steht auf dem tiefsten Punkt und sinkt in den Rest ein. Auf Fundamenten (später) rastet es aufs Raster |
| Ports | **Feste Ports pro Gebäude**: Eingänge hinten, Ausgänge vorn, mittig auf 1-m-Rasterpunkten, 2 m Abstand |
| Steuerung | **Schema A** (siehe unten) |

### Bänder (Schritt 2)

| Thema | Entscheidung |
|---|---|
| Modi (Wechsel mit `R`) | **Standard** (weiche S-Kurve nach Port-Richtung), **Gerade** (nur gerade Stücke + 90°-Ecken), **Kurve** (gleichmäßige Bögen) |
| Grenzen pro Abschnitt | **max. 56 m** (7 Kacheln), **max. 35° Steigung**, **Kurvenradius min. 2 m** |
| Freies Ende | Automatisch eine **Bandstütze** (`LOG_BeltPole`), von dort weiterziehen |
| Kosten | **1 Eisenplatte pro angefangene 2 m** (Mk1; höhere Stufen später). Gilt schon jetzt für den Abriss von Cheat-Bändern |

## Steuerung (Schema A)

| Aktion | Maus/Tastatur | Gamepad |
|---|---|---|
| Gebäude wählen | `1`–`0` (Hotbar), später `Q` = Baumenü | Steuerkreuz links/rechts |
| Platzieren | Linksklick | RT |
| Drehen | Mausrad (90°), `Strg` + Mausrad (15°) | LB / RB |
| Modus wechseln (Bänder) | `R` | Y |
| Abbrechen | Rechtsklick oder `Esc` | B |
| Abriss-Modus an/aus | `F`, dann Linksklick **halten** | Steuerkreuz runter, RT halten |
| Abriss-Kiste leeren | `E` | X |

> ✏️ **Entwurf, von dir freizugeben:** `E` / Gamepad X als allgemeine Interaktions-Taste (bisher nur für Kisten, später auch Maschinen-Fenster).

## Gebäude-Tabelle

**Datei:** `Content/FactoryGame/Core/Data/DT_Buildings.csv`. Gleiches Prinzip wie Items und Rezepte: wird beim Start gelesen, im Spiel mit `ReloadBuildings` neu laden.

| Spalte | Inhalt |
|---|---|
| `---` | Gebäude-ID (`BLD_…`) |
| `NameEN` / `NameDE` | Anzeigename |
| `Tier` | 0–8, Freischalt-Tier (wird noch nicht geprüft) |
| `Category` | Kategorie fürs Baumenü, z. B. `Production` |
| `SizeX` | Länge in Fluss-Richtung (hinten → vorn) in m |
| `SizeY` | Breite in m |
| `SizeZ` | Höhe in m |
| `Inputs` / `Outputs` | Anzahl Band-Eingänge (hinten) / -Ausgänge (vorn) |
| `Cost1`…`Cost4`, `Cost1Amount`… | Baukosten. Unbenutzte Spalten: leer und `0` |

Beim Laden wird jede Zeile geprüft (unbekannte oder nicht tragbare Kosten-Items, ID ohne Menge, mehr Ports als Kantenmeter); Fehler stehen im Output-Log.

## Gebäude (mit dir festgelegt)

| Gebäude | Tier | Größe (L × B × H) | Ports | Kosten |
|---|---|---|---|---|
| Schmelzofen | 1 | 4 × 4 × 5 m | 1 → 1 | 10 Eisenplatten |
| Konstruktor | 1 | 8 × 4 × 4 m | 1 → 1 | 8 Eisenplatten, 8 Kupferdraht |
| Assembler | 2 | 8 × 8 × 5 m | 2 → 1 | 6 Verst. Eisenplatten, 4 Rotoren |
| Gießerei | 2 | 8 × 4 × 6 m | 2 → 1 | 10 Zahnräder, 10 Beton |

> ✏️ **Entwurf, von dir freizugeben:** die **Höhen** (5 / 4 / 5 / 6 m), Größe und Kosten hast du festgelegt.

Damit der erste Assembler baubar ist, gibt es den **Rotor jetzt auch an der Werkbank** (`DT_Recipes.csv`: `Workbench = True`).

## Platzieren – Regeln

- Das Hologramm folgt dem Fadenkreuz (bis 40 m) und rastet aufs **1-m-Raster**. Bei ungerader Kantenlänge liegt die Mitte auf einer Zellmitte, damit die Kanten trotzdem auf dem Raster liegen.
- Am Anfang zeigt das Gebäude vom Spieler weg (Ausgang vorn).
- **Grün** = baubar, **rot** = nicht baubar. Über der Hotbar stehen Name, Kosten (`hast / brauchst`, rot wenn es fehlt) und der Grund:
  - *Boden zu uneben*: mehr als 1 m Höhenunterschied unter Mitte und Ecken
  - *Etwas ist im Weg*: ein anderes Gebäude, Band, eine Kiste oder der Spieler im Footprint, oder irgendetwas (Fels, Wand) oberhalb der 1-m-Einsink-Zone
  - *Nicht genug Material*
- Nach dem Bauen bleibt das Gebäude gewählt, man kann also direkt das nächste setzen.
- Hologramm ist vorerst ein deckender Würfel (grün/rot) mit Port-Platten, den echten Hologramm-Shader liefert `MAT_Hologram`.

## Speichern

Maschinen (Gebäude, Position, Rezept, Puffer, Fortschritt) und Abriss-Kisten werden mit der Welt gespeichert (Spielstand-Version 3). **Bänder noch nicht**, das kommt mit Schritt 2, weil sie dann an Ports hängen.

## Testwelt

`/Game/FactoryGame/Maps/Lvl_Flat`: 2 × 2 km flache Ebene mit Raster-Material, Sonne und Himmel. Sie ist vorerst die Spielwelt, das Hauptmenü zeigt weiter die Template-Map. Neu erzeugen (Editor geschlossen):

```
UnrealEditor-Cmd.exe "<Projekt>\FactoryGame.uproject" -run=pythonscript -script="<Projekt>\Tools\CreateFlatMap.py" -unattended
```

## Test-Befehle (Konsole)

Alle Befehle auf einen Blick: [Cheatsheet](../Cheatsheet.md)

| Befehl | Wirkung |
|---|---|
| `GiveBuildCosts Smelter 5` | Material für 5 Schmelzöfen ins Inventar |
| `ListBuildings` | Alle Gebäude mit Größe, Ports und Kosten ins Log |
| `ReloadBuildings` | `DT_Buildings.csv` neu laden und prüfen |

**Schnelltest:** `GiveBuildCosts Smelter 3` → Taste `1` → Hologramm grün → Linksklick baut → Mausrad dreht → `F`, auf den Ofen zielen, Linksklick halten → Eisenplatten kommen zurück.

## Code

| Klasse | Datei |
|---|---|
| `FFactoryBuildingDefinition` (Tabellenzeile, Ports) | `Source/FactoryGame/Buildings/FactoryBuildingTypes.h` |
| `UFactoryBuildingDatabase` (lädt `DT_Buildings.csv`) | `Source/FactoryGame/Buildings/FactoryBuildingDatabase.h` |
| `UFactoryBuildComponent` (Bauwerkzeug am Player Controller) | `Source/FactoryGame/Buildings/FactoryBuildComponent.h` |
| `AFactoryHologram` (Vorschau) | `Source/FactoryGame/Buildings/FactoryHologram.h` |
| `AFactoryCrate` (Abriss-Kiste) | `Source/FactoryGame/Buildings/FactoryCrate.h` |
