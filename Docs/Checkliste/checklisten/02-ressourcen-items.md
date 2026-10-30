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

**Item-Tabelle & Regeln (IDs, Stapelgrößen):** [Standards/10 Items & Inventar](../../Standards/10-items-inventar.md) · Daten: `Content/FactoryGame/Core/Data/DT_Items.csv`

Legende: `PT` Prototyp · `VS` Vertical Slice · `EA` Early Access · `1.0` Release

## Ressourcen-Vorkommen (Welt)

_Pro Ressource: Vorkommen-Mesh in 3 Varianten + Reinheitsstufen (unrein/normal/rein), Abbau-VFX, Abbau-SFX, Scanner-Icon, Karten-Icon, Rohstoff-Item._

- [ ] **Eisenerz** · `RES_IronOre` · `PT` — Tier 0 · _Vorkommen + Abbau stehen (Graybox, [15 Abbau](../../Standards/15-abbau.md)), Test offen; Mesh/Material fehlen_
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Kupfererz** · `RES_CopperOre` · `PT` — Tier 0 · _Vorkommen + Abbau stehen (Graybox, [15 Abbau](../../Standards/15-abbau.md)), Test offen; Mesh/Material fehlen_
  - [ ] Vorkommen-Mesh (3 Varianten)
  - [ ] Reinheitsstufen-Visual
  - [ ] Abbau-VFX & -SFX
  - [ ] Scanner- & Karten-Icon
  - [ ] Rohstoff-Item (Icon + Belt-Mesh)
- [ ] **Kalkstein** · `RES_Limestone` · `PT` — Tier 0 · _Vorkommen + Abbau stehen (Graybox, [15 Abbau](../../Standards/15-abbau.md)), Test offen; Mesh/Material fehlen_
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

- [ ] **Eisenbarren** · `ITM_IronIngot` · `PT`
- [ ] **Kupferbarren** · `ITM_CopperIngot` · `PT`
- [ ] **Eisenplatte** · `ITM_IronPlate` · `PT`
- [ ] **Eisenstange** · `ITM_IronRod` · `PT`
- [ ] **Schraube** · `ITM_Screw` · `VS`
- [ ] **Kupferdraht** · `ITM_CopperWire` · `VS`

### Tier 1 – Dampf & Kohle

- [ ] **Kabel** · `ITM_Cable` · `VS`
- [ ] **Beton** · `ITM_Concrete` · `VS`
- [ ] **Biomasse** · `ITM_Biomass` · `VS`
- [ ] **Verstärkte Eisenplatte** · `ITM_ReinforcedIronPlate` · `VS`
- [ ] **Forschungspaket I (Rot)** · `ITM_SciencePack_01` · `VS`

### Tier 2 – Stahl & Montage

- [ ] **Stahlbarren** · `ITM_SteelIngot` · `VS`
- [ ] **Stahlträger** · `ITM_SteelBeam` · `VS`
- [ ] **Stahlrohr** · `ITM_SteelPipe` · `VS`
- [ ] **Zahnrad** · `ITM_Gear` · `VS`
- [ ] **Rotor** · `ITM_Rotor` · `VS`
- [ ] **Stator** · `ITM_Stator` · `VS`
- [ ] **Motor** · `ITM_Motor` · `VS`
- [ ] **Quarzkristall** · `ITM_QuartzCrystal` · `VS`
- [ ] **Kieselerde** · `ITM_Silica` · `VS`
- [ ] **Forschungspaket II (Grün)** · `ITM_SciencePack_02` · `VS`

### Tier 3 – Öl & Chemie

- [ ] **Kunststoff** · `ITM_Plastic` · `EA`
- [ ] **Gummi** · `ITM_Rubber` · `EA`
- [ ] **Petrolkoks** · `ITM_PetroleumCoke` · `EA`
- [ ] **Schwefelsäure** · `ITM_SulfuricAcid` · `EA`
- [ ] **Treibstoff** · `ITM_Fuel` · `EA`
- [ ] **Schweröl-Rückstand** · `ITM_HeavyOilResidue` · `EA`
- [ ] **Leerer Kanister** · `ITM_EmptyCanister` · `EA`
- [ ] **Forschungspaket III (Blau)** · `ITM_SciencePack_03` · `EA`

### Tier 4 – Elektronik & Logik

- [ ] **Siliziumwafer** · `ITM_SiliconWafer` · `EA`
- [ ] **Schaltkreis** · `ITM_Circuit` · `EA`
- [ ] **Platine** · `ITM_CircuitBoard` · `EA`
- [ ] **Kondensator** · `ITM_Capacitor` · `EA`
- [ ] **Batterie** · `ITM_Battery` · `EA`
- [ ] **Computer** · `ITM_Computer` · `EA`
- [ ] **Logikmodul** · `ITM_LogicModule` · `EA`
- [ ] **Forschungspaket IV (Gelb)** · `ITM_SciencePack_04` · `EA`

### Tier 5 – Massenlogistik

- [ ] **Schienenstück** · `ITM_RailSection` · `EA`
- [ ] **Drohnen-Bausatz** · `ITM_DroneKit` · `EA`
- [ ] **Kühlsystem** · `ITM_CoolingSystem` · `EA`
- [ ] **Schwerer Modularrahmen** · `ITM_HeavyModularFrame` · `EA`
- [ ] **Forschungspaket V (Lila)** · `ITM_SciencePack_05` · `EA`

### Tier 6 – Fortgeschrittene Materialien

- [ ] **Aluminiumbarren** · `ITM_AluminumIngot` · `1.0`
- [ ] **Aluminiumblech** · `ITM_AluminumSheet` · `1.0`
- [ ] **Kühlkörper** · `ITM_HeatSink` · `1.0`
- [ ] **Verbundplatte** · `ITM_CompositePlate` · `1.0`
- [ ] **Supraleiter** · `ITM_Superconductor` · `1.0`
- [ ] **Turbomotor** · `ITM_TurboMotor` · `1.0`
- [ ] **KI-Kern** · `ITM_AICore` · `1.0`
- [ ] **Forschungspaket VI (Weiß)** · `ITM_SciencePack_06` · `1.0`

### Tier 7 – Hochenergie

- [ ] **Uranbrennstab** · `ITM_UraniumFuelRod` · `1.0`
- [ ] **Fusionszelle** · `ITM_FusionCell` · `1.0`
- [ ] **Abgereichertes Material** · `ITM_DepletedMaterial` · `1.0`
- [ ] **Xeno-Legierung** · `ITM_XenoAlloy` · `1.0`
- [ ] **Forschungspaket VII (Schwarz)** · `ITM_SciencePack_07` · `1.0`

### Tier 8 – Exodus Mars

- [ ] **Raketentreibstoff** · `ITM_RocketFuel` · `1.0`
- [ ] **Hitzeschild** · `ITM_HeatShield` · `1.0`
- [ ] **Navigationsmodul** · `ITM_NavigationModule` · `1.0`
- [ ] **Lebenserhaltungsmodul** · `ITM_LifeSupportModule` · `1.0`
- [ ] **Kolonistenkapsel** · `ITM_ColonistPod` · `1.0`
- [ ] **Raketensegment** · `ITM_RocketSegment` · `1.0`

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
