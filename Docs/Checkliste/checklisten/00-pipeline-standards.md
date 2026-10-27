# Asset-Pipeline & Standards

Bevor das erste Hero-Asset entsteht. Jeder Punkt hier spart später Dutzende Stunden Nacharbeit.

Legende: `PT` Prototyp · `VS` Vertical Slice · `EA` Early Access · `1.0` Release

## Konventionen

- [ ] **Naming-Convention (Präfixe BLD_, ITM_, MAT_, SFX_ …) dokumentiert** · `DOC_Naming` · `PT`
- [ ] **Ordnerstruktur in Content/ festgelegt** · `DOC_Folders` · `PT`
- [ ] **Maßstab & Raster (z. B. 1 Kachel = 8 m, Port-Höhen) festgelegt** · `DOC_Grid` · `PT`
- [ ] **Polycount-/Texel-Budgets pro Asset-Klasse** · `DOC_Budgets` · `PT`
- [ ] **LOD-Regeln (Distanzen, Reduktion %) & Nanite ja/nein pro Klasse** · `DOC_LOD` · `PT`

## Art Direction

- [ ] **Style-Guide: Industrial + Solarpunk-Akzente, Referenz-Board** · `ART_StyleGuide` · `PT`
- [ ] **Farbpalette (Material-Farben, Hersteller-Akzentfarbe pro Tier)** · `ART_Palette` · `PT`
- [ ] **Formensprache pro Tier (Tier 1 rostig/grob → Tier 8 sauber/futuristisch)** · `ART_ShapeLanguage` · `VS`
- [ ] **3 Look-Dev-Szenen (Fabrik, Natur, Nacht) als Qualitäts-Referenz** · `ART_LookDev` · `VS`

## Tooling

- [ ] **Blender-Exportpreset + Skript (FBX, Sockets, Kollision UCX_)** · `TOOL_BlenderExport` · `PT`
- [ ] **Master-Materialien: Metall, Lack, Gummi, Glas, Emissive, Item** · `MAT_Masters` · `PT`
- [ ] **Trim-Sheets / Decal-Atlas für Industriedetails (Schrauben, Warnstreifen, Lüftungen)** · `TEX_TrimSheets` · `VS`
- [ ] **Icon-Rig (feste Kamera/Licht, automatischer Render aller Items)** · `TOOL_IconRig` · `VS`
- [ ] **Hologramm-Auto-Generierung aus Mesh** · `TOOL_HoloGen` · `VS`
- [ ] **Asset-Validierung (Editor-Utility: Naming, LODs, Kollision prüfen)** · `TOOL_Validator` · `EA`
- [ ] **Git-LFS-Regeln für .uasset/.umap/.blend/.wav** · `TOOL_GitLFS` · `PT`

## Platzhalter-Strategie

- [ ] **Liste, welche Fab-Assets temporär genutzt werden + Lizenz geprüft** · `DOC_FabPlaceholders` · `PT`
- [ ] **Plan, welche Platzhalter bis EA / 1.0 ersetzt sein müssen** · `DOC_ReplacePlan` · `VS`
