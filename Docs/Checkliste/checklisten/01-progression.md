# Progression & Tech-Tree

Factorio-Prinzip: Jede Tier wird über ein neues **Forschungspaket** freigeschaltet, das selbst produziert werden muss. Jede Stufe zwingt dich, die Fabrik umzubauen oder zu erweitern.

| Tier | Name | Neue Rohstoffe | Kern-Freischaltungen | Forschungspaket | Meilenstein |
|---|---|---|---|---|---|
| 0 | Notlandung | Eisenerz, Kupfererz, Kalkstein | Handarbeit, erste Brennstoff-Maschinen | – | `VS` |
| 1 | Dampf & Kohle | Kohle, Wasser | Erste Automatisierung, Strom | I (Rot) | `VS` |
| 2 | Stahl & Montage | Quarz | Assembler, Logistik-Grundlagen | II (Grün) | `VS` |
| 3 | Öl & Chemie | Schwefel, Rohöl, Erdgas | Fluide, Kunststoff, Gummi | III (Blau) | `EA` |
| 4 | Elektronik & Logik | – | Schaltkreise, Logiknetz, Solar/Wind | IV (Gelb) | `EA` |
| 5 | Massenlogistik | – | Züge, Drohnen, Bänder Mk3–4 | V (Lila) | `EA` |
| 6 | Fortgeschrittene Materialien | Bauxit | Aluminium, Verbundstoffe, Supraleiter | VI (Weiß) | `1.0` |
| 7 | Hochenergie | Uranerz, Xenokristall | Kernspaltung, Fusion | VII (Schwarz) | `1.0` |
| 8 | Exodus Mars | – | Raketenbau, Startkomplex, Spielende | Raketenstart | `1.0` |

## Design-Checkliste

- [ ] Vollständiger Rezeptgraph (Tabelle/Spreadsheet) – jedes Item hat Quelle und Verwendung
- [ ] Rezeptgraph als DataTables in UE importierbar
- [ ] Taktzeiten & Durchsätze balanciert (ganze Verhältnisse wie 1:2, 2:3 bevorzugen)
- [ ] Alternative Rezepte (Satisfactory-Style) – mind. 2 pro Tier ab Tier 2
- [ ] Forschungs-UI: Tech-Tree-Graph mit Abhängigkeiten
- [ ] Meilenstein-Belohnungen pro Tier (neues Gebäude, Ausrüstung, Kosmetik)
- [ ] Infinite-/Endlos-Forschung nach Tier 8 (Produktivität, Bandgeschwindigkeit)
- [ ] Freischalt-Sequenz durchgespielt: kein Softlock möglich
- [ ] Ressourcen-Verteilung auf der Karte passt zur Tier-Reihenfolge
- [ ] Zielspielzeit festgelegt (z. B. 40–60 h bis Raketenstart) und gemessen
- [ ] Tooltips/Kodex-Einträge für jedes Rezept
- [ ] Produktionsstatistik-Fenster (Factorio-Style Graphen)

## Forschungs-Assets

- [ ] **Forschungspaket-Meshes I–VII (je eigene Farbe/Form)** · `ITM_SciencePack_*` · `EA`
- [ ] **Labor-Füllstandsanzeige pro Paket** · `MAT_LabFill` · `EA`
- [ ] **Tech-Tree-Icons (ein Icon pro Technologie)** · `UI_TechIcons` · `EA`
- [ ] **Meilenstein-Fanfare (SFX + kleiner VFX)** · `SFX_Milestone` · `VS`
- [ ] **Tier-Freischalt-Sequenz (Kamera/Popup)** · `SEQ_TierUnlock` · `1.0`
