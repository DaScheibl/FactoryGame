# FactoryGame – Asset-Pipeline & Standards

Verbindliche Regeln für alle Assets. Gehört zu [`00-pipeline-standards.md`](../Checkliste/checklisten/00-pipeline-standards.md).
Wenn ein Standard geändert wird: hier anpassen **und** im Änderungsprotokoll unten eintragen.

| Dokument | Inhalt | Checklisten-ID |
|---|---|---|
| [01 Naming](01-naming.md) | Dateinamen, Präfixe, Suffixe | `DOC_Naming` |
| [02 Ordnerstruktur](02-ordnerstruktur.md) | Aufbau von `Content/` | `DOC_Folders` |
| [03 Maßstab & Raster](03-raster.md) | Bauraster, Port-Höhen, Einheiten | `DOC_Grid` |
| [04 Budgets, LOD & Nanite](04-budgets-lod.md) | Ziel-Hardware, Polycount, Texel, LOD-Regeln | `DOC_Budgets`, `DOC_LOD` |
| [05 Art Direction](05-art-direction.md) | Stil, Farbpalette, Formensprache pro Tier | `ART_StyleGuide`, `ART_Palette`, `ART_ShapeLanguage` |
| [06 Blender → Unreal](06-blender-export.md) | Exportpreset, Kollision, Sockets, Skript | `TOOL_BlenderExport` |
| [07 Materialien & Texturen](07-materialien.md) | Master-Materialien, Trim-Sheets, Textur-Packing | `MAT_Masters`, `TEX_TrimSheets` |
| [08 Platzhalter](08-platzhalter.md) | Genutzte Fremd-Assets, Ersatzplan | `DOC_FabPlaceholders`, `DOC_ReplacePlan` |
| [09 Git & LFS](09-git-lfs.md) | Was ins Repo gehört, LFS-Regeln | `TOOL_GitLFS` |
| [10 Items, Inventar & Hotbar](10-items-inventar.md) | Item-IDs, Stapelgrößen, Item-Tabelle, Inventar/HUD-Bedienung, Speicherformat | `SYS_Recipes`, `UI_Inventory`, `UI_HUD` |
| [11 Rezepte](11-rezepte.md) | Rezept-Tabelle, Format, Werte Tier 0–2 | `SYS_Recipes` |
| [12 Simulation](12-simulation.md) | Fester Takt, Maschinen-Ablauf, Puffer, Graybox-Maschine, Förderbänder | `SYS_Sim`, `SYS_BeltSim` |
| [13 Bauen](13-bauen.md) | Gebäude-Tabelle, Platzieren, Abriss, Steuerung, Band-Regeln, Baumenü, Testwelt | `SYS_Build`, `EQP_BuildGun`, `UI_BuildMenu` |
| [14 Maschinen-Panel](14-maschinen-panel.md) | Rezept wählen, Puffer füllen/leeren, Zustand | `UI_MachinePanel` |
| [15 Abbau](15-abbau.md) | Erz-Vorkommen, Reinheit, Bergbaubohrer, Abbau von Hand, Testwelt-Belegung | `BLD_Drill_Mk1`, `RES_*` |

Alle Konsolenbefehle zum Testen: [Cheatsheet](../Cheatsheet.md)

## Entscheidungen auf einen Blick

| Thema | Entscheidung |
|---|---|
| Naming | Unreal-Typpräfix + Checklisten-ID: `SM_BLD_Drill_Mk1` |
| Ordner | Nach Feature, ein Ordner pro Asset unter `Content/FactoryGame/` |
| Raster | Fundament 8 × 8 m, Subraster 1 m, Wandhöhe 4 m |
| Nanite | An für alles Statische, aus für Belt-Items, Skeletal, Fahrzeuge, Transluzentes |
| Ziel-Hardware | Mittelklasse: Min GTX 1070 / RX 5600, Empf. RTX 3060 / RX 6600 |
| Texeldichte | 1024 px/m (Gebäude, über Trims), 512 px/m (Items, Umgebung) |
| Stil | Semi-realistisch stilisiert |
| Farben | Feste Akzentfarbe pro Tier, Basismaterialien neutral |
| Texturing | Blender + Trim-Sheets, Abnutzung im Master-Material; Substance später optional für Hero-Assets |
| Export | FBX |
| Template-Inhalte | Bleiben als Platzhalter bis zum geplanten Ersatz |
| Item-IDs | Englisch, `ITM_IronPlate`, Forschungspakete `ITM_SciencePack_01` … `_07` |
| Inventar | 20 Slots, +10 pro Forschungsstufe bis 60 |
| Stapelgrößen | Nach Item-Klasse: 500 / 100 / 50 / 20 |
| Hotbar | 10 Bau-Shortcuts (Tasten 1–0) |
| Simulation | 20 Ticks/s fest, Aufholen max. 0,25 s, Maschinenpuffer 1 Stapel |
| Förderbänder | 1 Spur, Item-Abstand 0,5 m, Mk1–Mk5: 60/120/240/480/960 Items/min |
| Bauen | Kosten aus dem Inventar, Abriss erstattet alles (0,5 s halten, Rest in Kiste), feste Ports, 1 m Gelände-Toleranz |
| Rezepte | CSV mit festen Spalten (4 Eingänge, 2 Ausgänge), Werte Satisfactory-ähnlich |

## Änderungsprotokoll

| Datum | Änderung |
|---|---|
| 2026-10-27 | Erstfassung aller Standards |
| 2026-10-27 | 10 Items, Inventar & Hotbar ergänzt; Item-IDs in der Checkliste auf Englisch umgestellt |
| 2026-10-27 | 11 Rezepte ergänzt (Tabelle Tier 0–2); Inventar-Zeile auf 20 Slots korrigiert |
| 2026-10-27 | 12 Simulation ergänzt |
| 2026-10-28 | 12 Simulation: Förderbänder (`SYS_BeltSim`) ergänzt |
| 2026-10-28 | 13 Bauen ergänzt (Gebäude-Tabelle, Platzieren, Abriss, Band-Regeln); Rotor an der Werkbank |
| 2026-10-30 | 13 Bauen: Baumenü und Band-Kollision; 14 Maschinen-Panel ergänzt |
| 2026-10-30 | 15 Abbau ergänzt (Vorkommen, Bohrer Mk1, Abbau von Hand) |
