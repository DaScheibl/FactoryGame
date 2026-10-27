# 04 – Budgets, LOD & Nanite

## Ziel-Hardware

| Stufe | Hardware | Ziel |
|---|---|---|
| Minimum | GTX 1070 / RX 5600, 16 GB RAM, SSD | 1080p, Low, 60 FPS, Lumen Software-RT |
| Empfohlen | RTX 3060 / RX 6600, 16 GB RAM | 1080p, High, 60 FPS |
| Hoch | RTX 4070 + | 1440p, Epic, Hardware-RT optional |

- Texturen-VRAM-Budget (Streaming-Pool): **~4 GB** auf „Empfohlen“.
- Gleichzeitig sichtbare Belt-Items: **~20.000** (Instancing, kein Nanite).
- Performance-Test immer in einer Fabrik-Testmap mit ≥ 300 Maschinen (`Maps/Test/L_Test_Stress`).

> ⚠️ Das Projekt hat aktuell `r.Lumen.HardwareRayTracing=True` und `r.RayTracing=True` als Standard. Für die Minimum-Hardware muss Software-Lumen funktionieren. Vor dem Prototyp in den Scalability-Settings sicherstellen, dass Low/Medium Software-RT nutzen (Checkliste 12, Performance).

## Nanite-Regeln

| Asset-Klasse | Nanite | LODs |
|---|---|---|
| Gebäude / Maschinen (statische Teile) | **an** | keine eigenen LODs; Fallback-Mesh-Qualität prüfen |
| Bewegliche Maschinenteile (Kolben, Räder) | **an**, als eigene Static-Mesh-Komponente animiert | – |
| Bauteile (Fundamente, Wände, Treppen) | **an** | – |
| Felsen, Klippen, Props | **an** | – |
| Foliage (Bäume, Gras) | Bäume an (Nanite Foliage), Gras aus | Gras: LOD0–2 |
| Items auf Bändern | **aus** | LOD0, LOD1, Imposter/Billboard |
| Items in der Hand / Welt-Drop | aus (gleiches Mesh wie Band) | wie oben |
| Fahrzeuge | aus | LOD0–3 |
| Skeletal Meshes (Spieler, Gegner) | aus | LOD0–3 |
| Glas, transluzente und Masked-Materialien | aus | LOD0–2 |

**Folge für die Gebäude-Checkliste:** Der Schritt „LOD1–LOD3 (+ Fern-Imposter)“ entfällt für Nanite-Assets. Dafür: Fallback-Mesh prüfen und Nanite-Visualisierung (Overdraw, Cluster) ansehen.

## Klassische LOD-Distanzen (Nanite aus)

| LOD | Screen Size | Reduktion |
|---|---|---|
| LOD0 | 1.0 | 100 % |
| LOD1 | 0.5 | 50 % |
| LOD2 | 0.25 | 25 % |
| LOD3 / Imposter | 0.1 | 10 % oder Billboard |
| Cull | < 0.02 | – |

## Polycount-Budgets

| Klasse | Budget LOD0 | Bemerkung |
|---|---|---|
| Item auf Band | ≤ 300 Tris | Silhouette zählt, keine Mikrodetails |
| Item (Ausrüstung in der Hand) | ≤ 15 k Tris | – |
| Gebäude klein (≤ 4 × 4 m), Nanite | ≤ 300 k Tris | Fasen/Schrauben dürfen Geometrie sein |
| Gebäude groß (≥ 8 × 8 m), Nanite | ≤ 1,5 M Tris | – |
| Bauteil (Fundament, Wand), Nanite | ≤ 20 k Tris | Wird tausendfach platziert, Speicher beachten |
| Förderband-Segment | ≤ 2 k Tris | Nanite an, Bandmaterial per Panner |
| Fahrzeug | ≤ 80 k Tris | – |
| Spielercharakter / Arme | ≤ 60 k / 20 k Tris | – |
| Felsen / Klippen, Nanite | ≤ 1 M Tris | – |

Nanite-Budgets sind Obergrenzen für Disk-Größe und Speicher, keine Ziele. Faustregel: Ein Nanite-Mesh sollte unter **20 MB** auf der Platte bleiben.

## Texel-Dichte & Texturgrößen

| Klasse | Texel-Dichte | Max. Auflösung |
|---|---|---|
| Gebäude, Bauteile (über Trim-Sheets) | **1024 px/m** | Trims 2K–4K, geteilt |
| Unique-Hero-Teile (Logos, Panels) | 1024 px/m | 2K, Ausnahme 4K mit Begründung |
| Items | 512 px/m | 1K (Belt-Items 512) |
| Umgebung / Felsen | 512 px/m + Detail-Normal | 2K |
| Fahrzeuge | 1024 px/m | 2K |
| UI-Icons | – | 256 px ingame, 512 px Master |

- Texturgrößen immer Zweierpotenzen.
- Mip-Maps an, Streaming an (außer UI).
- Für Trim-basiertes Texturing siehe [07 Materialien](07-materialien.md).

## Material-Budgets

| Wert | Grenze |
|---|---|
| Material-Slots pro Mesh | ≤ 3 (Gebäude), 1 (Items) |
| Shader-Instruktionen Master-Material | ≤ 250 (Pixel) |
| Texture Samples pro Material | ≤ 8 |
