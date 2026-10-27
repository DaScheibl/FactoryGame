# Bedrohung / Aliens (optional & abschaltbar)

Aus dem ursprünglichen Konzept: Lärm → Bedrohung → Welle. Empfehlung: als Spieloption (wie Factorio Peaceful), damit der entspannte Modus genauso vollwertig ist. **Wenn gestrichen: Datei komplett überspringen.**

<details>
<summary><b>Definition of Done</b> für jeden Eintrag in dieser Datei</summary>

- Mesh + Rig + Animationen (Laufen, Angriff, Treffer, Tod)
- AI-Verhalten & Pathfinding auf Fabrikgelände
- VFX/SFX
- Balancing im Wellen-System
- Kodex-Eintrag

</details>

Legende: `PT` Prototyp · `VS` Vertical Slice · `EA` Early Access · `1.0` Release

## System

- [ ] **Spieloption: Bedrohung aus/niedrig/normal/hoch** · `SYS_ThreatSetting` · `VS`
- [ ] **Lärm-Messung pro Maschine + Lärm-Overlay** · `SYS_Noise` · `EA`
- [ ] **Bedrohungs-Meter HUD** · `UI_ThreatMeter` · `EA`
- [ ] **Wellen-Ansage (Warnung, Countdown, Musik-Wechsel)** · `SYS_WaveWarning` · `EA`

## Gegner

- [ ] **Kriecher (Schwarm, schwach)** · `ENM_Crawler` · `EA`
- [ ] **Sprenger (zerstört Bänder)** · `ENM_Bomber` · `EA`
- [ ] **Brecher (gepanzert, Gebäude-Angreifer)** · `ENM_Brute` · `EA`
- [ ] **Flieger** · `ENM_Flyer` · `1.0`
- [ ] **Boss / Hive-Mutter** · `ENM_Boss` · `1.0`
- [ ] **Alien-Nester (Weltobjekt)** · `ENM_Nest` · `EA`

## Verteidigung

- [ ] **Mauer-Set (Stahl, verstärkt)** · `DEF_Walls` · `EA`
- [ ] **Geschützturm (Munition)** · `DEF_TurretAmmo` · `EA`
- [ ] **Laser-Turm (Strom)** · `DEF_TurretLaser` · `1.0`
- [ ] **Schalldämpfer-Modul (senkt Lärm)** · `DEF_Silencer` · `EA`
- [ ] **Munition-Items** · `ITM_Ammo` · `EA`
- [ ] **Reparatur-Drohne** · `DEF_RepairDrone` · `1.0`
