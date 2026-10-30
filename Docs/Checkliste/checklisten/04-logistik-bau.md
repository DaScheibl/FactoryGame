# Logistik, Strom, Fahrzeuge & Bauteile

Alles, was Maschinen verbindet. Bänder und Rohre sind die meistgesehenen Assets im Spiel – hier lohnt sich Detail **und** Performance-Arbeit (Instancing/HISM).

<details>
<summary><b>Definition of Done</b> für jeden Eintrag in dieser Datei</summary>

- Modular: Spline-Mesh oder Kit-Teile, nahtlos kombinierbar
- LOD0–LOD2, Instancing-tauglich (keine Unikat-Materialien)
- Kollision & Snap-Regeln im Bau-System
- Animation/Shader (z. B. Bandlauf über UV-Panning)
- SFX + Icon + Bau-Hologramm
- Fahrzeuge zusätzlich: Rig, Fahrphysik, Innenraum/Cockpit, Licht

</details>

Legende: `PT` Prototyp · `VS` Vertical Slice · `EA` Early Access · `1.0` Release

## Förderbänder

- [ ] **Förderband Mk1 (gerade, Kurve, Rampe)** · `LOG_Belt_Mk1` · `PT` — _Funktion steht (Graybox, 3 Bau-Modi, [13 Bauen](../../Standards/13-bauen.md)); Mesh, Bandlauf-Shader, SFX fehlen_
- [ ] **Förderband Mk2** · `LOG_Belt_Mk2` · `VS`
- [ ] **Förderband Mk3** · `LOG_Belt_Mk3` · `EA`
- [ ] **Förderband Mk4** · `LOG_Belt_Mk4` · `EA`
- [ ] **Förderband Mk5** · `LOG_Belt_Mk5` · `1.0`
- [ ] **Bandstützen / Pfeiler (Stapelbar)** · `LOG_BeltPole` · `PT` — _Funktion steht (Graybox, 1–7 m per Mausrad statt Stapeln); Mesh fehlt_
- [ ] **Wandhalterung & Deckenaufhängung** · `LOG_BeltMounts` · `EA`
- [ ] **Förderlift Mk1–Mk5** · `LOG_Lift` · `VS`
- [ ] **Splitter** · `LOG_Splitter` · `VS`
- [ ] **Merger** · `LOG_Merger` · `VS`
- [ ] **Smart-/Filter-Splitter** · `LOG_SmartSplitter` · `EA`
- [ ] **Programmierbarer Splitter** · `LOG_ProgSplitter` · `EA`
- [ ] **Band-Durchführung (Wand/Boden)** · `LOG_Passthrough` · `EA`
- [ ] **Mengen-Drossel / Ventil für Bänder** · `LOG_Throttle` · `EA`

## Rohre & Fluide

- [ ] **Rohr Mk1 (gerade, Kurve, Bogen)** · `LOG_Pipe_Mk1` · `EA`
- [ ] **Rohr Mk2** · `LOG_Pipe_Mk2` · `EA`
- [ ] **Rohrstützen & Wandhalter** · `LOG_PipeSupport` · `EA`
- [ ] **Rohrpumpe** · `LOG_PipePump` · `EA`
- [ ] **Ventil** · `LOG_Valve` · `EA`
- [ ] **Rohr-Kreuzung / Verteiler** · `LOG_PipeJunction` · `EA`
- [ ] **Rohr-Sichtfenster (Fluss-Visualisierung)** · `LOG_PipeWindow` · `EA`

## Strom

- [ ] **Strommast klein** · `LOG_PowerPole_S` · `PT`
- [ ] **Strommast groß / Überlandmast** · `LOG_PowerPole_L` · `EA`
- [ ] **Wandsteckdose** · `LOG_PowerWall` · `VS`
- [ ] **Kabel-Rendering (Durchhang)** · `LOG_PowerCable` · `PT`
- [ ] **Stromschalter** · `LOG_PowerSwitch` · `VS`
- [ ] **Priorität-Schalter** · `LOG_PowerPriority` · `EA`

## Schiene

- [ ] **Gleis (gerade, Kurve, Steigung)** · `LOG_Rail` · `EA`
- [ ] **Weiche** · `LOG_RailSwitch` · `EA`
- [ ] **Blocksignal** · `LOG_RailSignal` · `EA`
- [ ] **Pfad-/Kettensignal** · `LOG_RailChainSignal` · `EA`
- [ ] **Prellbock** · `LOG_RailBuffer` · `EA`
- [ ] **Gleisstützen & Brücken** · `LOG_RailSupport` · `EA`

## Fahrzeuge

- [ ] **Lokomotive** · `VEH_Locomotive` · `EA`
- [ ] **Güterwagen** · `VEH_FreightCar` · `EA`
- [ ] **Fluidwagen** · `VEH_FluidCar` · `EA`
- [ ] **Transporter-Drohne** · `VEH_Drone` · `EA`
- [ ] **Erkundungs-Buggy** · `VEH_Buggy` · `EA`
- [ ] **Lastenfahrzeug (automatisierbar)** · `VEH_Truck` · `1.0`

## Bauteile & Struktur

- [ ] **Fundamente (1, 2, 4 m) + Rampen** · `STR_Foundation` · `PT` · [Doku](../../Standards/13-bauen.md#fundamente) — _Funktion steht (Graybox, Einrasten an Nachbarn, Ziehen bis 10 × 10, Maschinen auf Fundament-Raster, Speichern), Test offen. Tier 1 + Abriss-Regel = Entwurf; Mesh/Material fehlen_
- [ ] **Wände (glatt, Fenster, Tür, Durchführung)** · `STR_Wall` · `VS`
- [ ] **Laufstege & Treppen** · `STR_Walkway` · `VS`
- [ ] **Geländer** · `STR_Railing` · `VS`
- [ ] **Dächer** · `STR_Roof` · `EA`
- [ ] **Stahlträger-/Gerüst-Set** · `STR_Beam` · `EA`
- [ ] **Leitern** · `STR_Ladder` · `VS`
- [ ] **Pfeiler / Säulen** · `STR_Pillar` · `EA`
- [ ] **Deko: Schilder, Beleuchtung, Pflanzkübel (Solarpunk)** · `STR_Deco` · `1.0`
- [ ] **Fenster-/Glas-Set** · `STR_Glass` · `1.0`
