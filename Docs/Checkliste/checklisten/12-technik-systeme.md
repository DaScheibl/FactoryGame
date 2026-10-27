# Technik & Systeme (nicht-Asset, aber 1.0-relevant)

Damit die Assets überhaupt etwas zum Leben haben. Kurz gehalten – eine eigene Code-Roadmap lohnt sich separat.

Legende: `PT` Prototyp · `VS` Vertical Slice · `EA` Early Access · `1.0` Release

## Kern

- [ ] **Deterministische Simulation getrennt vom Rendering (Tick-basiert)** · `SYS_Sim` · `PT` · [Doku](../../Standards/12-simulation.md) — _Umgesetzt: 20 Ticks/s, Produktions-Komponente, Graybox-Maschine + Cheats. Test im Spiel offen; Maschinen werden noch nicht gespeichert_
- [ ] **Segment-basierte Band-Simulation (nicht pro Item)** · `SYS_BeltSim` · `PT`
- [ ] **Rezept-/Item-Datenbank aus DataTables** · `SYS_Recipes` · `PT` · [Doku](../../Standards/11-rezepte.md) — _Items (CSV → DataTable) und Rezepte Tier 0–2 (25, mit Prüfung beim Laden) stehen. Rezeptwerte = Entwurf, Freigabe offen; Tier 3–8 fehlen_
- [ ] **Strom-Netz-Berechnung** · `SYS_Power` · `VS`
- [ ] **Fluid-Simulation (vereinfacht)** · `SYS_Fluids` · `EA`
- [ ] **Zug-Pathfinding & Signale** · `SYS_Trains` · `EA`
- [ ] **Drohnen-Logistik** · `SYS_Drones` · `EA`
- [ ] **Logik-Netz** · `SYS_Logic` · `EA`
- [ ] **Blueprints (Kopieren/Einfügen von Fabrik-Teilen)** · `SYS_Blueprints` · `EA`

## Infrastruktur

- [x] **Speichern/Laden + Autosave + Versionsmigration** · `SYS_Save` · `VS` · [Doku](../../Standards/10-items-inventar.md)
- [x] **Code-Dokumentation: erklärende Kommentare im gesamten eigenen C++-Code** · `SYS_CodeDocs` · `PT` — _Kommentare auf Deutsch; Template-Code (Variant_Horror/Shooter) bleibt unverändert_
- [ ] **Rendering-LOD/HISM für Gebäude-Massen** · `SYS_Instancing` · `VS`
- [ ] **Performance-Ziel: 60 FPS bei 2.000 Maschinen (Ziel-Hardware definiert)** · `SYS_PerfTarget` · `EA`
- [ ] **Co-op-Multiplayer (Entscheidung: ja/nein/später)** · `SYS_Multiplayer` · `EA`
- [ ] **Mod-Support (Entscheidung)** · `SYS_Mods` · `1.0`
- [ ] **Crash-Reporting** · `SYS_Crash` · `EA`
- [ ] **Steam-Integration (Achievements, Cloud-Saves)** · `SYS_Steam` · `EA`
- [ ] **Steam-Deck-Verifizierung** · `SYS_Deck` · `1.0`
