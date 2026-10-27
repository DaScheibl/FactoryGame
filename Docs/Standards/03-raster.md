# 03 – Maßstab & Raster

**Einheit:** 1 Unreal Unit (uu) = 1 cm. In Blender: Einheitensystem *Metrisch*, *Unit Scale* 1.0, Länge in Metern. Export mit FBX-Skalierung so, dass 1 m in Blender = 100 uu (siehe [06](06-blender-export.md)).

## Bauraster

| Element | Maß | uu |
|---|---|---|
| Fundament-Kachel | 8 × 8 m | 800 × 800 |
| Fundamenthöhen | 1 m · 2 m · 4 m | 100 · 200 · 400 |
| Subraster (Maschinen, Bänder, Rohre) | 1 m | 100 |
| Wandhöhe (Standard) | 4 m | 400 |
| Wandstärke | 0,5 m | 50 |
| Etagenhöhe (Fundament + Wand) | 4 m | 400 |
| Rotationsschritte | 90° (Standard), 15° fein | – |

## Maschinen-Footprints

- Footprints immer in **ganzen Metern** (Subraster), Pivot unten mittig.
- Bevorzugte Größen so wählen, dass sie in 8-m-Kacheln aufgehen: **2 × 2, 4 × 4, 4 × 8, 8 × 8, 8 × 16 m**. Andere Größen (z. B. 3 × 5) sind erlaubt, wenn sie spielerisch Sinn ergeben, müssen aber im Asset-Issue begründet werden.
- Maschinenhöhe frei, aber Ports nur auf den Standardhöhen unten.

## Ports & Anschlüsse

| Anschluss | Höhe über Fundament-Oberkante | Abstand zur Außenkante |
|---|---|---|
| Förderband Ein-/Ausgang | 1 m (100 uu) Bandoberkante | Port bündig mit Footprint-Kante |
| Rohr Ein-/Ausgang | 1,75 m (175 uu) Rohrmitte | bündig mit Footprint-Kante |
| Stromanschluss | frei am Mesh, Socket `SOCKET_Power_*` | – |

- Ports liegen immer **mittig auf einem 1-m-Rasterpunkt** der Footprint-Kante.
- Bandbreite: 1 m Rasterspur, sichtbares Band ~0,8 m.
- Rohr-Durchmesser außen: 0,5 m.

## Spieler-Maßstab

| Größe | Wert |
|---|---|
| Spielergröße | 1,8 m |
| Augenhöhe (Kamera) | 1,65 m |
| Türen / Durchgänge | min. 1 × 2,5 m |
| Laufstege | min. 1 m breit, Geländer 1,1 m |
| Treppen | Stufenhöhe 0,25 m, Steigung 2 m pro 4 m |
| Sprunghöhe (ohne Jetpack) | ~1,2 m (darf eine 1-m-Fundamentstufe überwinden) |

> Template-Charakter (Mannequin) hat ~1,8 m und passt dazu. Abweichungen bei eigenem Charakter hier anpassen.
