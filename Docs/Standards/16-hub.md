# 16 – HUB, Werkbank & Meilensteine

Gehört zu `BLD_Hub`, `BLD_Workbench` (Herstellen von Hand) und den Meilensteinen aus [01 Progression](../Checkliste/checklisten/01-progression.md).

Stand: gebaut, Test im Spiel offen.

## Entscheidungen

| Thema | Entscheidung |
|---|---|
| HUB | **Werkbank + Lager + Meilensteine.** Jede Welt hat genau einen, er steht beim Start neben dem Startpunkt (12 m nördlich), kostenlos und nicht abreißbar. `E` öffnet ihn |
| Startproblem | **Schmelzofen und Konstruktor sind Tier 0**, und die Werkbank kann **Erz → Barren** von Hand. So startet man ohne Geschenke: Erz abbauen → Barren → Platten → erster Ofen |
| Werkbank | **Taste halten:** Rezept wählen, „Herstellen“ gedrückt halten (Maus oder Gamepad A). Solange gehalten wird, entsteht alle X Sekunden ein Teil. Loslassen setzt den Fortschritt zurück |
| Meilensteine | **Forschungspakete abgeben:** Tier 1 = 20 × Forschungspaket I, Tier 2 = 50 × Paket I + 20 × Paket II. Abgeben geht in Teilen. Gesperrte Gebäude sind im Baumenü grau („ab Tier N“), gesperrte Rezepte in Maschine und Werkbank auch |
| Lager | 24 Slots. „Lager öffnen“ zeigt das Inventarfenster mit dem HUB-Lager darüber. Shift-Klick verschiebt zwischen Lager und Inventar |

> ✏️ **Entwurf, von dir freizugeben:**
> - **Zeit von Hand = doppelte Rezeptdauer** (Eisenbarren 4 s, Eisenplatte 12 s).
> - HUB-Größe **8 × 8 × 6 m**, Platz 12 m nördlich vom Startpunkt.
> - Fertige Teile gehen ins Inventar, ist es voll, ins HUB-Lager, danach in eine Abriss-Kiste.
> - Die Meilenstein-Mengen (20 / 50 + 20) gelten bis zum Balancing.

Damit jede Stufe erreichbar ist, gibt es das Rezept für Paket N (und alles, was es braucht) schon eine Tier vorher. Deshalb sind Kabel und Paket I Tier 0, Zahnrad, Rotor, Paket II und der **Assembler Tier 1** ([11 Rezepte](11-rezepte.md)).

## Tier-Regeln

- Ein **Gebäude** ist baubar, wenn seine Tier ≤ der freigeschalteten Tier ist (Spalte `Tier` in `DT_Buildings.csv`).
- Ein **Rezept** in einer Maschine braucht seine eigene Tier und die Tier der Maschine. An der Werkbank zählt nur die Tier des Rezepts.
- Gesperrte Gebäude kommen nicht auf die Hotbar. Liegt eins von früher dort, zeigt die Hotbar-Taste nur den Hinweis.

## Speichern

Spielstand-Version 5: Position des HUB, Lager, freigeschaltete Tier und schon abgegebene Pakete. Ältere Welten bekommen beim Laden einen neuen HUB und starten bei Tier 0.

## Code

| Klasse | Datei |
|---|---|
| `AFactoryHub` (HUB, Lager, Tier, Meilensteine, Tier-Prüfungen) | `Source/FactoryGame/Buildings/FactoryHub.h` |
| `UFactoryHubWidget` (Fenster: Werkbank, Meilensteine) | `Source/FactoryGame/UI/FactoryHubWidget.h` |
| `UFactoryInventoryWidget::SetContainer` (Lager über dem Inventar) | `Source/FactoryGame/UI/FactoryInventoryWidget.h` |

## Schnelltest

Neue Welt → HUB steht vor dir → `E` → Werkbank → `GiveItem IronOre 10` → „Eisenbarren“ wählen, Herstellen gedrückt halten → alle 4 s ein Barren. Meilensteine: `GiveItem SciencePack_01 20` → Abgeben → Tier 1 freischalten. Ohne Abgeben: `UnlockTier 2`.
