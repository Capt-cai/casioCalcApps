#!/usr/bin/env python3
"""Verify target controller fixtures and production UI in the pinned emulator.

Requires host Pillow. Target fixtures produce a checked file; UI screenshots
have independent interaction invariants and are also inspected visually.
"""
from pathlib import Path
import hashlib
import json
import re
import shutil
import subprocess

from PIL import Image, ImageChops

root = Path(__file__).resolve().parents[1]
work = root / ".phase1/ecen21"
emulator = root / ".phase1/emulator-build/emucasiocalc"
expected_revision = "63e325708e9eea53eb4769b9c057082614ebd919"
revision = subprocess.check_output([
    "git", "-C", str(root / ".phase1/vendor/emuCasioCalc"), "rev-parse", "HEAD"
], text=True).strip()
if revision != expected_revision:
    raise RuntimeError("Emulator source revision differs from verified baseline")
out = work / "verification"
if out.exists():
    shutil.rmtree(out)
out.mkdir(parents=True)
(work / "verification.json").unlink(missing_ok=True)

def run(artifact, folder, extra):
    folder.mkdir(exist_ok=True)
    result = subprocess.run([
        str(emulator), "--headless", "--clock", "2026-01-01 12:00",
        *extra, str(artifact)
    ], cwd=folder, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
       text=True, timeout=120)
    (folder / "emulator.log").write_text(result.stdout)
    result.check_returncode()
    if "stopped with an error" in result.stdout or "add-in exited" not in result.stdout:
        raise RuntimeError(result.stdout)
    print(result.stdout, end="")
    return result.stdout

target_dir = out / "target"
target_log = run(work / "artifacts/EC21Test.g3a", target_dir, [])
report = (target_dir / "storage/fls0/ECEN21.txt").read_text()
expected = (work / "host-tests/results.log").read_text()
if report != expected or not re.fullmatch(
    r"PASS: ECEN21 controller; 218 checks; failure line 0\n", report
):
    raise RuntimeError("Target fixture result differs from passing host report: " + report)

ui_dir = out / "ui"
ui_dir.mkdir()
lines = (root / "apps/ecen21/screenshots/demo.script").read_text().splitlines()
names = [line[5:] for line in lines if line.startswith("shot ")]
script = ui_dir / "demo.script"
script.write_text("\n".join(
    "shot " + str(ui_dir / line[5:]) if line.startswith("shot ") else line
    for line in lines) + "\n")
ui_log = run(work / "artifacts/ECEN21.g3a", ui_dir, ["--script", str(script)])
images = {}
for name in names:
    image = Image.open(ui_dir / name).convert("RGB")
    if image.size != (792, 448) or image.getextrema() == ((255, 255),) * 3:
        raise RuntimeError("Unexpected/blank screenshot: " + name)
    images[name] = image

def equal(a, b, box=None):
    left, right = images[a], images[b]
    if box:
        left, right = left.crop(box), right.crop(box)
    return ImageChops.difference(left, right).getbbox() is None

# Screenshots are at 2x. Result regions ignore different input padding.
region = (0, 220, 792, 414)
invariants = {
    "typed_7E_equals_demo_result": equal("demo-result.png", "typed-result.png", region),
    "editing_invalidates_displayed_result": equal("start.png", "edited.png", region),
    "empty_input_is_distinct": not equal("start.png", "empty.png", region),
    "zero_and_one_are_distinct": not equal("zero.png", "all.png", region),
    "fifth_digit_is_ignored": equal("all.png", "bounded.png"),
    "about_preserves_result": equal("parity.png", "returned.png"),
}
if not all(invariants.values()):
    raise RuntimeError("UI invariant failed: " + repr(invariants))
artifact = work / "artifacts/ECEN21.g3a"
summary = {
    "artifact": str(artifact), "bytes": artifact.stat().st_size,
    "sha256": hashlib.sha256(artifact.read_bytes()).hexdigest(),
    "emulator_source_revision": revision,
    "target_controller_report": report.strip(),
    "screenshots": names, "interaction_invariants": invariants,
    "unused_key_diagnostic": "key left unused" in ui_log,
    "physical_calculator_verified": False,
}
(work / "verification.json").write_text(json.dumps(summary, indent=2) + "\n")
print(json.dumps(summary, indent=2))
