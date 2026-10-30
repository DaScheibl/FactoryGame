# 15 – Abbau: Erz-Vorkommen und Bergbaubohrer

Gehört zu `BLD_Drill_Mk1`, `RES_IronOre`, `RES_CopperOre`, `RES_Limestone` und dem Abbau von Hand. Grundlage: [12 Simulation](12-simulation.md), [13 Bauen](13-bauen.md).

Stand: gebaut, Test im Spiel offen.

## Entscheidungen

| Thema | Entscheidung |
|---|---|
| Vorkommen | **Feste Knoten** in der Welt (Eisenerz, Kupfererz, Kalkstein), **unendlich** |
| Reinheit | **unrein / normal / rein** = 30 / 60 / 120 Erz pro Minute mit dem Bohrer Mk1 (50 / 100 / 200 % Tempo) |
| Bohrer | Rastet mittig auf ein freies Vorkommen ein, **1 Ausgang**, kein Eingang. Ein Vorkommen trägt genau einen Bohrer |
| Abbau von Hand | **`E` halten** (Gamepad X) auf einem Vorkommen: **1 Erz pro Sekunde** ins Inventar |
| Testwelt | Feste Belegung rund um den Startpunkt, beim Start gesetzt (siehe unten). Die echte Welt kommt mit `WLD_Decision` |

## So funktioniert's

- **Bohrer bauen:** Bergbaubohrer im Baumenü (Produktion) wählen und auf ein Vorkommen zielen (bis 3,5 m daneben reicht). Das Hologramm springt auf die Mitte des Vorkommens, drehen geht frei mit dem Mausrad. Ohne Vorkommen steht „Auf ein Erzvorkommen zielen“ da, auf einem belegten „Auf diesem Vorkommen steht schon ein Bohrer“.
- **Technik:** Der Bohrer ist eine normale Maschine mit festem Abbau-Rezept `RCP_Mine_<Erz>` (1 Erz pro Sekunde, keine Zutaten). Die Reinheit setzt das Tempo der Maschine. Damit laufen Bänder, Puffer (ein Stapel), Speichern und Abriss wie bei jeder anderen Maschine.
- **Maschinen-Panel:** Beim Bohrer steht statt der Rezeptliste „Baut Eisenerz ab · 120/min (200 % durch die Reinheit)“. Das Rezept lässt sich nicht wechseln.
- Wird später Overclocking eingebaut, kommt es zusätzlich zum Reinheits-Tempo dazu (noch offen, wie genau).

> ✏️ **Entwurf, von dir freizugeben:** Bergbaubohrer Mk1 = **Tier 0, 4 × 4 × 6 m, Kosten 12 Eisenplatten + 6 Eisenstangen**. Steht in `DT_Buildings.csv` und lässt sich dort ändern.

## Vorkommen in der Testwelt (`Lvl_Flat`)

Gesetzt von `UFactoryResourceNodeSubsystem` beim Start, Positionen in Metern vom Startpunkt (0, 0). Vorkommen werden nicht gespeichert, sie stehen immer an derselben Stelle.

| Erz | Normal | Rein | Unrein |
|---|---|---|---|
| Eisenerz (Osten) | 25, 0 | 35, 18 | 35, −18 |
| Kupfererz (Westen) | −25, 0 | −35, 18 | −35, −18 |
| Kalkstein | 0, 30 | 0, −30 | – |

Graybox: flache Scheibe (3 m Durchmesser, 40 cm hoch) in der Erzfarbe, darüber Name und Reinheit.

## Code

| Klasse | Datei |
|---|---|
| `AFactoryResourceNode` (Vorkommen), `UFactoryResourceNodeSubsystem` (Testbelegung) | `Source/FactoryGame/Buildings/FactoryResourceNode.h` |
| Bohrer platzieren, Abbau von Hand | `UFactoryBuildComponent::UpdatePlacement`, `UpdateIdle` |

## Schnelltest

`GiveBuildCosts Drill_Mk1` → `Q` → Bergbaubohrer → auf das Eisenvorkommen 25 m östlich zielen → bauen → `E` auf den Bohrer: „Baut Eisenerz ab · 60/min“. Abbau von Hand: vor ein Vorkommen stellen, `E` halten. Eigenes Vorkommen: `SpawnNode IronOre Pure`.
