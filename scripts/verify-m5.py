#!/usr/bin/env python3
"""Verify M5 target fixtures, all accepted tools and sequential workflow."""
from pathlib import Path
import hashlib
import json
import re
import shutil
import subprocess
from PIL import Image, ImageChops

root = Path(__file__).resolve().parents[1]
work = root / ".phase1/ecen21-m5"
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
    if "stopped with an error" in p.stdout or "add-in exited" not in p.stdout or "headless: key script finished" in p.stdout:
        raise RuntimeError(p.stdout)
    return p.stdout

target = out / "target"
target.mkdir(exist_ok=True)
target_script = target / "budget.script"
target_script.write_text("wait 120000\nEXE\n")
target_log = run("EC21Test.g3a", target, ("--script", str(target_script)))
if "headless: key script finished" in target_log:
    raise RuntimeError("Target fixture instruction/time budget exhausted before program return")
report = (target / "storage/fls0/ECEN21.txt").read_text()
if report != (work / "host-tests/results.log").read_text() or len(re.findall(
    r"^PASS:.*checks; failure line 0$", report, re.M)) != 6:
    raise RuntimeError("Target fixtures differ from passing host report: " + report)

benchmark = out / "benchmark"
benchmark.mkdir(exist_ok=True)
benchmark_script = benchmark / "budget.script"
benchmark_script.write_text("wait 120000\nEXE\n")
benchmark_log = run("EC21Bench.g3a", benchmark, ("--script", str(benchmark_script)))
benchmark_report = (benchmark / "storage/fls0/M5BENCH.txt").read_text()
if not re.fullmatch(r"PASS: M5 maximum-domain; 16 rows; 32 form-row checks; [1-9][0-9]* nodes\n", benchmark_report):
    raise RuntimeError("Maximum-domain benchmark validation failed: " + benchmark_report)
benchmark_instructions = int(re.search(r"add-in exited after (\d+) instructions", benchmark_log).group(1))

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
# All accepted numeric tool pages must remain pixel-identical. Home/about
# intentionally change to add the third tool and are excluded.
numeric_reference = root / ".phase1/milestones/M3/verification/numeric"
unchanged_numeric = 0
for name, image in images.items():
    if name in {"home", "home-after", "home-about"}: continue
    previous = Image.open(numeric_reference / (name + ".png")).convert("RGB")
    if ImageChops.difference(image, previous).getbbox() is not None:
        raise RuntimeError("Accepted numeric/Boolean tool page changed: " + name)
    unchanged_numeric += 1

numeric_images = images
images, flipflop_log = ui_script("apps/ecen21/screenshots/m4.script", out / "flipflop")
# Exact pixel relationships complement independently checked mathematical fixtures.
flipflop_checks = {
    "D_without_active_edge_holds_zero": same("d-start", "d-falling-hold", (0,92,210,120)) and same("d-start", "d-none-hold", (0,92,210,120)),
    "D_active_edge_captures_one": not same("d-start", "d-capture", (0,92,210,120)),
    "falling_edge_D_captures_zero": same("d-start", "d-fall-capture", (0,92,210,120)),
    "JK_toggles_one_then_zero": same("d-capture", "jk-toggle-one", (0,92,210,120)) and same("d-start", "jk-toggle-zero", (0,92,210,120)),
    "T_one_zero_one_inputs_produce_one_one_zero": same("jk-toggle-one", "t-one", (0,92,210,120)) and same("t-one", "t-hold", (0,92,210,120)) and same("jk-toggle-zero", "t-zero", (0,92,210,120)),
    "T_ignores_wrong_edge_then_toggles_on_falling": same("t-zero", "t-wrong-edge", (0,92,210,120)) and same("t-one", "t-fall-toggle", (0,92,210,120)),
    "SR_11_without_active_edge_holds_zero": same("d-start", "sr-11-wrong-edge-hold", (0,92,210,120)) and same("d-start", "sr-11-no-event-hold", (0,92,210,120)),
    "SR_set_reset_recovery": same("d-capture", "sr-set", (0,92,210,120)) and same("d-start", "sr-reset", (0,92,210,120)) and same("d-start", "sr-recovered", (0,92,210,120)),
    "reset_cancel_preserves_simulation": same("reset-one", "reset-cancel"),
    "tool_switch_preserves_complete_simulation": same("before-switch", "after-switch"),
    "help_preserves_complete_simulation": same("after-switch", "help-return"),
    "history_limit_rejects_without_overwriting_state": same("sixteen-steps", "history-full", (0, 0, 792, 320)),
    "history_limit_has_visible_feedback": not same("sixteen-steps", "history-full", (0, 320, 792, 385)),
    "all_history_pages_accessible": not same("history-page-one", "history-page-three") and not same("history-page-two", "history-page-three"),
    "invalid_SR_differs_from_valid_step": not same("sr-invalid", "sr-reset"),
    "locked_SR_has_no_binary_current_Q": same("sr-invalid", "sr-locked", (0, 90, 792, 130)),
    "reset_recovers_after_invalid_SR": not same("sr-locked", "sr-recovered", (0, 90, 792, 130)),
    "type_change_resets_to_default_D": same("d-start", "type-clears-history"),
    "arrows_and_EXE_select_flipflop_tool": same("type-clears-history", "arrow-enters-flip"),
    "reset_after_full_clears_error_and_history": not same("history-full", "reset-after-full"),
    "Boolean_tool_still_opens": ImageChops.difference(images["boolean-reentry"], Image.open(root / ".phase1/milestones/M2B/verification/ui/start.png").convert("RGB")).getbbox() is None,
    "numeric_tool_still_computes_default": ImageChops.difference(images["numeric-reentry"], numeric_images["carry-default"]).getbbox() is None,
}
if not all(flipflop_checks.values()):
    raise RuntimeError("Flip-flop UI invariants failed: " + repr(flipflop_checks))
# The accepted M4 module itself is unchanged; only its launcher screenshot changes.
flipflop_reference = root / ".phase1/milestones/M4/verification/flipflop"
unchanged_flipflop = 0
for name, image in images.items():
    if name == "home": continue
    previous = Image.open(flipflop_reference / (name + ".png")).convert("RGB")
    if ImageChops.difference(image, previous).getbbox() is not None:
        raise RuntimeError("Accepted M4 tool changed: " + name)
    unchanged_flipflop += 1
flipflop_images = images
images = {}
sequential_logs = []
for part in "abc":
    part_images, part_log = ui_script("apps/ecen21/screenshots/m5-" + part + ".script", out / ("seq-" + part))
    if set(images) & set(part_images): raise RuntimeError("Duplicate M5 screenshot names")
    images.update(part_images)
    sequential_logs.append(part_log)
sequential_log = "\n".join(sequential_logs)
sequential_checks = {
    "manual_cycle_matches_preset_state_table": same("t-state-first", "manual-cycle-state"),
    "repair_restores_original_state_table": same("t-state-first", "cycle-repaired"),
    "cancelled_capacity_edit_preserves_valid_result": same("cycle-repaired", "cancel-preserves-solution"),
    "duplicate_error_clears_old_result": not same("t1-equation", "no-stale-after-error"),
    "policy_edit_clears_old_result": not same("t-state-first", "policy-edit-invalidates"),
    "hold_policy_changes_unused_transitions": not same("t-state-first", "hold-state"),
    "fixed_control_selects_different_sequence": not same("controlled-e0-trace", "controlled-e1-trace"),
    "timeline_and_fixed_control_are_distinct": not same("controlled-e0-trace", "controlled-timeline"),
    "reverse_repair_restores_state_table": same("reverse-state", "reverse-repaired"),
    "mixed_trace_survives_tool_switch": same("before-tool-switch", "after-tool-switch"),
    "help_preserves_nonzero_page": same("trace-page-two-before-menu", "trace-page-two-after-help"),
    "menu_cancel_preserves_nonzero_page": same("trace-page-two-before-menu", "trace-page-two-after-menu-cancel"),
    "help_returns_without_data_loss": same("after-tool-switch", "after-help"),
    "sample_cancel_preserves_result": same("after-help", "after-sample-cancel"),
    "arrow_EXE_reentry_preserves_result": same("after-sample-cancel", "arrow-reentry"),
    "all16_rows_accessible": not same("max-domain-first", "max-domain-middle") and not same("max-domain-middle", "max-domain-last"),
    "selected_form_and_groups_are_visible": not same("t0-group", "t0-pos-group") and not same("t0-map", "t0-group"),
    "Boolean_solver_still_computes": ImageChops.difference(images["boolean-regression"], Image.open(root / ".phase1/milestones/M2B/verification/ui/default-sop.png").convert("RGB")).getbbox() is None,
    "numeric_solver_still_computes": ImageChops.difference(images["numeric-regression"], numeric_images["carry-default"]).getbbox() is None,
    "flipflop_tool_remains_unchanged": ImageChops.difference(images["flipflop-regression"], flipflop_images["d-start"]).getbbox() is None,
}
if not all(sequential_checks.values()):
    raise RuntimeError("Sequential UI invariants failed: " + repr(sequential_checks))
for name in ("production-header.log","target-test-header.log","benchmark-header.log"):
    text=(work / "logs" / name).read_text().split("Fields with unknown meanings:")[0]
    statuses=re.findall(r"^\s+(OK|BAD|FAIL|ERROR)\s",text,re.M)
    if len(statuses)!=12 or any(s!="OK" for s in statuses): raise RuntimeError(name)
manifest=json.loads((work / "source-manifest.json").read_text())
if any(hashlib.sha256((root/name).read_bytes()).hexdigest()!=sha for name,sha in manifest.items()):
    raise RuntimeError("Workspace differs from build inputs; rebuild required")
artifact=work / "artifacts/ECEN21.g3a"
summary={"milestone":"M5","artifact":str(artifact),"bytes":artifact.stat().st_size,
    "sha256":hashlib.sha256(artifact.read_bytes()).hexdigest(),
    "target_fixture_report":report.strip(), "maximum_domain_report":benchmark_report.strip(),
    "maximum_domain_whole_program_instruction_bound":benchmark_instructions,"emulator_revision":revision,
    "unchanged_accepted_boolean_screens":len(boolean),"numeric_navigation_screenshots":len(numeric_images),
    "unchanged_accepted_numeric_tool_screens":unchanged_numeric,
    "flipflop_screenshots":len(flipflop_images), "flipflop_interaction_invariants":flipflop_checks,
    "unchanged_accepted_flipflop_tool_screens":unchanged_flipflop,
    "sequential_screenshots":len(images), "sequential_interaction_invariants":sequential_checks,
    "interaction_invariants":checks,
    "known_unused_key_diagnostic": any("key left unused" in s for s in (boolean_log,numeric_log,flipflop_log,sequential_log)),
    "physical_hardware_verified_by_agent":False}
(work / "verification.json").write_text(json.dumps(summary,indent=2)+"\n")
print(json.dumps(summary,indent=2))
