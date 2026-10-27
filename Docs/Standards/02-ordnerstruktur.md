# 02 – Ordnerstruktur in `Content/`

Prinzip: **nach Feature**. Jedes Asset mit Checklisten-ID bekommt einen eigenen Ordner, in dem alles liegt, was nur dieses Asset nutzt (Mesh, Materialinstanzen, Texturen, Blueprint, Animationen, Sounds, VFX).
Was mehrere Assets teilen, liegt in `Core/`.

```
Content/
└── FactoryGame/
    ├── Core/                    # Geteiltes
    │   ├── Materials/           # M_Master_*, MF_*, MPC_*
    │   ├── Textures/            # Trim-Sheets, Decal-Atlas, Detail-Normals, Noise
    │   ├── Data/                # DT_Recipes_*, DA_Tier_*, Enums/Structs
    │   ├── Audio/               # SC_*, SA_*, Submixe, geteilte MetaSounds
    │   ├── VFX/                 # geteilte Niagara-Emitter
    │   └── Blueprints/          # Basis-Klassen (BP_BuildingBase …)
    ├── Buildings/
    │   └── BLD_Drill_Mk1/
    │       ├── SM_BLD_Drill_Mk1
    │       ├── MI_BLD_Drill_Mk1
    │       ├── T_BLD_Drill_Mk1_BC / _N / _ORM   (nur wenn Unique-Texturen)
    │       ├── A_BLD_Drill_Mk1_Working
    │       ├── NS_BLD_Drill_Mk1_Dust
    │       ├── MS_SFX_Drill_Mk1
    │       └── BP_BLD_Drill_Mk1
    ├── Items/ITM_<Name>/
    ├── Resources/RES_<Name>/
    ├── Logistics/LOG_<Name>/
    ├── Structures/STR_<Name>/
    ├── Vehicles/VEH_<Name>/
    ├── Equipment/EQP_<Name>/
    ├── Player/                  # Arme, Charakter, Kamera
    ├── Environment/
    │   ├── Biomes/<Biom>/
    │   └── ENV_<Name>/
    ├── Threat/                  # optional, siehe Checkliste 07
    ├── UI/
    │   ├── Icons/               # Auto-gerenderte Item-Icons
    │   ├── Widgets/             # WBP_UI_*
    │   └── Style/               # Fonts, Farben, Brushes
    ├── Audio/Music/             # MUS_*
    ├── Cinematics/              # LS_*
    └── Maps/
        ├── L_Main
        ├── LookDev/             # L_LookDev_Factory / _Nature / _Night
        └── Test/                # Testlevel, Performance-Maps
```

## Regeln

- **Alles Eigene liegt unter `Content/FactoryGame/`.** Template-Ordner (`FirstPerson/`, `Characters/`, `LevelPrototyping/`, `Variant_*`, `Weapons/`, `Input/`) und Fab-Assets bleiben getrennt, damit sie später sauber entfernt werden können (siehe [08 Platzhalter](08-platzhalter.md)).
- Fab-/Marktplatz-Assets in `Content/ThirdParty/<Paketname>/`, nie umbenennen und nie in eigene Ordner mischen.
- `Content/Developers/<Name>/` ist privater Testbereich und wird nie von eigenen Assets referenziert.
- Ordnernamen englisch, PascalCase, keine Leerzeichen.
- Asset-Ordner tragen exakt die Checklisten-ID (`BLD_Drill_Mk1`), damit Suche und Validierung funktionieren.
- Beim Verschieben in Unreal danach **Fix Up Redirectors** auf dem Ordner ausführen.
