#!/usr/bin/env python3
"""Zählt die abgehakten Checkboxen in checklisten/*.md und schreibt eine
Fortschritts-Tabelle in die README.md (zwischen den PROGRESS-Markern).

Aufruf:  python tools/progress.py
Läuft auch automatisch per GitHub Action bei jedem Push.
"""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CL = ROOT / "checklisten"
README = ROOT / "README.md"
MILESTONES = ["PT", "VS", "EA", "1.0"]

box = re.compile(r"^\s*- \[( |x|X)\] ")
top = re.compile(r"^- \[( |x|X)\] .*?`(PT|VS|EA|1\.0)`")


def bar(done, total, width=20):
    pct = done / total if total else 0
    full = round(pct * width)
    return f"`{'█' * full}{'░' * (width - full)}` {pct:5.1%}"


rows, ms_count = [], {m: [0, 0] for m in MILESTONES}
all_done = all_total = 0
for f in sorted(CL.glob("*.md")):
    title = f.read_text(encoding="utf-8").splitlines()[0].lstrip("# ").strip()
    done = total = 0
    for ln in f.read_text(encoding="utf-8").splitlines():
        m = box.match(ln)
        if m:
            total += 1
            done += m.group(1) != " "
        t = top.match(ln)
        if t:
            ms_count[t.group(2)][1] += 1
            ms_count[t.group(2)][0] += t.group(1) != " "
    all_done += done
    all_total += total
    rows.append(f"| [{title}](checklisten/{f.name}) | {done}/{total} | {bar(done, total)} |")

table = ["| Bereich | Erledigt | Fortschritt |", "|---|---|---|", *rows,
         f"| **Gesamt** | **{all_done}/{all_total}** | {bar(all_done, all_total)} |", "",
         "| Meilenstein | Assets fertig | Fortschritt |", "|---|---|---|"]
names = {"PT": "Prototyp", "VS": "Vertical Slice", "EA": "Early Access", "1.0": "Release 1.0"}
for m in MILESTONES:
    d, t = ms_count[m]
    table.append(f"| `{m}` {names[m]} | {d}/{t} | {bar(d, t)} |")

text = README.read_text(encoding="utf-8")
new = re.sub(r"(<!-- PROGRESS:START -->).*?(<!-- PROGRESS:END -->)",
             lambda m: m.group(1) + "\n" + "\n".join(table) + "\n" + m.group(2),
             text, flags=re.S)
README.write_text(new, encoding="utf-8")
print(f"{all_done}/{all_total} Checkboxen erledigt")
