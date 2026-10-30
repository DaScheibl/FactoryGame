# 17 – Strom

Gehört zu `LOG_PowerPole_S`, `LOG_PowerPole_M`, `LOG_PowerPole_L`, `LOG_PowerCable`, `BLD_GenBiomass`, `ITM_Leaves`, `ITM_Wood`, `ITM_Biomass`.

Stand: **gebaut (Graybox), Test im Spiel offen.**

## Entscheidungen

| Thema | Entscheidung |
|---|---|
| Überlast | **Die Sicherung fliegt raus:** Verbraucht das Netz mehr, als erzeugt wird, fällt das ganze Netz aus, bis man es am Generator wieder einschaltet |
| Erster Generator | **Biomasse-Brenner, 30 MW.** Brennt so lange wie der Brennstoff hält: Blatt 2 s, Holz 4 s, Biomasse 8 s |
| Strom ab wann | **Von Anfang an:** Ofen, Konstruktor und Bohrer brauchen schon in Tier 0 Strom. Deshalb sind Brenner, Mast Mk1 und Kabel **Tier 0** |
| Verbrauch | Bohrer 5 MW, Ofen 4 MW, Konstruktor 4 MW, Assembler 15 MW |
| Pflanzen | **Büsche + Bäume** an festen Stellen, per Skript wie die Erzvorkommen. Busch gibt Blätter (schnell), Baum gibt Holz (langsamer). Sammeln mit `E` halten wie Erz von Hand |
| Nachwachsen | **Ja, nach 5 Minuten** Spielzeit. Später kommt eine Pflanzen-Farm, die Biomasse automatisch erzeugt |
| Biomasse | Blätter und Holz werden im **Konstruktor oder an der Werkbank** zu Biomasse gepresst |
| Masten | Kabel immer **max. 25 m**. Nur die Anschlüsse steigen: **Mk1 4 (Tier 0), Mk2 8 (Tier 2), Mk3 12 (Tier 3)**. Maschinen und Generatoren haben 1 Anschluss |
| Sicherung zurücksetzen | Mit **`E` am Generator**, Knopf „Wieder einschalten“. Geht nur, wenn genug Leistung da ist, sonst Hinweis |

> ✏️ **Entwurf, von dir freizugeben:**
> - **Brennstoff wählt man wie ein Rezept** im Brenner-Panel (Blätter / Holz / Biomasse verbrennen). Der Brenner nimmt dann nur diesen Brennstoff, auch über ein Band.
> - **Brennstoff je nach Last:** Braucht das Netz nur 12 von 30 MW, hält der Brennstoff 2,5-mal so lange. Ohne Verbraucher verbrennt nichts (Zustand „Bereit“).
> - **Verbrauch zählt nur, solange eine Maschine arbeiten könnte** (Zutaten da, Ausgang frei). Wartende Maschinen brauchen keinen Strom.
> - **Ohne Brennstoff fliegt keine Sicherung**, das Netz hat dann einfach keinen Strom. Sie fliegt, sobald Generatoren laufen und der Verbrauch höher ist.
> - Sammeln: **Busch 1,5 s → 5 Blätter, Baum 4 s → 4 Holz**, alles auf einmal, danach bleibt ein Stumpf.
> - Biomasse: **10 Blätter → 5 Biomasse** oder **4 Holz → 5 Biomasse**, je 4 s. So lohnt sich das Pressen: 10 Blätter brennen 20 s, 5 Biomasse 40 s.
> - Kosten: **Brenner** 12 Eisenplatten + 8 Eisenstangen + 4 Kabel (4 × 4 × 5 m). **Mast Mk1** 2 Eisenstangen + 1 Kabel (5 m hoch), Mk2 1 Stahlträger + 2 Kabel (6 m), Mk3 2 Stahlträger + 1 Kunststoff (7 m). **Stromkabel** 1 Kabel pro angefangene 5 m.
> - Kabel nur zwischen Mast und Maschine/Mast. Klickt man ins Leere, entsteht **kein** neuer Mast (anders als beim Band). Nach dem Verbinden geht es vom Ende aus weiter, bis man rechtsklickt.
> - Maschinen haben ihren Anschluss **oben mittig auf dem Dach**.
> - Gießerei (Tier 2) bekommt vorläufig 16 MW.

> Offen: Tier und Werte der Pflanzen-Farm.

## So funktioniert ein Netz

Alles, was über Kabel verbunden ist (Masten, Maschinen, Generatoren), ist **ein Netz**. Pro Simulations-Tick, vor den Maschinen:

1. **Leistung** = Summe der Generatoren mit Brennstoff, **Verbrauch** = Summe der Maschinen, die arbeiten könnten.
2. Verbrauch > Leistung (und Leistung > 0): **die Sicherung fliegt raus**. Das ganze Netz steht, auf dem Bildschirm kommt eine Warnung. Der Zustand wird gespeichert.
3. Sonst haben alle Maschinen Strom, und die Generatoren verbrennen ihren Brennstoff anteilig zur Last.

Maschinen ohne Kabel haben nie Strom (Zustand „Kein Strom“, blauer Würfel). Das Maschinen-Panel zeigt unter dem Zustand eine Stromzeile („Strom: 4 MW · Netz: 12 / 30 MW“). Am Generator steht dort bei rausgeflogener Sicherung der Knopf **„Wieder einschalten“**.

## Bauen

| Was | Wie |
|---|---|
| Brenner | wie eine Maschine (Hotbar oder Baumenü „Strom“), 1 Band-Eingang für Brennstoff |
| Mast | wie eine Maschine, rastet aufs 1-m-Raster |
| Kabel | „Stromkabel“ wählen → Linksklick auf Mast oder Maschine → Linksklick auf das zweite Ende. Das HUD zeigt Länge (max. 25 m) und Kosten. Rechtsklick: zurück |
| Abriss | `F`: Masten und Maschinen nehmen ihre Kabel mit (rot markiert), einzelne Kabel trifft man, wenn man bis 40 cm daneben zielt |

Fehler beim Kabel: „Auf einen Strommast oder eine Maschine zielen“, „Kein Anschluss frei“, „Schon verbunden“, „Kabel zu lang (max. 25 m)“.

## Pflanzen in der Testwelt

18 Pflanzen rund um den Start: 10 Büsche zwischen 6 und 25 m vom Startpunkt, 8 Bäume weiter draußen (30–50 m). Sie stehen fest und werden nicht gespeichert, nur welche gerade nachwachsen. Gebäude können nicht in Pflanzen gebaut werden.

## Speichern

Spielstand-Version 7: Strommasten, Kabel (verweisen über die Position auf Maschinen und Masten), gesammelte Pflanzen mit Restzeit, rausgeflogene Sicherung am Generator. In älteren Welten stehen die Maschinen nach dem Laden still, bis sie einen Brenner bekommen.

## Code

| Klasse | Datei |
|---|---|
| `UFactoryPowerSubsystem` (Netze, Last, Sicherung) | `Source/FactoryGame/Simulation/FactoryPowerSubsystem.h` |
| `AFactoryPowerPole` (Mast) | `Source/FactoryGame/Buildings/FactoryPowerPole.h` |
| `AFactoryPowerCable` (Kabel, Anschlüsse, Kosten) | `Source/FactoryGame/Buildings/FactoryPowerCable.h` |
| `AFactoryPlant`, `UFactoryPlantSubsystem` (Büsche, Bäume) | `Source/FactoryGame/Buildings/FactoryPlant.h` |
| Generator-Takt, `WantsToRun`, `NoPower`/`Standby` | `Source/FactoryGame/Simulation/FactoryProductionComponent.h` |

Neue Spalten in `DT_Buildings.csv`: `PowerMW` (Verbrauch, bei Generatoren Leistung) und `PowerConnections` (Kabel-Anschlüsse). Neue Typen: `Generator`, `PowerPole`, `PowerCable`. Brennstoff-Rezepte heißen `RCP_Burn_…` und haben kein Produkt.

## Schnelltest

Neue Welt → Busch anschauen, `E` halten → 5 Blätter. Brenner bauen (`GiveBuildCosts GenBiomass`), Mast (`GiveBuildCosts LOG_PowerPole_S`), Ofen. Stromkabel: Brenner → Mast → Ofen. Brenner öffnen, „Blätter verbrennen“ wählen, Blätter einlegen. Ofen mit Erz füllen → arbeitet, das Panel zeigt „Netz: 4 / 30 MW“. Überlast testen: 8 Öfen an einen Brenner (32 > 30 MW) → Sicherung fliegt. Ohne Strom testen: `FreePower`, `PowerInfo` zeigt alle Netze.
