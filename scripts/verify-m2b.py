#!/usr/bin/env python3
"""Run shared SuperH fixtures and M2B production interaction checks.

Requires the verified native emulator and Pillow. No physical hardware access.
"""
from pathlib import Path
import hashlib
import json
import re
import shutil
import subprocess
from PIL import Image, ImageChops

root = Path(__file__).resolve().parents[1]
work = root / ".phase1/ecen21-m2b"
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
    result = subprocess.run([str(emulator), "--headless", "--clock", "2026-01-01 12:00",
        *extra, str(work / "artifacts" / name)], cwd=folder, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120)
    (folder / "emulator.log").write_text(result.stdout)
    result.check_returncode()
    if "stopped with an error" in result.stdout or "add-in exited" not in result.stdout:
        raise RuntimeError(result.stdout)
    return result.stdout

target = out / "target"
target_log = run("EC21Test.g3a", target)
report = (target / "storage/fls0/ECEN21.txt").read_text()
host_report = (work / "host-tests/results.log").read_text()
if report != host_report or len(re.findall(r"^PASS:.*checks; failure line 0$", report, re.M)) != 2:
    raise RuntimeError("Shared target fixtures failed or differ from host: " + report)

ui = out / "ui"; ui.mkdir()
lines = (root / "apps/ecen21/screenshots/m2b.script").read_text().splitlines()
names = [line[5:] for line in lines if line.startswith("shot ")]
script = ui / "m2b.script"
script.write_text("\n".join("shot " + str(ui / line[5:]) if line.startswith("shot ") else line
    for line in lines) + "\n")
ui_log = run("ECEN21.g3a", ui, ("--script", str(script)))
images = {}
for name in names:
    image = Image.open(ui / name).convert("RGB")
    if image.size != (792,448) or image.getextrema() == ((255,255),) * 3:
        raise RuntimeError("Invalid screenshot: " + name)
    images[name[:-4]] = image

def same(a,b,box=None):
    left,right=images[a],images[b]
    if box: left,right=left.crop(box),right.crop(box)
    return ImageChops.difference(left,right).getbbox() is None

invariants = {
    "SOP_POS_are_distinct": not same("default-sop","default-pos"),
    "manual_1_to_X_changes_table": not same("manual-one","manual-x"),
    "manual_navigation_preserves_table": same("manual-x","manual-roundtrip",(0,80,792,360)),
    "explicit_expression_restore_matches_original": same("default-sop","restored-expression-sop"),
    "typed_equivalent_expression_matches_original": same("default-sop","typed-equivalent-sop"),
    "cursor_edit_changes_result": not same("typed-equivalent-sop","cursor-result"),
    "invalid_table_does_not_show_prior_table": not same("default-table","invalid-table",(0,80,792,360)),
    "removing_DC_changes_result": not same("dc-sop","dc-removed-sop"),
    "restoring_DC_recovers_result": same("dc-sop","dc-restored-sop"),
    "variable_roundtrip_preserves_low_rows": same("dc-sop","vars-roundtrip-sop"),
    "all_DC_forms_have_distinct_completions": not same("all-dc-sop","all-dc-pos"),
    "zero_and_one_differ": not same("zero-sop","one-sop"),
    "result_page_wrap_retains_content": same("parity-sop","parity-page-wrap"),
    "all_eight_groups_accessible": not same("parity-group-first","parity-group-last"),
    "group_navigation_wraps_without_loss": same("parity-group-first","parity-group-wrap"),
    "help_return_preserves_result": same("parity-pos","help-return"),
}
if not all(invariants.values()):
    raise RuntimeError("UI invariants failed: " + repr(invariants))
for name in ("production-header.log", "target-test-header.log"):
    header=(work / "logs" / name).read_text().split("Fields with unknown meanings:")[0]
    statuses=re.findall(r"^\s+(OK|BAD|FAIL|ERROR)\s",header,re.M)
    if len(statuses)!=12 or any(s!="OK" for s in statuses):
        raise RuntimeError("Header validation failed: " + name)
manifest=json.loads((work / "source-manifest.json").read_text())
if any(hashlib.sha256((root/name).read_bytes()).hexdigest()!=sha for name,sha in manifest.items()):
    raise RuntimeError("Build inputs no longer match workspace; rebuild required")
artifact=work / "artifacts/ECEN21.g3a"
summary={"milestone":"M2B","artifact":str(artifact),"bytes":artifact.stat().st_size,
    "sha256":hashlib.sha256(artifact.read_bytes()).hexdigest(),
    "emulator_source_revision":revision,"target_fixture_report":report.strip(),
    "screenshots":len(names),"interaction_invariants":invariants,
    "unused_key_diagnostic":"key left unused" in ui_log,
    "physical_hardware_verified_by_agent":False}
(work / "verification.json").write_text(json.dumps(summary,indent=2)+"\n")
print(json.dumps(summary,indent=2))
