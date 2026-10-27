# 05 – Art Direction

## Stil

**Semi-realistisch stilisiert.**

- Echte PBR-Werte (Metall ist metallisch, Gummi matt), aber leicht **überzeichnete Proportionen**: dickere Schrauben, breitere Fasen, gut lesbare Rohre und Kabel.
- **Lesbarkeit vor Realismus:** Funktion einer Maschine aus 20 m erkennbar (Eingänge, Ausgänge, was sie tut).
- **Kanten-Fasen auf allem** (min. 1–2 cm). Fängt Licht und macht den Look weich.
- Leicht gesättigte Farben, keine reinen Schwarz- und Weißwerte (BaseColor zwischen sRGB 30 und 240).
- Abnutzung **gezielt**: Kanten, Griffe, Bodenkontakt. Keine flächige Schmutz-Überlagerung.
- Setting: zerstörte Erde, die Fabrik baut die Flucht zum Mars. **Industrial-Basis**, **Solarpunk-Akzente** (Pflanzen, warmes Licht, Glas, Solarflächen) nehmen mit dem Fortschritt zu.

## Referenz-Board

> 🔲 **Offen:** Referenzbilder sammeln und in `Docs/Standards/references/` ablegen (oder PureRef-Datei verlinken).
> Vorschlag für Kategorien: Industrie-Maschinen (Bergbau, Stahlwerk), Solarpunk-Architektur, stilisierte Hard-Surface-Spiele, Lichtstimmung Tag/Nacht, verlassene Landschaften.

## Farbpalette

### Basismaterialien (neutral, für alle Tiers)

| Rolle | Farbe | Hex | Einsatz |
|---|---|---|---|
| Stahl roh | Mittelgrau | `#8A8D8F` | Rahmen, Träger |
| Stahl dunkel | Anthrazit | `#3C3F42` | Gehäuse, Sockel |
| Lack Basis | Warmgrau | `#B5B0A6` | Große Verkleidungen |
| Gummi | Fast schwarz | `#232323` | Bänder, Dichtungen, Reifen |
| Warnung | Signalgelb/Schwarz | `#E6B422` / `#1E1E1E` | Warnstreifen, Kanten |
| Gefahr | Signalrot | `#C0392B` | Not-Aus, Hitze |
| Licht/Status OK | Warmweiß | `#FFE2B0` | Lampen, Displays |

### Tier-Akzentfarben

Die Akzentfarbe sitzt auf **Lackstreifen, Herstellerplaketten, Statuslampen und dem UI-Tag** des Tiers. Maximal ~15 % der sichtbaren Fläche einer Maschine.

| Tier | Thema | Akzent | Hex |
|---|---|---|---|
| T0 | Notlandung | Rostorange | `#C8642A` |
| T1 | Dampf & Kohle | Messing | `#B8913A` |
| T2 | Stahl & Montage | Signalgelb | `#E6B422` |
| T3 | Öl & Chemie | Petrol | `#1F7A80` |
| T4 | Elektronik & Logik | Cyan | `#2BB3D9` |
| T5 | Massenlogistik | Signalgrün | `#5DB043` |
| T6 | Fortgeschrittene Materialien | Violett | `#7A4FC2` |
| T7 | Hochenergie | Magenta | `#D2367A` |
| T8 | Exodus Mars | Weiß / Rot | `#EDEDED` / `#C0392B` |

- Umsetzung: Parameter `AccentColor` im Master-Material. Die Werte kommen aus einer Data-Table `DT_TierColors` bzw. `MPC_Global`, nicht hart in Instanzen eintragen.
- T2-Akzent und Warnfarbe sind beide Signalgelb. Warnstreifen deshalb immer **gestreift** mit Schwarz, der Tier-Akzent immer **flächig**.

## Formensprache pro Tier

> ✏️ **Entwurf, von dir freizugeben.** Nach Freigabe diesen Hinweis entfernen und `ART_ShapeLanguage` abhaken.

| Tier | Formen | Material & Oberfläche | Details | Solarpunk-Anteil |
|---|---|---|---|---|
| T0 Notlandung | Zusammengeschweißt, asymmetrisch, improvisiert. Teile des abgestürzten Schiffs wiederverwendet. | Roher Stahl, Rost, Schweißnähte, Planen | Klebeband, Drahtbinder, sichtbare Bolzen | – |
| T1 Dampf & Kohle | Schwer, rund, gedrungen. Kessel, Zylinder, große Räder. | Gusseisen, Messing, Ruß | Nieten, Manometer, Dampfventile | – |
| T2 Stahl & Montage | Kantig, robust, symmetrisch. Klassische Industrie. | Lackierter Stahl, erste Abnutzung | Wartungsklappen, Warnstreifen, Leitern | – |
| T3 Öl & Chemie | Rohre, Tanks, Türme, vertikal. | Lack mit Chemie-Flecken, Edelstahl | Rohrbündel, Ventilräder, Sichtfenster | – |
| T4 Elektronik & Logik | Kompakter, modulare Panels, präzisere Fugen. | Pulverbeschichtung, Kunststoff | Kabelkanäle, Displays, Lüftungsgitter | Erste Pflanzenkübel an Laufstegen |
| T5 Massenlogistik | Groß, wiederholbar, stapelbar. Effizienz-Ästhetik. | Sauberer Lack, wenig Abnutzung | Container-Raster, Signal-Lichter | Solarpaneele auf Dächern |
| T6 Fortgeschrittene Materialien | Weiche Radien, Verbundstoffe, schlanker. | Carbon, Aluminium, Glas | Verdeckte Schrauben, beleuchtete Fugen | Glas-Gewächshaus-Elemente |
| T7 Hochenergie | Ringförmig, radial, Energiefelder. | Keramik, Glas, Emissive | Kühlrippen, leuchtende Kerne | Begrünte Kühltürme |
| T8 Exodus Mars | Sauber, futuristisch, aerodynamisch. Raumfahrt-Look. | Weißer Lack, Hitzeschild-Kacheln | Minimal, präzise, Raketen-Markierungen | Integrierte Gärten, Licht |

**Leitlinie:** Mit steigendem Tier werden Formen **sauberer, runder und heller** und die Abnutzung geringer. Die Natur kehrt zurück: Der Solarpunk-Anteil steigt ab T4.

## Look-Dev-Szenen (`ART_LookDev`, VS)

Geplant in `Content/FactoryGame/Maps/LookDev/`:
- `L_LookDev_Factory`: Innenhalle mit T1/T2-Maschinen, Bänder, Kunstlicht
- `L_LookDev_Nature`: Außenbereich, ein Biom, Tageslicht, Fabrikrand
- `L_LookDev_Night`: Fabrik bei Nacht, Emissives und Lampen
