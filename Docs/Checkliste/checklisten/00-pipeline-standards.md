# Asset-Pipeline & Standards

Bevor das erste Hero-Asset entsteht. Jeder Punkt hier spart später Dutzende Stunden Nacharbeit.

**Alle Standards:** [Docs/Standards](../../Standards/README.md)

Legende: `PT` Prototyp · `VS` Vertical Slice · `EA` Early Access · `1.0` Release

## Konventionen

- [x] **Naming-Convention (Präfixe BLD_, ITM_, MAT_, SFX_ …) dokumentiert** · `DOC_Naming` · `PT` · [Doku](../../Standards/01-naming.md)
- [x] **Ordnerstruktur in Content/ festgelegt** · `DOC_Folders` · `PT` · [Doku](../../Standards/02-ordnerstruktur.md)
- [x] **Maßstab & Raster (z. B. 1 Kachel = 8 m, Port-Höhen) festgelegt** · `DOC_Grid` · `PT` · [Doku](../../Standards/03-raster.md)
- [x] **Polycount-/Texel-Budgets pro Asset-Klasse** · `DOC_Budgets` · `PT` · [Doku](../../Standards/04-budgets-lod.md)
- [x] **LOD-Regeln (Distanzen, Reduktion %) & Nanite ja/nein pro Klasse** · `DOC_LOD` · `PT` · [Doku](../../Standards/04-budgets-lod.md)

## Art Direction

- [ ] **Style-Guide: Industrial + Solarpunk-Akzente, Referenz-Board** · `ART_StyleGuide` · `PT` · [Doku](../../Standards/05-art-direction.md) — _Referenz-Board fehlt noch_
- [x] **Farbpalette (Material-Farben, Hersteller-Akzentfarbe pro Tier)** · `ART_Palette` · `PT` · [Doku](../../Standards/05-art-direction.md)
- [ ] **Formensprache pro Tier (Tier 1 rostig/grob → Tier 8 sauber/futuristisch)** · `ART_ShapeLanguage` · `VS` · [Doku](../../Standards/05-art-direction.md) — _Entwurf, Freigabe offen_
- [ ] **3 Look-Dev-Szenen (Fabrik, Natur, Nacht) als Qualitäts-Referenz** · `ART_LookDev` · `VS` · [Doku](../../Standards/05-art-direction.md)

## Tooling

- [ ] **Blender-Exportpreset + Skript (FBX, Sockets, Kollision UCX_)** · `TOOL_BlenderExport` · `PT` · [Doku](../../Standards/06-blender-export.md) — _Skript da, Test in Blender offen_
- [ ] **Master-Materialien: Metall, Lack, Gummi, Glas, Emissive, Item** · `MAT_Masters` · `PT` · [Doku](../../Standards/07-materialien.md) — _Spezifikation fertig, Umsetzung in UE offen_
- [ ] **Trim-Sheets / Decal-Atlas für Industriedetails (Schrauben, Warnstreifen, Lüftungen)** · `TEX_TrimSheets` · `VS` · [Doku](../../Standards/07-materialien.md)
- [ ] **Icon-Rig (feste Kamera/Licht, automatischer Render aller Items)** · `TOOL_IconRig` · `VS`
- [ ] **Hologramm-Auto-Generierung aus Mesh** · `TOOL_HoloGen` · `VS`
- [ ] **Asset-Validierung (Editor-Utility: Naming, LODs, Kollision prüfen)** · `TOOL_Validator` · `EA`
- [x] **Git-LFS-Regeln für .uasset/.umap/.blend/.wav** · `TOOL_GitLFS` · `PT` · [Doku](../../Standards/09-git-lfs.md)

## Platzhalter-Strategie

- [x] **Liste, welche Fab-Assets temporär genutzt werden + Lizenz geprüft** · `DOC_FabPlaceholders` · `PT` · [Doku](../../Standards/08-platzhalter.md)
- [ ] **Plan, welche Platzhalter bis EA / 1.0 ersetzt sein müssen** · `DOC_ReplacePlan` · `VS` · [Doku](../../Standards/08-platzhalter.md) — _Entwurf, Freigabe offen_
