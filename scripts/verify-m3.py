#!/usr/bin/env python3
"""Verify M3 SuperH fixtures, complete Boolean regression and numeric UI."""
from pathlib import Path
import hashlib
import json
import re
import shutil
import subprocess
from PIL import Image, ImageChops

root = Path(__file__).resolve().parents[1]
work = root / ".phase1/ecen21-m3"
out = work / "verification"
emulator = root / ".phase1/emulator-build/emucasiocalc"
revision = subprocess.check_output(["git", "-C", str(root / ".phase1/vendor/emuCasioCalc"),
    "rev-parse", "HEAD"], text=True).strip()
if revision != "63e325708e9eea53eb4769b9c057082614ebd919":
    raise RuntimeError("Unexpected emulator source revision")
if out.exists(): shutil.rmtree(out)
out.mkdir(parents=True)
(work / "verification.json").unlink(missing_ok=True)

def run(name, folder, extra=()):
    folder.mkdir(exist_ok=True)
    p = subprocess.run([str(emulator), "--headless", "--clock", "2026-01-01 12:00",
        *extra, str(work / "artifacts" / name)], cwd=folder, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120)
    (folder / "emulator.log").write_text(p.stdout)
    p.check_returncode()
    if "stopped with an error" in p.stdout or "add-in exited" not in p.stdout:
        raise RuntimeError(p.stdout)
    return p.stdout

target = out / "target"
target_log = run("EC21Test.g3a", target)
report = (target / "storage/fls0/ECEN21.txt").read_text()
if report != (work / "host-tests/results.log").read_text() or len(re.findall(
    r"^PASS:.*checks; failure line 0$", report, re.M)) != 3:
    raise RuntimeError("Target fixtures differ from passing host report: " + report)

def ui_script(relative, folder, prefix=(), suffix=()):
    folder.mkdir(exist_ok=True)
    lines = (root / relative).read_text().splitlines()
    names = [line[5:] for line in lines if line.startswith("shot ")]
    text = list(prefix) + ["shot " + str(folder / line[5:]) if line.startswith("shot ") else line
        for line in lines] + list(suffix)
    script = folder / "run.script"; script.write_text("\n".join(text) + "\n")
    log = run("ECEN21.g3a", folder, ("--script", str(script)))
    images = {}
    for name in names:
        image = Image.open(folder / name).convert("RGB")
        if image.size != (792,448) or image.getextrema() == ((255,255),) * 3:
            raise RuntimeError("Invalid screenshot: " + name)
        images[name[:-4]] = image
    return images, log

boolean, boolean_log = ui_script("apps/ecen21/screenshots/m2b.script", out / "boolean",
    prefix=("1",), suffix=("EXIT", "wait 300"))
reference = root / ".phase1/milestones/M2B/verification/ui"
for name, image in boolean.items():
    prior = Image.open(reference / (name + ".png")).convert("RGB")
    if ImageChops.difference(image, prior).getbbox() is not None:
        raise RuntimeError("Accepted Boolean screen changed: " + name)

images, numeric_log = ui_script("apps/ecen21/screenshots/m3.script", out / "numeric")
def same(a,b,box=None):
    x,y=images[a],images[b]
    if box: x,y=x.crop(box),y.crop(box)
    return ImageChops.difference(x,y).getbbox() is None

checks = {
    "Boolean_state_preserved_between_tools": same("boolean-before","boolean-after"),
    "numeric_state_preserved_between_tools": same("carry-default","numeric-after"),
    "operation_changes_result": not same("manual-add","manual-sub"),
    "base_roundtrip_preserves_arithmetic": same("manual-sub","manual-sub-roundtrip"),
    "conversion_values_independent_of_input_base": same("negative-dec","negative-hex",(0,220,792,350)) and same("negative-dec","negative-bin",(0,220,792,350)),
    "negative_decimal_roundtrip": same("negative-dec","negative-dec-roundtrip"),
    "signed_toggle_preserves_hex_binary_bits": same("negative-dec","negative-unsigned",(0,282,792,350)),
    "signed_width_roundtrip": same("signed8-hex","signed8-roundtrip"),
    "truncation_retains_low_bits": same("signed8-hex","truncated-input",(0,0,792,350)),
    "truncation_warning_differs": not same("signed8-hex","truncated-input",(0,350,792,410)),
    "invalid_input_has_no_old_result": not same("carry-default","range-no-result"),
    "invalid_binary_blocks_base_change": same("binary-digit-error","blocked-base-change",(0,0,792,85)),
    "reset_recomputes_original_case": same("carry-default","carry-reset"),
    "sample_menu_return_preserves_result": same("signed32-min-sub","numeric-sample-return"),
    "numeric_reentry_preserves_result": same("signed32-min-sub","numeric-reentered"),
}
if not all(checks.values()): raise RuntimeError("UI invariants failed: " + repr(checks))
for name in ("production-header.log","target-test-header.log"):
    text=(work / "logs" / name).read_text().split("Fields with unknown meanings:")[0]
    statuses=re.findall(r"^\s+(OK|BAD|FAIL|ERROR)\s",text,re.M)
    if len(statuses)!=12 or any(s!="OK" for s in statuses): raise RuntimeError(name)
manifest=json.loads((work / "source-manifest.json").read_text())
if any(hashlib.sha256((root/name).read_bytes()).hexdigest()!=sha for name,sha in manifest.items()):
    raise RuntimeError("Workspace differs from build inputs; rebuild required")
artifact=work / "artifacts/ECEN21.g3a"
summary={"milestone":"M3","artifact":str(artifact),"bytes":artifact.stat().st_size,
    "sha256":hashlib.sha256(artifact.read_bytes()).hexdigest(),
    "target_fixture_report":report.strip(),"emulator_revision":revision,
    "unchanged_accepted_boolean_screens":len(boolean),"numeric_navigation_screenshots":len(images),
    "interaction_invariants":checks,
    "known_unused_key_diagnostic": any("key left unused" in s for s in (boolean_log,numeric_log)),
    "physical_hardware_verified_by_agent":False}
(work / "verification.json").write_text(json.dumps(summary,indent=2)+"\n")
print(json.dumps(summary,indent=2))
