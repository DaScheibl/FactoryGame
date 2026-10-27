# Audio

In Fabrikspielen trägt Audio enorm: 100 Maschinen dürfen nicht nervig klingen. Plane früh ein Mixing-/Ducking-Konzept.

<details>
<summary><b>Definition of Done</b> für jeden Eintrag in dieser Datei</summary>

- Format WAV 48 kHz, normalisiert
- MetaSound/SoundCue mit Variation (Pitch/Random)
- Attenuation & Concurrency-Limit gesetzt
- Im Mix mit vielen gleichzeitigen Maschinen getestet

</details>

Legende: `PT` Prototyp · `VS` Vertical Slice · `EA` Early Access · `1.0` Release

## Musik

- [ ] **Hauptmenü-Thema** · `MUS_MainMenu` · `VS`
- [ ] **Explorations-Tracks (6–10)** · `MUS_Explore` · `EA`
- [ ] **Fabrik-/Bau-Tracks (adaptiv, Intensität nach Fabrikgröße)** · `MUS_Build` · `EA`
- [ ] **Tier-Freischalt-Stinger (9×)** · `MUS_Stingers` · `1.0`
- [ ] **Wellen-/Gefahr-Musik (falls Bedrohung)** · `MUS_Threat` · `EA`
- [ ] **Finale: Raketenstart** · `MUS_Finale` · `1.0`

## SFX

- [ ] **Maschinen-Loops (siehe Gebäude-Datei, ~55 Stück)** · `SFX_Machines` · `EA`
- [ ] **Band-Loop pro Mk + Item-Klappern** · `SFX_Belts` · `VS`
- [ ] **Bau/Abriss/Platzierung ungültig** · `SFX_Build` · `PT`
- [ ] **Schritte pro Oberfläche (Erde, Metall, Beton, Wasser, Gras)** · `SFX_Footsteps` · `VS`
- [ ] **Werkzeuge & Ausrüstung** · `SFX_Equipment` · `VS`
- [ ] **Fahrzeuge & Züge** · `SFX_Vehicles` · `EA`
- [ ] **UI-Sounds** · `SFX_UI` · `VS`
- [ ] **Strom: Ausfall, Brummen, Überlast** · `SFX_Power` · `VS`
- [ ] **Fluide: Rohrfluss, Pumpen, Tanks** · `SFX_Fluids` · `EA`

## Ambience & Stimme

- [ ] **Biom-Ambience (7 Biome × Tag/Nacht)** · `AMB_Biomes` · `EA`
- [ ] **Wetter-Ambience** · `AMB_Weather` · `EA`
- [ ] **Fabrik-Gesamtrauschen (Distanz-Layer)** · `AMB_FactoryHum` · `EA`
- [ ] **KI-Begleiter-Stimme (Tutorial/Story, DE+EN)** · `VO_Companion` · `1.0`

## Mix

- [ ] **Sound-Klassen & Submixe (Musik, SFX, UI, Ambience, Stimme)** · `AUD_Mix` · `VS`
- [ ] **Concurrency-Regeln für Maschinen (max. N gleichzeitig)** · `AUD_Concurrency` · `VS`
