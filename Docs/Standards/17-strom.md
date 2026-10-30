# 17 – Strom

Gehört zu `LOG_PowerPole_S`, `LOG_PowerCable`, `BLD_GenBiomass`.

Stand: **noch nicht gebaut.** Entscheidungen stehen (2026-10-30).

## Entscheidungen

| Thema | Entscheidung |
|---|---|
| Überlast | **Die Sicherung fliegt raus:** Verbraucht das Netz mehr, als erzeugt wird, fällt das ganze Netz aus, bis man es am Generator wieder einschaltet |
| Erster Generator | **Biomasse-Brenner, 30 MW.** Brennt so lange wie der Brennstoff hält: Blatt 2 s, Holz 4 s, Biomasse 8 s |
| Strom ab wann | **Von Anfang an:** Ofen, Konstruktor und Bohrer brauchen schon in Tier 0 Strom. Deshalb sind Brenner, Mast Mk1 und Kabel **Tier 0** |
| Verbrauch | Bohrer 5 MW, Ofen 4 MW, Konstruktor 4 MW, Assembler 15 MW |
| Pflanzen | **Büsche + Bäume** an festen Stellen, per Skript wie die Erzvorkommen. Busch gibt Blätter (schnell), Baum gibt Holz (langsamer). Sammeln mit `E` halten wie Erz von Hand |
| Nachwachsen | **Ja, nach 5 Minuten** Spielzeit. Später kommt eine Pflanzen-Farm, die Biomasse automatisch erzeugt |
| Biomasse | Blätter und Holz werden im **Konstruktor oder an der Werkbank** zu Biomasse gepresst |
| Masten | Kabel immer **max. 25 m**. Nur die Anschlüsse steigen: **Mk1 4 (Tier 0), Mk2 8 (Tier 2), Mk3 12 (Tier 3)**. Maschinen und Generatoren haben 1 Anschluss |
| Sicherung zurücksetzen | Mit **`E` am Generator**, Knopf „Wieder einschalten“. Geht nur, wenn genug Leistung da ist, sonst Hinweis |

> Offen: Kosten von Brenner, Mast und Kabel, Mengen der Biomasse-Rezepte, Tier und Werte der Pflanzen-Farm.
