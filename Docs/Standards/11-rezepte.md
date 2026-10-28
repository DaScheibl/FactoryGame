# 11 – Rezepte

Gehört zu `SYS_Recipes` und zum Rezeptgraphen in [01 Progression](../Checkliste/checklisten/01-progression.md).

## Entscheidungen

| Thema | Entscheidung |
|---|---|
| Tabellenformat | **Feste Spalten**: bis zu 4 Eingänge, 2 Ausgänge (Produkt + Nebenprodukt), je als Spaltenpaar ID/Menge |
| Rezept-IDs | `RCP_<Produkt>`, z. B. `RCP_IronPlate`. Alternativrezepte später `RCP_Alt_<Name>` |
| Balancing | **Satisfactory-ähnlich**: Taktzeit in Sekunden pro Durchlauf, ganze Mengen, möglichst glatte Werte pro Minute |
| Umfang (jetzt) | Tier 0–2 (alles bis zum Vertical Slice) |

## Rezept-Tabelle

**Datei:** `Content/FactoryGame/Core/Data/DT_Recipes.csv`. Gleiches Prinzip wie `DT_Items.csv`: wird beim Start gelesen, kein Import im Editor nötig.

| Spalte | Inhalt |
|---|---|
| `---` | Rezept-ID (Zeilenname) |
| `NameEN` / `NameDE` | Nur für Alternativrezepte. Leer → das Rezept heißt wie sein erstes Produkt |
| `Tier` | 0–8, Freischalt-Tier |
| `Building` | Gebäude, das das Rezept ausführt (`BLD_Constructor` …). `BLD_Workbench` = nur von Hand |
| `Workbench` | `True`: kann zusätzlich an der Werkbank von Hand gebaut werden |
| `Duration` | Sekunden pro Durchlauf bei 100 % |
| `Input1`…`Input4`, `Input1Amount`… | Zutaten pro Durchlauf. Unbenutzte Spalten: leer und `0` |
| `Output1`, `Output1Amount` | Produkt |
| `Output2`, `Output2Amount` | Nebenprodukt (ab Tier 3, z. B. Raffinerie) |

Beim Laden prüft das Spiel jedes Rezept und schreibt Fehler ins Output-Log (`LogFactoryGame`): unbekannte Item-IDs, ID ohne Menge, doppelte Zutaten, kein Produkt, Fluide an der Werkbank, Zutat aus einer höheren Tier als das Rezept.

## Rezepte Tier 0–2

> ✏️ **Entwurf, von dir freizugeben:** alle Mengen und Taktzeiten unten.

| Tier | Rezept | Gebäude | Werkbank | Dauer | Eingang | Ausgang | pro Minute (Ausgang) |
|---|---|---|---|---|---|---|---|
| 0 | Eisenbarren | Schmelzofen | – | 2 s | 1 Eisenerz | 1 Eisenbarren | 30 |
| 0 | Kupferbarren | Schmelzofen | – | 2 s | 1 Kupfererz | 1 Kupferbarren | 30 |
| 0 | Eisenplatte | Konstruktor | ✔ | 6 s | 3 Eisenbarren | 2 Eisenplatten | 20 |
| 0 | Eisenstange | Konstruktor | ✔ | 4 s | 1 Eisenbarren | 1 Eisenstange | 15 |
| 0 | Schraube | Konstruktor | ✔ | 6 s | 1 Eisenstange | 4 Schrauben | 40 |
| 0 | Kupferdraht | Konstruktor | ✔ | 4 s | 1 Kupferbarren | 2 Kupferdraht | 30 |
| 0 | Handbohrer | nur Werkbank | ✔ | 10 s | 5 Eisenplatten, 5 Eisenstangen | 1 Handbohrer | – |
| 1 | Kabel | Konstruktor | ✔ | 2 s | 2 Kupferdraht | 1 Kabel | 30 |
| 1 | Beton | Konstruktor | ✔ | 4 s | 3 Kalkstein | 1 Beton | 15 |
| 1 | Verstärkte Eisenplatte | Assembler | ✔ | 12 s | 6 Eisenplatten, 12 Schrauben | 1 Verst. Eisenplatte | 5 |
| 1 | Forschungspaket I | Konstruktor | ✔ | 8 s | 2 Kabel | 1 Forschungspaket I | 7,5 |
| 1 | Stirnlampe | nur Werkbank | ✔ | 8 s | 2 Eisenplatten, 5 Kabel | 1 Stirnlampe | – |
| 1 | Ressourcen-Scanner | nur Werkbank | ✔ | 10 s | 4 Eisenplatten, 10 Kabel | 1 Scanner | – |
| 2 | Stahlbarren | Gießerei | – | 4 s | 3 Eisenerz, 3 Kohle | 3 Stahlbarren | 45 |
| 2 | Stahlträger | Konstruktor | – | 4 s | 4 Stahlbarren | 1 Stahlträger | 15 |
| 2 | Stahlrohr | Konstruktor | – | 6 s | 3 Stahlbarren | 2 Stahlrohre | 20 |
| 2 | Zahnrad | Konstruktor | – | 4 s | 2 Eisenplatten | 1 Zahnrad | 15 |
| 2 | Rotor | Assembler | ✔ | 15 s | 5 Eisenstangen, 25 Schrauben | 1 Rotor | 4 |
| 2 | Stator | Assembler | – | 12 s | 3 Stahlrohre, 8 Kupferdraht | 1 Stator | 5 |
| 2 | Motor | Assembler | – | 12 s | 2 Rotoren, 2 Statoren | 1 Motor | 5 |
| 2 | Quarzkristall | Konstruktor | – | 8 s | 5 Rohquarz | 3 Quarzkristalle | 22,5 |
| 2 | Kieselerde | Konstruktor | – | 8 s | 3 Rohquarz | 5 Kieselerde | 37,5 |
| 2 | Forschungspaket II | Assembler | – | 10 s | 1 Zahnrad, 2 Kabel | 1 Forschungspaket II | 6 |
| 2 | Kettensäge | Ausrüstungswerkstatt | – | 20 s | 5 Verst. Eisenplatten, 20 Eisenstangen, 100 Schrauben | 1 Kettensäge | – |
| 2 | Gleiter | Ausrüstungswerkstatt | – | 20 s | 10 Stahlrohre, 20 Kabel, 20 Eisenplatten | 1 Gleiter | – |

Ohne Rezept (werden abgebaut bzw. gesammelt): Eisenerz, Kupfererz, Kalkstein, Kohle, Rohquarz, Biomasse.

## Offene Punkte

> ✏️ **Entwurf, von dir freizugeben:**
> - **Forschungspaket I** kommt aus dem **Konstruktor** (1 Eingang: 2 Kabel), damit es schon in Tier 1 automatisiert werden kann. Der Assembler kommt erst mit Tier 2.
> - **Verstärkte Eisenplatte** ist laut Item-Tabelle Tier 1, der Assembler aber Tier 2. In Tier 1 gibt es sie deshalb nur an der Werkbank.
> - **Brennstoff-Schmelze** (`BLD_BurnerSmelter`, Tier 0) soll dieselben Rezepte wie der Schmelzofen nutzen, nur langsamer und mit Brennstoff. Das kommt mit dem Gebäude-System. Bis dahin stehen die Barren-Rezepte nur beim Schmelzofen.
> - An der Werkbank gilt `Duration` als Handarbeitszeit (z. B. Taste gedrückt halten).

## Test-Befehle (Konsole)

Alle Befehle auf einen Blick: [Cheatsheet](../Cheatsheet.md)

| Befehl | Wirkung |
|---|---|
| `ListRecipes` | Alle Rezepte mit Mengen und Werten pro Minute ins Output-Log |
| `ListRecipes Constructor` | Nur Rezepte eines Gebäudes (`BLD_` kann wegfallen; `Workbench` zeigt alles, was von Hand geht) |
| `ListRecipes IronPlate` | Rezepte, die das Item herstellen oder verbrauchen |
| `ReloadRecipes` | `DT_Recipes.csv` neu laden und prüfen |
| `ReloadItems` | Lädt die Items neu und prüft danach auch die Rezepte |

## Code

| Klasse | Datei |
|---|---|
| `FFactoryRecipeDefinition` | `Source/FactoryGame/Items/FactoryRecipeTypes.h` |
| `UFactoryRecipeDatabase` (GameInstance-Subsystem, lädt nach der Item-Datenbank) | `Source/FactoryGame/Items/FactoryRecipeDatabase.h` |
