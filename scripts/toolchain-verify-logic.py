#!/usr/bin/env python3
"""Run the freshly built Logic artifact in the existing native emulator.

Requires Pillow and the Phase 1 emulator build; outputs stay in .phase1/.
"""
from pathlib import Path
import hashlib
import json
import subprocess
import sys

from PIL import Image

root = Path(__file__).resolve().parents[1]
work = root / ".phase1/toolchain"
artifact = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else work / "artifacts/Logic.g3a"
emulator = root / ".phase1/emulator-build/emucasiocalc"
out = work / "verification"
out.mkdir(exist_ok=True)
names = ["main", "expression", "truth-table", "kmap", "kmap-edited"]
for name in names:
    (out / (name + ".png")).unlink(missing_ok=True)
lines = (root / "apps/logic/screenshots/demo.script").read_text().splitlines()
script = out / "demo.script"
script.write_text("\n".join(
    "shot " + str(out / line[5:]) if line.startswith("shot ") else line
    for line in lines) + "\n")
command = [str(emulator), "--headless", "--clock", "2026-01-01 12:00",
           "--script", str(script), str(artifact)]
result = subprocess.run(command, cwd=out, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
(work / "logs/logic-fresh-emulator.log").write_text(result.stdout)
print(result.stdout, end="")
result.check_returncode()
matches = {}
for name in names:
    actual = Image.open(out / (name + ".png")).convert("RGB")
    expected = Image.open(root / "apps/logic/screenshots" / (name + ".png")).convert("RGB")
    matches[name] = actual.size == expected.size and actual.tobytes() == expected.tobytes()
    assert matches[name], f"Screenshot mismatch: {name}"
summary = {"artifact": str(artifact), "bytes": artifact.stat().st_size,
           "sha256": hashlib.sha256(artifact.read_bytes()).hexdigest(),
           "emulator_returncode": result.returncode, "exact_pixel_matches": matches}
(work / "verification.json").write_text(json.dumps(summary, indent=2) + "\n")
print(json.dumps(summary, indent=2))
