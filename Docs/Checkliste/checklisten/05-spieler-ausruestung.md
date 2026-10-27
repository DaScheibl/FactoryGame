# Spieler & Ausrüstung

First-Person wie Satisfactory: Arme und Werkzeuge sieht man ständig, sie müssen gut aussehen.

<details>
<summary><b>Definition of Done</b> für jeden Eintrag in dieser Datei</summary>

- Mesh LOD0–1 (1P) bzw. LOD0–3 (3P für Multiplayer)
- Rig & Animationen (Equip, Idle, Benutzen)
- VFX & SFX
- Icon + Kodex-Eintrag

</details>

Legende: `PT` Prototyp · `VS` Vertical Slice · `EA` Early Access · `1.0` Release

## Charakter

- [ ] **First-Person-Arme (Rig, Handschuhe)** · `CHR_Arms1P` · `PT` — _Platzhalter: Arme aus dem UE-First-Person-Template_
- [ ] **Third-Person-Charakter (für Co-op/Fotomodus)** · `CHR_Body3P` · `EA`
- [ ] **Anzug-Varianten / Kosmetik (3+)** · `CHR_Suits` · `1.0`
- [ ] **Helm & Visier-Overlay** · `CHR_Helmet` · `VS`
- [ ] **Animations-Set: Laufen, Springen, Klettern, Hocken, Schwimmen** · `ANM_Locomotion` · `VS`
- [ ] **Bau-/Abriss-Gesten** · `ANM_Build` · `VS`
- [ ] **Tod / Respawn** · `ANM_Death` · `EA`

## Werkzeuge

- [ ] **Bauwerkzeug (Hologramm-Projektor)** · `EQP_BuildGun` · `PT`
- [ ] **Handbohrer / Abbau-Werkzeug** · `EQP_HandDrill` · `PT`
- [ ] **Ressourcen-Scanner** · `EQP_Scanner` · `VS`
- [ ] **Kettensäge / Rodungswerkzeug** · `EQP_Chainsaw` · `EA`
- [ ] **Lackier-Werkzeug** · `EQP_Painter` · `EA`
- [ ] **Blueprint-Werkzeug** · `EQP_Blueprint` · `EA`

## Mobilität & Ausrüstung

- [ ] **Jetpack** · `EQP_Jetpack` · `EA`
- [ ] **Gleiter / Fallschirm** · `EQP_Glider` · `EA`
- [ ] **Seilrutsche (an Stromleitungen)** · `EQP_Zipline` · `EA`
- [ ] **Hover-Pack** · `EQP_Hoverpack` · `1.0`
- [ ] **Inventar-Erweiterung** · `EQP_Backpack` · `VS` — _Entschieden: nur über Forschung (+10 je Stufe, 20 → 60), kein Rucksack-Item. Code steht (`UpgradeInventory`), fehlt: Forschungs-Freischaltung_
- [ ] **Taschenlampe / Stirnlampe** · `EQP_Flashlight` · `VS`
- [ ] **Atemmaske (Gift-/Strahlungszonen)** · `EQP_GasMask` · `EA`
- [ ] **Verteidigungswaffe (nur falls Bedrohung aktiv)** · `EQP_Weapon` · `EA`
