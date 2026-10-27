# Welt & Umgebung

Post-apokalyptische Erde, die du im Laufe des Spiels (Solarpunk-Twist) teilweise begrünst. Handgebaute Karte wie Satisfactory oder prozedural wie Factorio – **diese Entscheidung zuerst treffen**, sie verändert diese ganze Datei.

<details>
<summary><b>Definition of Done</b> für jeden Eintrag in dieser Datei</summary>

- Mesh mit LODs bzw. Nanite (Felsen/Klippen)
- Material mit Wetter-/Verschmutzungs-Varianten
- Foliage: Wind-Animation, Kollision ja/nein, abholzbar ja/nein
- Getestet in jedem Biom, in dem es vorkommt

</details>

Legende: `PT` Prototyp · `VS` Vertical Slice · `EA` Early Access · `1.0` Release

## Kartenaufbau

- [ ] **Entscheidung: handgebaut vs. prozedural vs. Hybrid** · `WLD_Decision` · `PT`
- [ ] **Weltgröße & Landschafts-Heightmap** · `WLD_Landscape` · `VS`
- [ ] **Landschafts-Master-Material (Auto-Material, Schichten)** · `MAT_Landscape` · `VS`
- [ ] **Ressourcen-Platzierung pro Biom** · `WLD_ResPlacement` · `EA`
- [ ] **Spawn-/Startgebiete (3 Schwierigkeiten)** · `WLD_Spawns` · `EA`
- [ ] **Weltpartitionierung & Streaming-Setup** · `WLD_Streaming` · `VS`

## Biome (je: Boden-Layer, Felsen-Set, Flora-Set, Ambient-Sound, Farb-Grading)

- [ ] **Biom 1: Verwüstete Ebene (Start)** · `BIO_Wasteland` · `VS`
- [ ] **Biom 2: Überwucherte Stadtruinen** · `BIO_RuinCity` · `EA`
- [ ] **Biom 3: Rostwald / Industriefriedhof** · `BIO_RustForest` · `EA`
- [ ] **Biom 4: Toxische Sümpfe** · `BIO_Swamp` · `EA`
- [ ] **Biom 5: Wüste / Salzseen** · `BIO_Desert` · `1.0`
- [ ] **Biom 6: Gebirge & Klippen** · `BIO_Mountains` · `1.0`
- [ ] **Biom 7: Kristallzone (Xenokristall, Alien-Einfluss)** · `BIO_Crystal` · `1.0`

## Umgebungs-Kits

- [ ] **Felsen-Kit (klein/mittel/groß/Klippe)** · `ENV_Rocks` · `VS`
- [ ] **Baum-/Strauch-Set (tot & lebendig)** · `ENV_Trees` · `VS`
- [ ] **Gras/Bodenbedecker** · `ENV_Grass` · `VS`
- [ ] **Ruinen-Kit (Gebäude, Straßen, Autos, Strommasten)** · `ENV_Ruins` · `EA`
- [ ] **Wasser: Seen, Flüsse, Ozeanküste** · `ENV_Water` · `VS`
- [ ] **Höhlen-Kit** · `ENV_Caves` · `1.0`
- [ ] **Abstürzende Kapseln / Lootkisten (Erkundungsbelohnung)** · `ENV_Loot` · `EA`
- [ ] **Alte Forschungsstationen (Lore-Orte)** · `ENV_LorePOI` · `1.0`
- [ ] **Begrünungs-Visual (Fabrikumfeld wird mit Fortschritt grüner)** · `ENV_Regrowth` · `1.0`

## Atmosphäre

- [ ] **Tag-/Nacht-Zyklus** · `WLD_DayNight` · `VS`
- [ ] **Himmel & Wolken** · `WLD_Sky` · `VS`
- [ ] **Wetter: Regen, Staubsturm, Nebel** · `WLD_Weather` · `EA`
- [ ] **Post-Process / Color-Grading pro Biom** · `WLD_Grading` · `EA`
- [ ] **Fauna (Ambient-Tiere, nicht feindlich)** · `ENV_Fauna` · `1.0`
