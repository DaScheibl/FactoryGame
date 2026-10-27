# 06 – Blender → Unreal

## Szene einrichten

- *Scene Properties → Units:* Metric, Unit Scale **1.0**, Length **Meters**.
- Modelliert wird in Metern, 1 Blender-Einheit = 1 m.
- Vorne = **−Y** in Blender (wird beim Export zu +X in Unreal).
- Pivot/Origin: **unten mittig** am Footprint (bei Items: Mittelpunkt der Unterseite).
- Transformationen vor dem Export anwenden (*Ctrl+A → All Transforms*). Skalierung muss 1.0 sein.

## Objekt-Benennung in Blender

| Objekt | Name |
|---|---|
| Render-Mesh | `SM_BLD_Drill_Mk1` (= späterer Assetname) |
| Kollision (konvex) | `UCX_SM_BLD_Drill_Mk1_01`, `_02` … |
| Socket (leeres Objekt) | `SOCKET_BeltIn_01`, `SOCKET_BeltOut_01`, `SOCKET_PipeIn_01`, `SOCKET_Power_01`, `SOCKET_Snap_N` … |
| Bewegliches Teil | `SM_BLD_Drill_Mk1_Piston` (eigenes Mesh, Pivot = Drehpunkt) |
| LODs (nur Nanite-aus) | `SM_ITM_IronPlate_LOD0`, `_LOD1` … in einer leeren Gruppe `SM_ITM_IronPlate` |

- Sockets zeigen mit ihrer **+X-Achse** (Unreal) aus der Maschine heraus, also in Bandrichtung.
- Sockets liegen exakt auf den Rasterpunkten aus [03 Raster](03-raster.md).

## UVs

- **UV0:** Textur-UVs. Trim-Assets: Flächen auf die passenden Trim-Streifen legen (siehe [07](07-materialien.md)).
- **UV1:** nur falls nötig (Unique-Masken, Decals). Lightmap-UVs sind nicht nötig (Lumen).
- Texel-Dichte mit Add-on (z. B. *Texel Density Checker*) auf 10,24 px/cm prüfen.

## FBX-Exportpreset `FactoryGame_UE`

| Einstellung | Wert |
|---|---|
| Limit to | Selected Objects |
| Object Types | Empty, Armature, Mesh |
| Apply Scalings | **FBX Units Scale** |
| Forward / Up | **−Y Forward**, **Z Up** |
| Apply Unit | an |
| Apply Transform | aus |
| Geometry → Smoothing | **Face** |
| Tangent Space | an |
| Apply Modifiers | an |
| Armature → Add Leaf Bones | **aus** |
| Bake Animation | nur bei Animationen an |

Dateiablage außerhalb von `Content/`: `SourceArt/<Bereich>/<ID>/<ID>.blend` und `.fbx` (z. B. `SourceArt/Buildings/BLD_Drill_Mk1/`).

## Unreal-Import

- Import über Interchange, Ziel ist der Asset-Ordner aus [02](02-ordnerstruktur.md).
- *Combine Meshes* an (außer bei beweglichen Teilen), *Import Collision* an (UCX), *Generate Lightmap UVs* aus.
- *Build Nanite* nach den Regeln in [04](04-budgets-lod.md).
- Materialien beim Import **nicht** erzeugen lassen. Stattdessen das passende `MI_` von Hand zuweisen.

## Export-Skript

[`Tools/Blender/factorygame_export.py`](../../Tools/Blender/factorygame_export.py) prüft Namen, Skalierung und Kollision und exportiert die Auswahl mit dem Preset oben. Installation: Blender → *Edit → Preferences → Add-ons → Install from Disk*, dann im 3D-View-Seitenpanel (N) unter **FactoryGame**.

> 🔲 Skript ist noch **nicht in Blender getestet**. Nach dem ersten erfolgreichen Export eines Test-Assets `TOOL_BlenderExport` abhaken.
