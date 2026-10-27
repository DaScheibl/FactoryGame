# 07 – Materialien & Texturen

**Workflow:** Blender + Trim-Sheets. Abnutzung, Schmutz und Tier-Farbe entstehen im **Master-Material** in Unreal, nicht in der Textur. Substance Painter kann später für einzelne Hero-Assets dazukommen. Diese bekommen dann Unique-Texturen nach demselben Packing (`_BC`, `_N`, `_ORM`).

## Master-Materialien (`MAT_Masters`)

Ablage: `Content/FactoryGame/Core/Materials/`

| Master | Zweck | Wichtige Parameter |
|---|---|---|
| `M_Master_Metal` | Stahl, Guss, Messing, Alu (unlackiert) | BaseTint, Roughness-Range, EdgeWear, Rust, Dirt |
| `M_Master_Paint` | Lackierte Verkleidungen | PaintColor, **AccentColor** (Tier), AccentMask-Kanal, EdgeWear (Lack ab → Metall darunter), Dirt |
| `M_Master_Rubber` | Gummi, Kunststoff, Bänder | Tint, Roughness, Panner-Speed (Bänder) |
| `M_Master_Glass` | Fenster, Sichtgläser (transluzent, Nanite aus) | Tint, Opacity, Dirt |
| `M_Master_Emissive` | Lampen, Displays, Kerne | EmissiveColor, Intensity, Flicker/Pulse, Status (OK/Fehler/Aus) |
| `M_Master_Item` | Alle Items (Band, Hand, Drop) | BaseColor-Texture/Tint, Roughness, Metallic, eigene schnelle Variante ohne Wear |
| `M_Master_Hologram` | Bauvorschau (`TOOL_HoloGen`) | Color (gültig/ungültig), Scanline, Fresnel |
| `M_Master_Terrain` / `M_Master_Rock` | Landschaft, Felsen (in Checkliste 06) | – |

### Geteilte Material-Funktionen

| Funktion | Inhalt |
|---|---|
| `MF_EdgeWear` | Kantenabnutzung aus Trim-Maske/Krümmung + Vertex-Color R |
| `MF_Dirt` | Schmutz von unten (World-Z-Gradient) + Vertex-Color G |
| `MF_Rust` | Rost-Overlay, Stärke pro Tier (T0 hoch → T8 aus) |
| `MF_TrimSample` | Einheitliches Sampling von Trim-Sheet `_BC/_N/_ORM` |
| `MF_DetailNormal` | Tiling-Detailnormal für große Flächen |

### Vertex-Color-Belegung

| Kanal | Bedeutung |
|---|---|
| R | Zusätzliche Kantenabnutzung |
| G | Schmutz / Ruß |
| B | Akzentfarben-Maske (1 = Tier-Akzent) |
| A | frei / reserviert |

Damit kann jede Maschine in Blender per Vertex-Paint gezielt gealtert und akzentuiert werden, ohne eigene Texturen.

## Trim-Sheets (`TEX_TrimSheets`)

Ablage: `Content/FactoryGame/Core/Textures/Trims/`

| Sheet | Inhalt | Größe |
|---|---|---|
| `T_Trim_Metal_01` | Bleche, Fugen, Schweißnähte, Riffelblech, Kanten | 4K |
| `T_Trim_Details_01` | Schrauben, Nieten, Lüftungsgitter, Griffe, Scharniere | 2K |
| `T_Trim_Panels_01` | Wartungsklappen, Warnstreifen, Typenschilder | 2K |
| `T_Decal_Atlas_01` | Decals: Warnsymbole, Nummern, Herstellerlogos pro Tier, Leckagen | 4K |

Jedes Sheet hat `_BC`, `_N`, `_ORM` (plus `_M` für Wear-Masken). Streifenhöhen in Zweierpotenzen (32, 64, 128, 256, 512 px) und horizontal kachelbar.

## Textur-Packing & Import

- `_ORM`: R = Ambient Occlusion, G = Roughness, B = Metallic.
- Normalmaps im **DirectX-Format** (Y−).
- Import-Einstellungen siehe Suffix-Tabelle in [01 Naming](01-naming.md).
