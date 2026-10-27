# 01 – Naming-Convention

Schema: **`<Typ>_<ID>[_<Variante>][_<Suffix>]`**

- `<Typ>` = Unreal-Assettyp (Tabelle unten)
- `<ID>` = Asset-ID aus der Checkliste, z. B. `BLD_Drill_Mk1`, `ITM_IronPlate`
- `<Variante>` = optional, z. B. `Damaged`, `A`, `B`, `01`
- `<Suffix>` = nur bei Texturen (Kanal) und Animationen (Aktion)

Regeln:
- Nur `A–Z a–z 0–9 _`. Keine Leerzeichen, Umlaute oder Bindestriche.
- PascalCase innerhalb eines Teils: `BLD_BurnerSmelter`, nicht `BLD_burner_smelter`.
- Englische Namen im Dateinamen (Anzeigenamen kommen über Lokalisierung).
- Stufen als `Mk1`, `Mk2` …, Nummern zweistellig: `_01`, `_02`.

## Kategorie-IDs (aus der Checkliste)

| ID-Präfix | Bereich |
|---|---|
| `BLD_` | Gebäude / Maschinen |
| `ITM_` | Item / Zwischenprodukt |
| `RES_` | Rohstoff / Vorkommen |
| `LOG_` | Logistik (Bänder, Rohre, Strom, Schiene) |
| `STR_` | Bauteile (Fundamente, Wände, Treppen) |
| `VEH_` | Fahrzeug |
| `EQP_` | Ausrüstung / Werkzeug |
| `ENV_` | Umgebung |
| `UI_` · `SFX_` · `MUS_` · `VFX_` · `MAT_` | UI · Sound · Musik · Effekte · Material-Familien |

## Unreal-Typpräfixe

| Präfix | Assettyp | Beispiel |
|---|---|---|
| `SM_` | Static Mesh | `SM_BLD_Drill_Mk1` |
| `SK_` | Skeletal Mesh | `SK_EQP_Jetpack` |
| `SKEL_` | Skeleton | `SKEL_VEH_Truck` |
| `PHYS_` | Physics Asset | `PHYS_VEH_Truck` |
| `A_` | Animation Sequence | `A_BLD_Drill_Mk1_Working` |
| `ABP_` | Animation Blueprint | `ABP_VEH_Truck` |
| `M_` | Material (Master) | `M_Master_Metal` |
| `MI_` | Material Instance | `MI_BLD_Drill_Mk1` |
| `MF_` | Material Function | `MF_EdgeWear` |
| `MPC_` | Material Parameter Collection | `MPC_Global` |
| `T_` | Texture | `T_BLD_Drill_Mk1_BC` |
| `BP_` | Blueprint (Actor) | `BP_BLD_Drill_Mk1` |
| `WBP_` | Widget Blueprint | `WBP_UI_BuildMenu` |
| `DA_` | Data Asset | `DA_ITM_IronPlate` |
| `DT_` | Data Table | `DT_Recipes_Tier1` |
| `E_` / `S_` | Enum / Struct (Blueprint) | `E_ItemForm`, `S_RecipeRow` |
| `NS_` | Niagara System | `NS_BLD_Smelter_Sparks` |
| `NE_` | Niagara Emitter | `NE_Sparks_Small` |
| `SW_` | Sound Wave | `SW_SFX_Drill_Loop` |
| `MS_` | MetaSound Source | `MS_SFX_Drill` |
| `SC_` / `SA_` | Sound Class / Attenuation | `SA_Machine_Medium` |
| `IA_` / `IMC_` | Input Action / Mapping Context | `IA_Build`, `IMC_Default` |
| `L_` | Level / Map | `L_Main`, `L_LookDev_Factory` |
| `LS_` | Level Sequence | `LS_Intro` |
| `CUR_` | Curve | `CUR_PowerFalloff` |

## Textur-Suffixe

| Suffix | Inhalt | sRGB | Kompression |
|---|---|---|---|
| `_BC` | Base Color (+ ggf. Alpha) | an | Default (BC1/BC3) |
| `_N` | Normal (DirectX / Y−) | aus | Normalmap (BC5) |
| `_ORM` | R = AO, G = Roughness, B = Metallic | aus | Masks |
| `_E` | Emissive | an | Default |
| `_M` | Masken (frei belegt, im Material dokumentiert) | aus | Masks |
| `_H` | Height / Displacement | aus | Grayscale |

## Kollision & Sockets (in Blender, siehe [06](06-blender-export.md))

- `UCX_<MeshName>_01` – konvexe Kollision
- `SOCKET_<Typ>_<Richtung>_<Nr>` – z. B. `SOCKET_BeltIn_01`, `SOCKET_BeltOut_01`, `SOCKET_PipeIn_01`, `SOCKET_Power_01`, `SOCKET_Snap_N`
