# UI & HUD

Baut auf deinem CommonUI-Menüsystem (CLM_MenuSystem) auf. Komplett mit Gamepad bedienbar = Steam-Deck-tauglich.

<details>
<summary><b>Definition of Done</b> für jeden Eintrag in dieser Datei</summary>

- Funktioniert mit Maus/Tastatur **und** Gamepad
- Skaliert 1080p–4K + Steam Deck (1280×800)
- Alle Texte über Lokalisierungs-Keys
- Hover-/Klick-SFX
- Barrierefreiheit: Textgröße, farbenblind-sicher

</details>

Legende: `PT` Prototyp · `VS` Vertical Slice · `EA` Early Access · `1.0` Release

## Menüs

- [ ] **Hauptmenü + Hintergrund-Szene** · `UI_MainMenu` · `VS`
- [ ] **Neues Spiel (Weltoptionen, Seed, Bedrohung)** · `UI_NewGame` · `VS`
- [ ] **Speichern/Laden mit Vorschaubild** · `UI_SaveLoad` · `VS`
- [ ] **Einstellungen: Grafik, Audio, Steuerung, Gameplay, Barrierefreiheit** · `UI_Settings` · `VS`
- [ ] **Tastenbelegung (rebindbar)** · `UI_Keybinds` · `EA`
- [ ] **Pause-Menü** · `UI_Pause` · `PT`
- [ ] **Credits** · `UI_Credits` · `1.0`

## Ingame

- [ ] **HUD: Fadenkreuz, Hotbar, Hinweise** · `UI_HUD` · `PT`
- [ ] **Inventar** · `UI_Inventory` · `PT`
- [ ] **Baumenü mit Kategorien & Suche** · `UI_BuildMenu` · `PT`
- [ ] **Maschinen-Panel (Rezept, Status, Overclock)** · `UI_MachinePanel` · `PT`
- [ ] **Tech-Tree / Forschung** · `UI_Research` · `VS`
- [ ] **Karte + Markierungen** · `UI_Map` · `EA`
- [ ] **Produktionsstatistik (Graphen)** · `UI_Stats` · `EA`
- [ ] **Strom-Übersicht** · `UI_PowerGrid` · `EA`
- [ ] **Zugfahrplan-Editor** · `UI_TrainSchedule` · `EA`
- [ ] **Logik-Netz-Editor** · `UI_LogicEditor` · `EA`
- [ ] **Blueprint-Bibliothek** · `UI_Blueprints` · `EA`
- [ ] **Kodex / Enzyklopädie** · `UI_Codex` · `1.0`
- [ ] **Tutorial-/Hinweis-System** · `UI_Tutorial` · `EA`
- [ ] **Benachrichtigungen (Kein Strom, Band voll …)** · `UI_Notifications` · `VS`
- [ ] **Fotomodus** · `UI_PhotoMode` · `1.0`

## Grafik-Assets

- [ ] **UI-Style-Kit (Panels, Buttons, Rahmen, Schrift)** · `UI_StyleKit` · `VS`
- [ ] **Schriftarten (Lizenz geprüft, Umlaute!)** · `UI_Fonts` · `PT`
- [ ] **Icon-Set für Kategorien/Status (~80 Icons)** · `UI_IconSet` · `VS`
- [ ] **Gamepad-Glyphen (Xbox, PS, Deck)** · `UI_Glyphs` · `EA`
- [ ] **Ladebildschirme + Tipps** · `UI_Loading` · `EA`
- [ ] **Cursor** · `UI_Cursor` · `VS`
