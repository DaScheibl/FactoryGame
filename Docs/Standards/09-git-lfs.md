# 09 – Git & LFS

## Was ins Repo gehört

| Ja | Nein (`.gitignore`) |
|---|---|
| `Source/`, `Config/`, `Content/`, `*.uproject` | `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/` |
| `Docs/`, `Tools/`, `SourceArt/` (.blend, Trim-Quellen) | `.vs/`, `.idea/`, `*.sln`, `*.slnx` (neu erzeugen über *Generate Visual Studio project files*) |
| `.mcp.json`, `.vsconfig` | `.claude/settings.local.json`, `__pycache__/` |

## LFS-Regeln (`.gitattributes`)

Binärdateien laufen über Git LFS:
- Unreal: `.uasset`, `.umap`
- 3D: `.blend`, `.fbx`, `.obj`, `.glb`, `.abc`
- Bild: `.png`, `.tga`, `.exr`, `.psd`, `.jpg`, `.tif`, `.hdr`, `.kra`, `.pur` (PureRef)
- Audio/Video: `.wav`, `.ogg`, `.mp3`, `.flac`, `.mp4`
- Fonts: `.ttf`, `.otf`
- Substance (falls später genutzt): `.spp`, `.sbsar`

`.blend1` (Blender-Backups) werden ignoriert.

## Hinweise

- Neuer Rechner: erst `git lfs install`, dann klonen.
- GitHub Free: **1 GB LFS-Speicher + 1 GB Download/Monat.** Speicher im Blick behalten (*Settings → Billing*). Ab ~800 MB über ein Data Pack oder einen anderen Host nachdenken.
- `SourceArt/`-Dateien können schnell groß werden. Nur Endstände committen, keine Zwischenversionen.
- Die GitHub Action committet den Checklisten-Fortschritt selbst. Deshalb vor jedem Push `git pull`.
