# 14 – Maschinen-Panel

Gehört zu `UI_MachinePanel`. Grundlage: Produktions-Simulation ([12 Simulation](12-simulation.md)), Rezepte ([11 Rezepte](11-rezepte.md)).

Stand: gebaut, Test im Spiel offen.

## Entscheidungen

| Thema | Entscheidung |
|---|---|
| Öffnen | **`E`** auf eine Maschine (bis 8 m), Gamepad **X**. Über dem Fadenkreuz steht „E: Schmelzofen öffnen“ |
| Inhalt | **Rezept + Puffer + Status:** Rezeptliste (nur Rezepte dieser Maschine), Ein- und Ausgangs-Puffer, Zustand und Fortschrittsbalken. Overclock kommt später mit Strom |
| Rezeptwechsel | **Alles zurück ins Inventar:** Puffer und die Zutaten eines laufenden Durchlaufs, was nicht passt, kommt in eine Abriss-Kiste. Der angefangene Fortschritt verfällt |
| Spiel | Läuft weiter, während das Panel offen ist |

## Aufbau

- **Kopf:** Name der Maschine, darunter der Zustand in der Würfelfarbe (grau = kein Rezept, gelb = wartet auf Zutaten, grün = arbeitet mit %, rot = Ausgang voll) und ein Fortschrittsbalken.
- **Puffer:** Eingänge → Ausgänge als Slots. Unter jedem Slot stehen der Item-Name und wie viele davon im Inventar liegen.
- **Rezepte:** eine Zeile pro Rezept, z. B. `Eisenbarren   1 Eisenerz → 1 Eisenbarren · 30/min`. Das aktive Rezept ist orange umrandet.
- Das Panel schließt sich, wenn die Maschine abgerissen wird oder man sich mehr als 15 m entfernt.

> ✏️ **Entwurf, von dir freizugeben:** Bedienung der Puffer per Klick statt Ziehen: **Klick auf einen Eingang** füllt ihn aus dem Inventar (so viel, wie passt), **„Zurücknehmen“** darunter legt den Inhalt zurück ins Inventar, **Klick auf einen Ausgang** nimmt alles, was ins Inventar passt. Gamepad: A auf dem Slot bzw. Button. Schließen mit `E`, `Esc` oder B. Später, wenn Truhen kommen, kann das Panel neben dem Inventar stehen, mit Ziehen wie im Inventar.

## Code

| Klasse | Datei |
|---|---|
| `UFactoryMachineWidget` (Panel) | `Source/FactoryGame/UI/FactoryMachineWidget.h` |
| `UFactoryProductionComponent::ExtractInput` (Eingang zurücknehmen) | `Source/FactoryGame/Simulation/FactoryProductionComponent.h` |
| `AFactoryGamePlayerController::OpenMachinePanel`, `GiveItemsOrCrate` | `Source/FactoryGame/FactoryGamePlayerController.h` |

## Schnelltest

`SpawnMachine Smelter` → hinschauen, `E` → Rezept „Eisenbarren“ wählen → `GiveItem IronOre 50` → Klick auf den Eingang → nach 2 s erscheint der erste Barren im Ausgang → Klick auf den Ausgang.
