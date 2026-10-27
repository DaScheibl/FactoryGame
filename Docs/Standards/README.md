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

## Änderungsprotokoll

| Datum | Änderung |
|---|---|
| 2026-10-27 | Erstfassung aller Standards |
