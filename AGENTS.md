# ECEN21 Toolkit engineering workflow

## Communication and ownership

- User-facing updates, questions, explanations, blockers and final reports: Simplified Chinese, with English technical terms where useful.
- Subagent instructions, coordination, reviews and handoffs: English.
- Source/comments, technical documentation, plans, test reports/logs and commit messages: English.
- User acceptance guides are an explicit exception: write them in Simplified Chinese for non-developers.
- Keep communication concise. Summarize relevant findings for the user; do not translate entire internal reports repeatedly.
- The Main Agent owns planning, implementation, delegation, integration, verification and project records.

## Persistent project records

Read `PLAN.md`, `PROGRESS.md`, `TESTING.md` and relevant implementation/design documentation before work. Initial setup also reviewed `docs/ECEN21-PHASE1.md`, `docs/TOOLCHAIN.md` and `docs/BOOLEAN-MINIMIZATION.md`.

- `PLAN.md`: approved milestone, backlog/priorities and acceptance criteria.
- `PROGRESS.md`: completed work, current status, blockers and next actions.
- `TESTING.md`: reproducible procedures, actual results, evidence and limits.
- `AGENTS.md`: persistent engineering rules.
- `PRD.md`: product requirements and proposed M5-M9 roadmap.
- `docs/PROBLEM-COVERAGE.md`: source-linked coverage, acceptance IDs and limits.
- `docs/M5-PLAN.md`: bounded M5 requirements and acceptance gates.
- `docs/M6-PLAN.md`: approved M6 scope,model assumptions and acceptance gates.

Identify the approved milestone and acceptance criteria, inspect existing code and form a concise execution plan. Work autonomously through its authorized subtasks. After meaningful changes, run relevant tests and append verified results/next actions to the appropriate records. Preserve prior evidence and avoid full-document rewrites for small updates.

## Authorized scope and approval boundaries

Within the approved milestone, inspect/modify project source, create modules/tests, run builds/emulators, debug/fix errors, reasonably refactor new code, update documents and make routine implementation choices without repeated approval.

Stop for user approval before an unapproved milestone, a major architectural change, significant modification of upstream Logic/shared application code, destructive/irreversible actions, physical calculator interaction/manual verification, or a significant unresolved blocker needing a user decision. State the concrete reason and reviewable decision. Do not turn ordinary implementation choices into approval gates.

**P0 version control/remote backup is the current highest-priority authorized task. All new feature development is paused. M9 has not started and is not authorized.** Preserve all existing history/files;protect M8 source/docs/tests/build recipes and historical recovery archives. Current M8 engineering passed but user acceptance remains pending;M7/custom-width pending,M5/M6 hardware deferred. The user explicitly reconfirmed original M2A-M4 physical acceptance in the P0 request;do not extend it to later revisions or invent measurements. Resume product work only after further user instruction.

## Mandatory milestone user acceptance handoff

After completing each milestone's engineering work, the Main Agent must proactively provide a Simplified Chinese user acceptance guide without waiting for the user to request one. Save a reusable copy in the repository and present actionable instructions in the user-facing handoff, not just a link or a developer test report. Use plain language; assume no knowledge of compilation, Git, terminals or test tools. Explain unavoidable terms and identify the exact user program file rather than a test harness.

Every guide must cover all seven items:

1. Features delivered in this milestone and relevant scope limits.
2. Automated checks actually passed and behaviours not yet verified, clearly distinguishing host tests, emulator execution and physical hardware.
3. How to install or run the delivered program using ordinary user operations; include the artifact location/version and applicable device.
4. A numbered procedure specifying exact keys/inputs and expected visible output at each step, including errors/boundaries and exit where applicable.
5. A clear pass/fail checklist tied to the approved milestone's acceptance criteria.
6. What the user should provide if a step fails: step number, exact input/key sequence, actual versus expected output, photo/error text, device/version if available and repeatability.
7. The next milestone's intended goals and a concise recommendation, explicitly marked as unapproved until authorization arrives.

Keep **engineering verification complete**, **user acceptance pending/passed**, and **next-milestone approval pending/granted** as separate states in `PLAN.md`/`PROGRESS.md`. Preserve existing verified evidence when updating status. Never treat automated success, delivery of a guide, silence or an ambiguous acknowledgement as user acceptance or permission to advance.

Do not begin the next milestone until the user explicitly confirms the current milestone's acceptance AND explicitly approves the next milestone. Both statements may be given in one message; acceptance alone is insufficient. Remain within the current milestone to address acceptance defects and rerun affected checks, then update the guide as needed. Do not operate the physical calculator on the user's behalf without explicit authorization; providing this guide does not count as hardware verification.

## Economical orchestration

Use one Main Agent by default. Spawn subagents only for substantially independent tasks where parallelism has meaningful value and explicit responsibilities, deliverables, acceptance criteria and file/integration ownership are established. Use English delegation. Avoid concurrent edits and duplicate investigations; require verified results, independently review critical outputs and integrate through the Main Agent. Do not simulate a team by assigning redundant roles. Report relevant conclusions to the user in Chinese.

## Engineering and verification

- Prioritize correctness, reproducibility, maintainability and bounded resource use.
- Reuse verified components; preserve `apps/logic/`, upstream shared code and checked-in binaries. Avoid unnecessary dependencies or large refactors. Preserve copyright/license/font attribution.
- Prefer isolated portable C computational modules, separate from gint display/input. Maintain independent host tests and emulator integration tests.
- Reuse the **pinned ARM64 Docker toolchain** from `docs/TOOLCHAIN.md`; do not repeat Phase 1 research or install native macOS tooling unless necessary. No host `fxsdk` installation is assumed.
- Keep generated build/log/screenshot artifacts isolated and ignored; preserve source manifests and artifact hashes. Clear stale outputs so failed runs cannot look successful.
- Run targeted tests during iteration, then relevant regression tests at milestone acceptance. Record exactly what ran, success/failure and material coverage limits. Never invent test success or represent emulator results as physical hardware verification.
- Measure target compilation/resource estimates and runtime where feasible. Clearly distinguish compiler estimates, emulator timing and physical measurements.
- Record user-reported physical acceptance separately from agent-run checks. M4 real fx-CG50 acceptance is explicitly reported by the user; unreported per-step results, OS version, physical latency/peak memory and long-run measurements must not be invented.
- Stop after the approved milestone and record the next actionable task; do not use scheduled automation to bypass milestone approval.

## Authorized M3 custom-width follow-up

The user explicitly requested custom Binary widths up to32, overriding the M3 freeze only for this bounded change. Support1..32 widths, retain preset access, validate partial HEX nibbles and arithmetic/conversion boundaries. M7 was subsequently explicitly approved as recorded below; M5/M6 physical verification remains deferred. Preserve the M6 engineering snapshot under `.phase1/milestones/M6-engineering/`. New outputs/helpers: `.phase1/ecen21-width/`, `scripts/toolchain-build-width.sh`, `scripts/verify-width.py`. Keep this follow-up acceptance separate from accepted original M3 and deferred M5/M6.

## M7 explicit authorization — November 12, 2026

User: “批准进入M7”. Implement the bounded scope and tests in `docs/M7-PLAN.md`. This explicit next-stage instruction authorizes M7 without inventing acceptance of the custom-width follow-up or M5/M6 hardware. Those acceptances remain pending/deferred. Preserve all previous cores/UI except launcher integration. Default single Main Agent. M8 remains unapproved. Current M7 engineering status: in progress.

## M7 completion state

Engineering passed:23912shared target/host,3861088extended host,91UIimages/21invariants,prior regressions and fresh builds. Records/Chinese guide delivered. M7 user acceptance pending; M8 unapproved. Do not advance automatically.

## M8 explicit authorization

User: “继续做M8”. M8 implementation is authorized under the bounded PRD HDL contract. M7 acceptance remains pending; M5/M6 physical verification stays deferred and custom-width follow-up acceptance stays pending. This user-directed next-stage exception does not fabricate earlier acceptance. Preserve `.phase1/milestones/M7-engineering/` and prior cores/UI except launcher. M9 remains unapproved. Single Main Agent; no subagents requested.

## M8 completion state

92120shared host/target checks,343HDL cases/280704checks,17native exports compiled/simulated,70UIimages/19invariants and prior regressions/fresh builds passed. M8 user acceptance pending;M9 unapproved. M7/custom-width pending andM5/M6 deferred statuses remain separate.

## Permanent Git workflow — applies to every future Codex session

- At each task start inspect git status,log,branch,remotes and relevant tags before
  editing. Read docs/VERSION-CONTROL.md. After clone ensure repository-local hooks
  are installed using scripts/setup-git-workflow.sh;do not overwrite user work.
- Each approved milestone uses an independent `codex/<milestone>-<topic>` branch.
- After a meaningful coherent change and relevant passing checks,automatically
  create a clear Git commit. Do not wait until several milestones accumulate.
- Before significant refactoring,make a recoverable checkpoint of tested work.
- Engineering verification passed ->annotated Engineering Checkpoint with source/
  artifact hashes and actual tests. Explicit user physical acceptance of the exact
  artifact ->Accepted Release. Never call a pending engineering build stable.
- Never commit credentials/private files/unrelated content/temp build caches.
  Inspect staged changes and run scripts/audit-git-content.py --staged;curated
  recovery bundles and documented user artifacts are intentional exceptions to
  excluding ordinary build outputs. Preserve original upstream licenses/credits.
- Automatically push tested commits/current authorized milestone branch/new tags
  only to the verified personal origin under tools/versioning/remote-policy.json.
  Check authentication,ownership/push permission and remote ancestry first.
  No upstream push,force push,history rewriting,remote deletion or tag replacement.
- Stop remote writes on login/permission/conflict/URL problems;continue independent
  local protection and report exact sync state. No false remote-success claims.
- Main promotion requires explicit authorized scope;P0 first recovered baseline may
  fast-forward personal main only if the observed remote history is unchanged.
- At each delivery report branch,commit/checkpoint hash,test state and remote
  synchronization. Maintain source manifests/artifact hashes and recovery index.
- These rules govern work during active authorized tasks;do not create unattended
  commit/push timers or bypass milestone approval/acceptance boundaries.

## M8 exam-oriented revision — authorized 2026-11-13

User explicitly authorized an M8-only revision, not M9. Branch:
codex/m8-exam-verilog. Preserve the recovered M8 engineering tag/artifact,
all accepted versions, and every existing user manual (including the PDF).
No new or rewritten user guide is authorized for this task.

Scope: concise fixed-selected-width Verilog-2001, correct behavioral always@(*)
vs dataflow assign vs genuinely structural primitive/FA-instance modeling;
retain all nine design families/settings/paging/ports/export/no-overwrite and
other six tools. Add an adder Model setting and a separate ECAD dataflow prefix.
Validate host/core regressions, actual Icarus behavior across boundary widths,
reset priority, both shifts, old/new results against independent oracles,
fresh production/target builds, all-screen code navigation/native exports and
unchanged previous-tool emulator pixels. Compare real before/after lengths.
New work/evidence uses .phase1/ecen21-m8-exam via ECEN21_M8_WORK so original
M8 evidence remains intact. Engineering in progress; hardware pending.
Stop after engineering handoff; M9 remains unapproved.

## M8 exam-oriented revision — engineering verification passed

Completed authorized M8-only revision on codex/m8-exam-verilog. All nine original
families/settings/export protections retained; adders explicitly distinguish
behavioral/dataflow/structural. Fixed-selected-width templates remove engineering
boilerplate; defaults shrink49.4..85.6%in bytes. Every raw line fits46characters.
Ten sanitized host suites and matching target suites pass;HDL328cases/280690checks
pass on both old/new generators against independent oracles.18native exports
compile/simulate;467prior tool screens unchanged;M8 74images/21invariants;
10variants/16complete code pages visually checked. Clean production/test/benchmark
builds pass;40maximum configs peak428source bytes/20visual lines.

Deliverable:releases/engineering/M8-exam/ECEN21.g3a,156256bytes,
SHA2568e2c940df3b16878c1f1a8c1b617e4b37f95b34fc7061161951bb2a3c80e4152.
Internal design/reproduction/results:docs/M8-EXAM-REVISION.md. Original M8 binary,
all10recovery archives and all10existing user guides/PDFs unchanged. No user
manual created/rewritten,per explicit task instruction. New engineering checkpoint
name:engineering/m8-exam-verilog (not an accepted/stable release).
Hardware acceptance of this revision is pending. Other pending/deferred statuses
remain separate. Next action:await user physical acceptance or bounded M8 fixes.
M9 remains unapproved and has not started. No physical timing/USB/FPGA claim.
