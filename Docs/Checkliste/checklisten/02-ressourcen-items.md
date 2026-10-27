# Ressourcen & Items

Alle Rohstoffe und Zwischenprodukte. Items sind auf Bändern sichtbar (wie in Satisfactory), brauchen also ein echtes Mesh – aber ein billiges.

<details>
<summary><b>Definition of Done</b> für jeden Eintrag in dieser Datei</summary>

- Icon 256 px im einheitlichen Icon-Stil (Beleuchtung/Winkel aus Icon-Rig)
- Belt-Mesh ≤ 300 Tris, eigenes LOD1 als Billboard/Imposter
- Material über Item-Master-Material
- Datensatz (DataAsset): Name, Beschreibung, Stapelgröße, Fluid ja/nein
- Lokalisierungs-Keys DE/EN

</details>

Legende: `PT` Prototyp · `VS` Vertical Slice · `EA` Early Access · `1.0` Release

## Ressourcen-Vorkommen (Welt)

_Pro Ressource: Vorkommen-Mesh in 3 Varianten + Reinheitsstufen (unrein/normal/rein), Abbau-VFX, Abbau-SFX, Scanner-Icon, Karten-Icon, Rohstoff-Item._

- [ ] **Eisenerz** · `RES_IronOre` · `PT` — Tier 0
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Kupfererz** · `RES_CopperOre` · `PT` — Tier 0
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Kalkstein** · `RES_Limestone` · `PT` — Tier 0
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Kohle** · `RES_Coal` · `VS` — Tier 1
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Wasser** · `RES_Water` · `VS` — Tier 1
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Quarz** · `RES_Quartz` · `VS` — Tier 2
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Schwefel** · `RES_Sulfur` · `EA` — Tier 3
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Rohöl** · `RES_CrudeOil` · `EA` — Tier 3
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Erdgas** · `RES_NatGas` · `EA` — Tier 3
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Bauxit** · `RES_Bauxite` · `1.0` — Tier 6
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Uranerz** · `RES_Uranium` · `1.0` — Tier 7
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Xenokristall (Alien-Material)** · `RES_Xenocrystal` · `1.0` — Tier 7
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)

## Zwischen- & Endprodukte

### Tier 0 – Notlandung

- [ ] **Eisenbarren** · `ITM_Eisenbarren` · `PT`
- [ ] **Kupferbarren** · `ITM_Kupferbarren` · `PT`
- [ ] **Eisenplatte** · `ITM_Eisenplatte` · `PT`
- [ ] **Eisenstange** · `ITM_Eisenstange` · `PT`
- [ ] **Schraube** · `ITM_Schraube` · `VS`
- [ ] **Kupferdraht** · `ITM_Kupferdraht` · `VS`

### Tier 1 – Dampf & Kohle

- [ ] **Kabel** · `ITM_Kabel` · `VS`
- [ ] **Beton** · `ITM_Beton` · `VS`
- [ ] **Biomasse** · `ITM_Biomasse` · `VS`
- [ ] **Verstärkte Eisenplatte** · `ITM_VerstärkteEisenplatte` · `VS`
- [ ] **Forschungspaket I (Rot)** · `ITM_ForschungspaketIRot` · `VS`

### Tier 2 – Stahl & Montage

- [ ] **Stahlbarren** · `ITM_Stahlbarren` · `VS`
- [ ] **Stahlträger** · `ITM_Stahlträger` · `VS`
- [ ] **Stahlrohr** · `ITM_Stahlrohr` · `VS`
- [ ] **Zahnrad** · `ITM_Zahnrad` · `VS`
- [ ] **Rotor** · `ITM_Rotor` · `VS`
- [ ] **Stator** · `ITM_Stator` · `VS`
- [ ] **Motor** · `ITM_Motor` · `VS`
- [ ] **Quarzkristall** · `ITM_Quarzkristall` · `VS`
- [ ] **Kieselerde** · `ITM_Kieselerde` · `VS`
- [ ] **Forschungspaket II (Grün)** · `ITM_ForschungspaketIiGrün` · `VS`

### Tier 3 – Öl & Chemie

- [ ] **Kunststoff** · `ITM_Kunststoff` · `EA`
- [ ] **Gummi** · `ITM_Gummi` · `EA`
- [ ] **Petrolkoks** · `ITM_Petrolkoks` · `EA`
- [ ] **Schwefelsäure** · `ITM_Schwefelsäure` · `EA`
- [ ] **Treibstoff** · `ITM_Treibstoff` · `EA`
- [ ] **Schweröl-Rückstand** · `ITM_SchwerölRückstand` · `EA`
- [ ] **Leerer Kanister** · `ITM_LeererKanister` · `EA`
- [ ] **Forschungspaket III (Blau)** · `ITM_ForschungspaketIiiBlau` · `EA`

### Tier 4 – Elektronik & Logik

- [ ] **Siliziumwafer** · `ITM_Siliziumwafer` · `EA`
- [ ] **Schaltkreis** · `ITM_Schaltkreis` · `EA`
- [ ] **Platine** · `ITM_Platine` · `EA`
- [ ] **Kondensator** · `ITM_Kondensator` · `EA`
- [ ] **Batterie** · `ITM_Batterie` · `EA`
- [ ] **Computer** · `ITM_Computer` · `EA`
- [ ] **Logikmodul** · `ITM_Logikmodul` · `EA`
- [ ] **Forschungspaket IV (Gelb)** · `ITM_ForschungspaketIvGelb` · `EA`

### Tier 5 – Massenlogistik

- [ ] **Schienenstück** · `ITM_Schienenstück` · `EA`
- [ ] **Drohnen-Bausatz** · `ITM_DrohnenBausatz` · `EA`
- [ ] **Kühlsystem** · `ITM_Kühlsystem` · `EA`
- [ ] **Schwerer Modularrahmen** · `ITM_SchwererModularrahmen` · `EA`
- [ ] **Forschungspaket V (Lila)** · `ITM_ForschungspaketVLila` · `EA`

### Tier 6 – Fortgeschrittene Materialien

- [ ] **Aluminiumbarren** · `ITM_Aluminiumbarren` · `1.0`
- [ ] **Aluminiumblech** · `ITM_Aluminiumblech` · `1.0`
- [ ] **Kühlkörper** · `ITM_Kühlkörper` · `1.0`
- [ ] **Verbundplatte** · `ITM_Verbundplatte` · `1.0`
- [ ] **Supraleiter** · `ITM_Supraleiter` · `1.0`
- [ ] **Turbomotor** · `ITM_Turbomotor` · `1.0`
- [ ] **KI-Kern** · `ITM_KiKern` · `1.0`
- [ ] **Forschungspaket VI (Weiß)** · `ITM_ForschungspaketViWeiß` · `1.0`

### Tier 7 – Hochenergie

- [ ] **Uranbrennstab** · `ITM_Uranbrennstab` · `1.0`
- [ ] **Fusionszelle** · `ITM_Fusionszelle` · `1.0`
- [ ] **Abgereichertes Material** · `ITM_AbgereichertesMaterial` · `1.0`
- [ ] **Xeno-Legierung** · `ITM_XenoLegierung` · `1.0`
- [ ] **Forschungspaket VII (Schwarz)** · `ITM_ForschungspaketViiSchwarz` · `1.0`

### Tier 8 – Exodus Mars

- [ ] **Raketentreibstoff** · `ITM_Raketentreibstoff` · `1.0`
- [ ] **Hitzeschild** · `ITM_Hitzeschild` · `1.0`
- [ ] **Navigationsmodul** · `ITM_Navigationsmodul` · `1.0`
- [ ] **Lebenserhaltungsmodul** · `ITM_Lebenserhaltungsmodul` · `1.0`
- [ ] **Kolonistenkapsel** · `ITM_Kolonistenkapsel` · `1.0`
- [ ] **Raketensegment** · `ITM_Raketensegment` · `1.0`

## Fluide

- [ ] **Wasser (Rohr-/Tank-Visual)** · `FLD_Water` · `VS`
- [ ] **Rohöl** · `FLD_Crude` · `EA`
- [ ] **Schweröl** · `FLD_HeavyOil` · `EA`
- [ ] **Treibstoff** · `FLD_Fuel` · `EA`
- [ ] **Schwefelsäure** · `FLD_Acid` · `EA`
- [ ] **Erdgas** · `FLD_Gas` · `EA`
- [ ] **Stickstoff / Kühlmittel** · `FLD_Coolant` · `1.0`
- [ ] **Raketentreibstoff** · `FLD_RocketFuel` · `1.0`

_Pro Fluid: Farbe/Material im Rohr-Sichtfenster, Tank-Füllstand-Material, Kanister-Variante, Icon._
