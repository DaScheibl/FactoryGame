# FactoryGame – Checkliste bis 1.0

Vollständige Asset- und Release-Checkliste für **FactoryGame**:
Detailgrad und Bau-Gefühl von *Satisfactory*, Progressionstiefe von *Factorio*.
Unreal Engine 5 · Blender · Ziel: Steam Early Access → 1.0.

## Fortschritt

<!-- PROGRESS:START -->
| Bereich | Erledigt | Fortschritt |
|---|---|---|
| [Asset-Pipeline & Standards](checklisten/00-pipeline-standards.md) | 8/18 | `█████████░░░░░░░░░░░` 44.4% |
| [Progression & Tech-Tree](checklisten/01-progression.md) | 0/17 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| [Ressourcen & Items](checklisten/02-ressourcen-items.md) | 0/141 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| [Gebäude & Produktionsmaschinen](checklisten/03-gebaeude.md) | 0/643 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| [Logistik, Strom, Fahrzeuge & Bauteile](checklisten/04-logistik-bau.md) | 0/49 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| [Spieler & Ausrüstung](checklisten/05-spieler-ausruestung.md) | 0/21 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| [Welt & Umgebung](checklisten/06-welt-umgebung.md) | 0/27 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| [Bedrohung / Aliens (optional & abschaltbar)](checklisten/07-bedrohung.md) | 0/16 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| [UI & HUD](checklisten/08-ui-hud.md) | 0/28 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| [Audio](checklisten/09-audio.md) | 0/21 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| [VFX](checklisten/10-vfx.md) | 0/15 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| [Story, Text & Cinematics](checklisten/11-story-cinematics.md) | 0/8 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| [Technik & Systeme (nicht-Asset, aber 1.0-relevant)](checklisten/12-technik-systeme.md) | 1/17 | `█░░░░░░░░░░░░░░░░░░░`  5.9% |
| [Release, Marketing & Recht](checklisten/13-release-marketing.md) | 0/30 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| **Gesamt** | **9/1051** | `░░░░░░░░░░░░░░░░░░░░`  0.9% |

| Meilenstein | Assets fertig | Fortschritt |
|---|---|---|
| `PT` Prototyp | 8/42 | `████░░░░░░░░░░░░░░░░` 19.0% |
| `VS` Vertical Slice | 1/108 | `░░░░░░░░░░░░░░░░░░░░`  0.9% |
| `EA` Early Access | 0/171 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
| `1.0` Release 1.0 | 0/75 | `░░░░░░░░░░░░░░░░░░░░`  0.0% |
<!-- PROGRESS:END -->

_Wird automatisch per GitHub Action aktualisiert (oder lokal: `python Docs/Checkliste/tools/progress.py`)._

## Aufbau

| Datei | Inhalt |
|---|---|
| [00 Pipeline & Standards](checklisten/00-pipeline-standards.md) | Naming, Budgets, LOD-Regeln, Master-Materialien, Tooling – **zuerst** |
| [01 Progression](checklisten/01-progression.md) | Tier-Übersicht (0–8), Forschungspakete, Rezeptgraph, Balancing |
| [02 Ressourcen & Items](checklisten/02-ressourcen-items.md) | Rohstoffe, Vorkommen, alle Zwischenprodukte, Fluide |
| [03 Gebäude](checklisten/03-gebaeude.md) | Jede Maschine mit eigener 11-Schritte-Unterliste |
| [04 Logistik & Bau](checklisten/04-logistik-bau.md) | Bänder, Rohre, Strom, Schiene, Fahrzeuge, Fundamente/Wände |
| [05 Spieler & Ausrüstung](checklisten/05-spieler-ausruestung.md) | Arme, Charakter, Werkzeuge, Jetpack & Co. |
| [06 Welt & Umgebung](checklisten/06-welt-umgebung.md) | Karte, 7 Biome, Umgebungs-Kits, Wetter |
| [07 Bedrohung](checklisten/07-bedrohung.md) | Lärm→Welle-System, Gegner, Verteidigung – **optional** |
| [08 UI & HUD](checklisten/08-ui-hud.md) | Menüs, Ingame-UI, Style-Kit, Gamepad |
| [09 Audio](checklisten/09-audio.md) | Musik, SFX, Ambience, Mix |
| [10 VFX](checklisten/10-vfx.md) | Niagara-Effekte |
| [11 Story & Cinematics](checklisten/11-story-cinematics.md) | Story-Beats, Lore, Intro/Finale |
| [12 Technik & Systeme](checklisten/12-technik-systeme.md) | Simulation, Save/Load, Performance, Steam |
| [13 Release & Marketing](checklisten/13-release-marketing.md) | Steam-Seite, Trailer, Lokalisierung, Recht |

## Meilensteine

Jeder Eintrag trägt ein Kürzel, **wann** er fertig sein muss:

| Kürzel | Meilenstein | Bedeutung |
|---|---|---|
| `PT` | Prototyp | Bohrer → Band → Ofen → Konstruktor läuft. Graybox reicht. |
| `VS` | Vertical Slice | Tier 0–2 in finaler Qualität spielbar. Zeigt, wie das fertige Spiel aussieht. |
| `EA` | Early Access | Tier 0–5 spielbar, Platzhalter weitgehend ersetzt, Steam-Seite live. |
| `1.0` | Release | Tier 0–8 bis Raketenstart, alle Biome, Story, Polish, Achievements. |

## Arbeitsweise

- Checkboxen direkt auf GitHub anklicken (in `.md`-Dateien im Web-Editor `[ ]` → `[x]`) oder lokal in Rider/VS Code.
- Für größere Brocken ein Issue mit der Vorlage **„Asset“** anlegen und die Asset-ID (z. B. `BLD_Drill_Mk1`) in den Titel schreiben.
- Asset-IDs = Ordnername in Unreal, Dateien bekommen davor den Typpräfix (`SM_BLD_Drill_Mk1`, siehe [Naming](../Standards/01-naming.md)). Präfixe: `BLD_` Gebäude · `ITM_` Item · `RES_` Ressource · `LOG_` Logistik · `STR_` Bauteile · `VEH_` Fahrzeug · `EQP_` Ausrüstung · `ENV_` Umgebung · `UI_` · `SFX_` · `MUS_` · `VFX_` · `MAT_`.
- Reihenfolge-Tipp: immer erst **Blockout einer ganzen Tier**, dann spielen, dann Hero-Assets.

## Annahmen (bitte prüfen)

- Setting aus dem bestehenden Konzept: zerstörte Erde, Fabrik baut die Flucht zum Mars; Industrial-Look mit Solarpunk-Akzenten.
- Stilisierter Look (kein Fotorealismus), aber hoher Detailgrad bei Maschinen.
- 9 Tiers (0–8), 7 Forschungspakete, ~55 Gebäude, ~60 Items, 12 Rohstoffe, 7 Biome.
- Alien-Bedrohung als abschaltbare Option. Wird sie gestrichen, Datei 07 einfach löschen.
- Offene Grundsatzentscheidungen sind selbst als Checkbox drin (Karte handgebaut/prozedural, Multiplayer, Mods).
