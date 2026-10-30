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

- [ ] **Hauptmenü + Hintergrund-Szene** · `UI_MainMenu` · `VS` — _Menü steht (Fortsetzen, Neue Welt, Laden, Einstellungen), Hintergrund-Szene fehlt_
- [ ] **Neues Spiel (Weltoptionen, Seed, Bedrohung)** · `UI_NewGame` · `VS`
- [ ] **Speichern/Laden mit Vorschaubild** · `UI_SaveLoad` · `VS` — _Laden, Löschen, Fortsetzen, Autosave stehen; Vorschaubild fehlt_
- [ ] **Einstellungen: Grafik, Audio, Steuerung, Gameplay, Barrierefreiheit** · `UI_Settings` · `VS` — _Grafik + Sprache stehen; Audio, Steuerung, Gameplay, Barrierefreiheit fehlen_
- [ ] **Tastenbelegung (rebindbar)** · `UI_Keybinds` · `EA`
- [ ] **Pause-Menü** · `UI_Pause` · `PT` — _Umgesetzt (Weiterspielen, Speichern, Hauptmenü), Gamepad geht; nur UI-SFX fehlen (`SFX_UI`)_
- [ ] **Credits** · `UI_Credits` · `1.0`

## Ingame

- [ ] **HUD: Fadenkreuz, Hotbar, Hinweise** · `UI_HUD` · `PT` · [Doku](../../Standards/10-items-inventar.md) — _Umgesetzt: Fadenkreuz, 10er-Hotbar, Hinweiszeile. Layout = Entwurf, Freigabe offen_
- [ ] **Inventar** · `UI_Inventory` · `PT` · [Doku](../../Standards/10-items-inventar.md) — _Umgesetzt: Ausrüstung (Kopf/Körper/Rücken/Hand) + 20er-Raster (bis 60 über Forschung), Stapel am Cursor, Rechtsklick halb/einzeln, Shift-/Strg-Klick, Drag & Drop, Sortieren, Maus + Gamepad. Tasten/Layout = Entwurf, Freigabe offen; Icons kommen mit `TOOL_IconRig`_
- [ ] **Baumenü mit Kategorien & Suche** · `UI_BuildMenu` · `PT` · [Doku](../../Standards/13-bauen.md#baumenü-schritt-3) — _Gebaut, Test im Spiel offen: Q öffnet, 6 Kategorien, Suche, Details mit Kosten, Hotbar per Ziehen / 1–0 belegen. Gamepad-Belegung = Entwurf, Freigabe offen; Icons fehlen (`UI_IconSet`)_
- [ ] **Maschinen-Panel (Rezept, Status, Overclock)** · `UI_MachinePanel` · `PT` · [Doku](../../Standards/14-maschinen-panel.md) — _Gebaut, Test offen: E öffnet, Rezeptliste, Puffer per Klick füllen/nehmen, Zustand + Fortschritt. Puffer-Bedienung = Entwurf, Freigabe offen; Overclock kommt mit Strom_
- [ ] **Tech-Tree / Forschung** · `UI_Research` · `VS`
- [ ] **Karte + Markierungen** · `UI_Map` · `EA`
- [ ] **Produktionsstatistik (Graphen)** · `UI_Stats` · `EA`
- [ ] **Strom-Übersicht** · `UI_PowerGrid` · `EA`
- [ ] **Zugfahrplan-Editor** · `UI_TrainSchedule` · `EA`
- [ ] **Logik-Netz-Editor** · `UI_LogicEditor` · `EA`
- [ ] **Blueprint-Bibliothek** · `UI_Blueprints` · `EA`
- [ ] **Kodex / Enzyklopädie** · `UI_Codex` · `1.0`
- [ ] **Tutorial-/Hinweis-System** · `UI_Tutorial` · `EA`
- [ ] **Benachrichtigungen (Kein Strom, Band voll …)** · `UI_Notifications` · `VS` · [Doku](../../Standards/10-items-inventar.md) — _System steht (+Items, Inventar voll); Strom-/Band-Meldungen kommen mit den jeweiligen Systemen_
- [ ] **Fotomodus** · `UI_PhotoMode` · `1.0`

## Grafik-Assets

- [ ] **UI-Style-Kit (Panels, Buttons, Rahmen, Schrift)** · `UI_StyleKit` · `VS`
- [ ] **Schriftarten (Lizenz geprüft, Umlaute!)** · `UI_Fonts` · `PT` — _Vorläufig Engine-Standardschrift (Roboto, Umlaute ok); eigene Schrift offen_
- [ ] **Icon-Set für Kategorien/Status (~80 Icons)** · `UI_IconSet` · `VS`
- [ ] **Gamepad-Glyphen (Xbox, PS, Deck)** · `UI_Glyphs` · `EA`
- [ ] **Ladebildschirme + Tipps** · `UI_Loading` · `EA`
- [ ] **Cursor** · `UI_Cursor` · `VS`
