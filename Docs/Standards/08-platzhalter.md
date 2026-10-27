# 08 – Platzhalter

Entscheidung: Template-Inhalte **bleiben**, bis eigener Ersatz fertig ist. Gelöscht wird erst, wenn nichts Eigenes mehr darauf verweist (*Reference Viewer* prüfen).

## Genutzte Fremd-Assets (`DOC_FabPlaceholders`)

| Ordner / Asset | Herkunft | Lizenz | Genutzt für |
|---|---|---|---|
| `Content/FirstPerson/` | UE First-Person-Template | Unreal Engine EULA, in UE-Projekten frei nutzbar | Testlevel `Lvl_FirstPerson`, Spieler-BP, Arme-Anims |
| `Content/Characters/Mannequins/` | UE-Template (Manny/Quinn) | UE EULA | Spielercharakter / Arme |
| `Content/Input/` | UE-Template | UE EULA | Input Actions & Mapping Contexts |
| `Content/LevelPrototyping/` | UE-Template | UE EULA | Graybox-Meshes, Grid-Materialien für Blockouts |
| `Content/Variant_Horror/` + `Source/FactoryGame/Variant_Horror/` | UE-Template-Variante | UE EULA | aktuell nichts |
| `Content/Variant_Shooter/` + `Source/FactoryGame/Variant_Shooter/` | UE-Template-Variante | UE EULA | evtl. Referenz für Gegner-KI (Checkliste 07) |
| `Content/Weapons/` | UE-Template | UE EULA | aktuell nichts |
| _Fab-Assets_ | – | – | noch keine. Neue Einträge hier mit Link und Lizenztyp (Standard / CC-BY …) |

Regel für Fab-Assets: Lizenz vor dem Download prüfen und hier eintragen. Ablage unter `Content/ThirdParty/<Paket>/`.

## Ersatzplan (`DOC_ReplacePlan`)

> ✏️ **Entwurf, von dir freizugeben.**

| Platzhalter | Ersetzt durch | Spätestens bis |
|---|---|---|
| `LevelPrototyping` Graybox-Meshes | Eigene Blockouts → finale Meshes | bleibt als Blockout-Werkzeug, aus Release-Build ausschließen `1.0` |
| `Lvl_FirstPerson` als Startmap | `L_Main` + Hauptmenü-Map | `PT` |
| Mannequin-Arme | Eigene Spieler-Arme (Checkliste 05) | `VS` |
| Mannequin-Ganzkörper | Eigener Charakter (Checkliste 05) | `EA` |
| Template-Input (`Content/Input/`) | `IA_*` / `IMC_*` unter `FactoryGame/Core/Input/` | `VS` |
| `Variant_Horror`, `Weapons` | – (löschen) | `VS`, nach separater Rückfrage |
| `Variant_Shooter` | – (löschen, sofern nicht für Gegner-KI gebraucht) | `EA`, nach separater Rückfrage |
| Alle Fab-Platzhalter | Eigene Assets | `EA`; nur explizit gelistete Ausnahmen bis `1.0` |
