# ECEN21 Toolkit plan

## P0 — version control and remote backup; feature development paused

The user explicitly paused all new features and prioritized Git/GitHub protection.
M9 not started. Initial normal history:60upstream commits,HEAD/origin/main
12d6b4e1086746534636c3bb87703deeef5734a8;ECEN21 source/docs/tests/scripts untracked,
no milestone commits/tags. Codex internal tree captures are not milestone commits.
Personal origin:Capt-cai/casioCalcApps;author upstream:joserodpt/casioCalcApps,
never a push target. Initial API authentication401;user chose browser login.

Created10curated recovery archives with original source-manifest/user-binary
hashes;all verified and restored into separate empty directories. M8 user binary/
examples now mirrored in releases/engineering/M8. Legacy M4 bundle retained locally
unchanged;its.DS_Store metadata excluded from remote-safe curated recovery copy.
No historic milestone commits/backdated releases manufactured. Git branch is
codex/p0-m8-version-backup;code commit/checkpoint and final sync are recorded after
execution. Added permanent Git policy/hooks,recovery checks and artifact index.

Current acceptance per user record:Phase1accepted;originalM2A-M4user-reported real
hardware acceptance;custom-width pending;M5/M6hardware deferred;M7/M8pending;
M9unapproved. Current source remains M8 engineering;no new feature/source changes.
See [version control/recovery](docs/VERSION-CONTROL.md). Remote success is not
claimed until authenticated push and independent remote-ref/clone verification.


**Phase 1 software verification: COMPLETE.** Unchanged Logic was freshly compiled in a pinned ARM64 Linux container on this Apple Silicon Mac, and the new binary passed native macOS emulator verification. A fully native macOS compiler installation and physical calculator validation remain unverified. The checked-in prebuilt was preserved.

## Current product state and authorization

**M8 explicitly approved by “继续做M8”; engineering verification passed; user acceptance pending.** See [M8 guide](docs/M8-USER-ACCEPTANCE.zh-CN.md) and [model](docs/M8-MODEL.md). M7 acceptance remains pending, M5/M6 hardware deferred, custom-width follow-up pending. M9 unapproved. Historical sections preserve earlier status.

Phase1/M2A/M2B/M3/M4 are user-accepted. The user explicitly reported M4 passed
realCasio fx-CG50 acceptance and formally approved closure. **M4 closed; stable
checkpoint [M4-accepted](docs/checkpoints/M4-ACCEPTED.md). M5 implementation
approved by “开始M5吧”; engineering complete, hardware acceptance deferred by the user. M6 explicitly approved by “开始M6吧 实机验收后面一起做”; engineering complete, hardware acceptance deferred.** The bounded M5 plan is now authorized. A subsequent explicit request authorizes the bounded M3 custom1..32bit follow-up documented below; broader M3 or overall architectural changes remain unapproved.
See [PRD](PRD.md), [coverage matrix](docs/PROBLEM-COVERAGE.md) and
[detailed M5 proposal](docs/M5-PLAN.md). Older sections below preserve stage evidence.

| Stage | Outcome | Engineering | User acceptance / authorization |
| --- | --- | --- | --- |
| Phase1 | Build environment and fresh Logic verification | Passed | Accepted |
| M2A | Independent ECEN21 app | Passed | Accepted |
| M2B | Bounded Boolean/K-map/exact SOP/POS/DC | Passed | Accepted |
| M3 |Original8/16/32-bit number systems/arithmetic | Passed | Accepted |
| M3 follow-up |Custom1..32-bit widths | Passed | User acceptance pending |
| M4 |Single SR/D/JK/T toolkit | Passed | Realfx-CG50 accepted; closed |
| M5 |Counter and sequential design solver | Passed | Hardware acceptance explicitly deferred |
| M6 |Timing diagram and sequential simulation | Passed | Hardware acceptance deferred |
| M7 |Combinational design: adders,MUX,encoders,BCD display | Passed | User acceptance pending |
| M8 |Parameterized Verilog generation,code paging,text export | Passed | User acceptance pending |
| M9 |Integration/release | Not started | Unapproved |

## M6 engineering completion; joint hardware acceptance deferred

Explicit user authorization overrides the usual M5 hardware gate for M6 only;
M5 remains engineering-complete, hardware-deferred. Its binary/source/evidence/
original helpers are frozen under `.phase1/milestones/M5-engineering/`, without
an accepted label. M6 outputs use `.phase1/ecen21-m6/`. Main owns UI/integration/
review/records; one isolated portable-timing-model/test subagent owns timing.[ch],
timing_checks.[ch] and its model note. Read [M6 scope/semantics and acceptance](docs/M6-PLAN.md).
After M6, deliver a Chinese guide for deferred joint checks and stop. M7 unapproved.

M6 implemented six cell types (D/T/JK/SR edge devices,D/SR latches),rising/falling
edges,high/low transparency,sync/async reset polarity/value,initialX,derived clocks,
4input/4cell/4gate/64event bounded models,editable connections/events,waves/tables
and selected-event phase/reason replay. M3/M4/M5 computations/views remain intact.

Independent core/test subagent finished; Main reviewed ordering/reset/unknown/
phase semantics and added product presets/independent literal goldens/UI. Review
Timing1-5 pass, including two actual master/slave latches,ripple phases,four-cell
old-state sampling and pre-edge synchronous feedback reset. A possible-clock
reset explanation bug was fixed with132 extra assertions before final verification.

Actual checks passed: **15,247 timing +381 product-preset** assertions on sanitized
host and actual SuperH emulator, alongside all six earlier suites with identical
reports. **93 timing screenshots/19 interaction invariants** pass; all59 Boolean,
51numeric,53M4-script and99M5-engineering tool reference images match, with their
existing numeric15/FF22/sequential20 invariants retained. Existing numeric2,359,296,
sequential916,124/exactBoolean153,092 andCTest5/5 regressions pass.

Fresh `.phase1/ecen21-m6/artifacts/ECEN21.g3a`, **129,304 bytes**, SHA-256
`a5ebfb19d0eddf81020cfe2e3778b4b23faf6a83b30cf0907b6f81587cd1e0f8`.
Production/test/benchmark target builds and12 header checks each pass without
final compiler/Make warnings/errors;107-input provenance matches. Maximum-domain
program validates65columns/12signals/4cells including replay and finishes after
292,929 whole-program emulated instructions, not physical or worst-case latency.
Static ELF text100,164/data464/BSS10,992; target frame observations inTESTING.

[Chinese M6/deferred joint acceptance guide](docs/M6-USER-ACCEPTANCE.zh-CN.md) is
delivered for later use, not a demand to test hardware now. **M5/M6 engineering
passed; both hardware acceptances deferred by the user; M7 approval not granted.**
Keep the M4 accepted checkpoint and M5 engineering snapshot distinct.

## Completed parallel work

Exactly two subagents were actually spawned in parallel, finished, and were reviewed by the lead, with separate write ownership:

| Workstream | Owner | Files |
|---|---|---|
| Complete toolchain, fresh Logic build, fresh-binary emulator verification | Agent A — Toolchain Engineer | `tools/toolchain/`, `scripts/toolchain-*`, `docs/TOOLCHAIN.md`, ignored `.phase1/toolchain/`; generated Logic build files only |
| Exact minimization design, portable prototype, independent host tests | Agent B — Algorithm Engineer | `experiments/boolean/`, `docs/BOOLEAN-MINIMIZATION.md` |
| Review, independent verification, integration and project status | Lead Engineer | `PLAN.md`, `PROGRESS.md`, `TESTING.md` and review logs under ignored `.phase1/lead/` |

Application source under `apps/logic/` and `common/`, and the checked-in distribution binary, must remain unchanged during this work.

## Acceptance gates and results

1. **Passed:** dependency versions, image digest, embedded Git revisions and commands recorded in [TOOLCHAIN.md](docs/TOOLCHAIN.md).
2. **Passed:** a clean source copy without prebuilt add-ins compiled, linked and packaged successfully. Fresh artifact: `.phase1/toolchain/artifacts/Logic.g3a`; its hash and evidence are in [TESTING.md](TESTING.md).
3. **Passed:** agent and lead independently ran the fresh binary in the native emulator. All five demo screens exactly matched reference pixels. The known trailing unused-key diagnostic is retained; physical behaviour is not inferred from emulation.
4. **Passed for the documented domain:** isolated exact SOP/POS/don't-care prototype reviewed; 153,092 host minimizations passed independent-oracle checks under ASan/UBSan. SuperH compilation also passed. Four-variable don't-care coverage is sampled; target runtime is pending.
5. **Passed:** both agents finished; lead reviewed source isolation/provenance, reran host tests and final fresh-binary emulation, checked pixels, and verified original source/prebuilt hashes.

## Milestone 2A — Minimal ECEN21 Application (engineering complete; user accepted)

Implement a separate `apps/ecen21/` with repository `casio_app`/shared UI conventions. Link the existing verified `experiments/boolean/exact_boolean.c` directly rather than duplicating it. A portable controller owns a bounded four-digit hexadecimal ON-mask input for four variables; EXE computes exact SOP and validates all 16 rows. Provide the known `0x007e` example, zero/all-one inputs, result/cost display, About and exit. Keep the computation/controller independent of gint input/display.

Acceptance criteria:

1. Repository-level persistent workflow instructions installed; existing Phase 1 findings/evidence preserved.
2. Fresh `ECEN21.g3a` build using the pinned ARM64 toolchain, with source manifest, target/header checks and recorded hash.
3. Functional scripted emulator interaction: input/edit, actual calculate, clear/recalculate, known counterexample, constants, About and exit. Verify target results against independently known fixtures, not just screenshot existence.
4. Existing host exact-engine regression suite and new controller tests pass; exercise empty/bounded input, stale-result invalidation and correct outputs.
5. Inspect rendered screens, record resource estimates/emulator observations and explicitly leave physical calculator behaviour unverified.
6. Original Logic/shared sources and checked-in binaries remain unchanged. Update project records with actual results and stop.

All six acceptance criteria passed on November 12, 2026. Fresh production output: `.phase1/ecen21/artifacts/ECEN21.g3a` (72,532 bytes). Host algorithm regression, 218 host/controller checks, the same 218 checks running as SuperH code, and the production emulator UI smoke test passed. See `TESTING.md` for final hash, evidence and limits. The production app exposes only editable four-variable ON-mask SOP minimization. The separately built target-test harness is excluded from production.

The user explicitly stated “M2A验收通过 批准进入M2B”. **M2A user acceptance: passed. M2B approval: granted.** Prior build/test evidence and artifact are preserved under `.phase1/milestones/M2A/` in addition to the original `.phase1/ecen21/`. This records the user's acceptance without inventing agent-observed hardware measurements. Handoff reference: [M2A user acceptance guide](docs/M2A-USER-ACCEPTANCE.zh-CN.md).

## Milestone 2B — Complete Boolean Solver (engineering complete; user accepted)

Scope is bounded to 2–4 variables: expression editor, editable truth table with 0/1/X, synchronized Gray-order K-map, exact SOP/POS using the verified engine, and group-to-term explanation (not a full algebraic proof). Preserve upstream Logic/common/engine and M2A delivery evidence.

Acceptance criteria:

1. Expressions support A–D, constants, parentheses, NOT, AND, XOR and OR with documented precedence and bounded input. Syntax errors identify a position and invalidate old tables/results.
2. Truth-table cells cycle 0 -> 1 -> X -> 0; K-map shows the same row values. Expressions and manual-table source are explicit, so navigation never silently discards edits. Active variable counts 2/3/4 have defined transition semantics.
3. Both exact SOP/POS are generated and verified on all cared rows. Switching result form does not discard source data; constants, all-DC and known nonminimal-greedy fixtures are covered.
4. K-map grouping is tied to the selected solution; every group can be inspected with covered minterms/DC and its product term or POS clause. Long results/groups are paginated or scrolled without silent truncation.
5. Strict sanitized host tests cover parser/controller/K-map invariants plus existing algorithm regression. Shared integration fixtures also run as SuperH code. Production emulator scripts cover expressions, errors, 0/1/X editing, K-map editing/groups, both forms, source restoration, variable changes, navigation and exit.
6. Produce a fresh validated `ECEN21.g3a` in isolated M2B artifacts using the existing ARM64 container. Inspect screens and resource/compiler observations; preserve original Logic/shared code and existing checked-in binaries.
7. Record actual results and remaining hardware limits, proactively deliver a Chinese user acceptance guide, and stop for user acceptance. No subsequent milestone is approved.

Execution: one portable-model/test subagent owns only `solver.[ch]`, `solver_checks.[ch]` and its design note; Main Agent owns native UI, build/emulator integration, review and project records. These independent boundaries allow parallel development without concurrent edits. No repeated toolchain investigation.

All seven engineering gates passed on November 12, 2026. Main Agent reviewed the model and added six maximum-depth malformed-editor cases to the subagent's 6,510-check suite: final model count **6,516**, plus **218** unchanged legacy controller checks, all passing on host and SuperH emulator. Exact-engine regression passed **153,092** minimizations. Production smoke produced **59 screenshots** and passed **16** interaction invariants; representative layouts were visually reviewed. The fresh production artifact is `.phase1/ecen21-m2b/artifacts/ECEN21.g3a`, **80,124 bytes**, SHA-256 `05cca11650ee7ebe21debf62f2a471bc6a5df2f1a2ce03bfbf19d233e5de15c8`. See `TESTING.md` for reproducible evidence and scope limits.

User handoff: [M2B physical-calculator acceptance guide](docs/M2B-USER-ACCEPTANCE.zh-CN.md). **M2B user acceptance: explicitly passed** (user stated “M2B检验成功”). The user subsequently chose **“先只确定计划，暂不开始”** for the scoped M3 proposal. At that planning-only checkpoint M3 was not approved; it was subsequently approved by “允许进入M3” as recorded below. No full algebraic proof or support above four variables was added; group explanations show how the selected cube yields its term/clause.

## Milestone 3 — Binary Arithmetic and Signed Numbers (engineering complete; user accepted)

Goal: extend ECEN21 with a bounded integer tool useful for coursework, while retaining the accepted Boolean solver. The latest user instruction “允许进入M3” explicitly authorizes implementation of the scope below; it supersedes the earlier plan-only choice.

### Approved scope and semantics

- **8/16/32-bit widths**, binary/decimal/hexadecimal conversion, signed/unsigned interpretation of the same bits, and two's-complement negative decimal encoding/decoding.
- Two operands A/B; addition and subtraction. Show the retained-width result in all three bases alongside **carry**, **borrow** and **signed overflow**, with clear applicability rather than conflating these flags.
- Arithmetic keeps the low `width` bits. Display an explicit wrap/overflow indication when mathematical results do not fit the selected interpretation; do not silently present a wrapped result as an ordinary exact integer result.
- Decimal input respects the selected signed/unsigned range; hex/binary input specifies the raw bit pattern within the selected width. Empty, malformed, disallowed-digit and out-of-range input gives a clear error and invalidates old results. Base/mode switching preserves the current bit pattern.
- Width increase uses sign extension in signed mode and zero extension in unsigned mode. Width reduction retains low bits and explicitly reports any change to the interpreted value; do not silently lose information. Changing inputs/settings invalidates the old arithmetic result.
- Add a simple Boolean/Binary tool selector. Preserve Boolean state while switching tools. Returning from a tool goes to the selector; exiting the selector returns to the calculator OS. Record this intentional navigation change in the user guide and regression tests.
- Reuse relevant `apps/base-n/` masking, formatting and signed-value conventions through reviewed adaptation, preserving original MIT attribution. Do not modify upstream Base-N/Logic/shared code or existing distribution binaries.
- Excluded: 64-bit mode, octal, multiplication/division, shifts/rotates, general expression arithmetic, one's-complement/sign-magnitude modes, flip-flops/counters/timing/Verilog. Consider these separately after this bounded milestone.

### Execution order

| Step | Work | Completion evidence |
|---|---|---|
| M3.1 | Portable width/input/conversion/flag model, separate from gint | Independent numeric oracle, boundary fixtures and sanitized host checks |
| M3.2 | Tool selector and bounded operand/result views | Correct key flows; Boolean state survives tool switching; errors clear stale output |
| M3.3 | Integration and regression | Shared fixtures pass as SuperH code, existing Boolean suites pass, fresh `.g3a` and emulator checks |
| M3.4 | Delivery and stop | Artifact/hash/resource records and seven-part Chinese physical-user acceptance guide |

Main Agent owns tool navigation/UI, integration and evidence. One independent numeric-core/test subagent owns only numeric.[ch], numeric_checks.[ch] and its design note; no concurrent file edits. Reuse upstream Base-N conventions without editing its sources. Reuse the existing ARM64 Docker toolchain and isolate future artifacts under a new milestone directory. Archive the accepted M2B binary/source/evidence before implementation, not by overwriting its output folder. No calendar completion date is promised before work begins.

### Acceptance examples (engineering checks passed)

| Width / operation | Expected retained result and flags |
|---|---|
| 8-bit signed conversion, decimal `-5` | Hex `FB`, binary `11111011`; same bits mean unsigned251 |
| 8-bit unsigned `255 + 1` | `00`, decimal0; carry=1, signed-overflow=0 |
| 8-bit signed `127 + 1` | `80`, interpreted as -128; signed-overflow=1, carry=0 |
| 8-bit unsigned `0 - 1` | `FF`, interpreted as255; borrow=1, signed-overflow=0 |
| 8-bit signed `-128 - 1` | `7F`, interpreted as127; signed-overflow=1, borrow=0 |
| 16-bit signed `32767 + 1` | `8000`, interpreted as -32768; signed-overflow=1 |
| 32-bit unsigned `4294967295 + 1` | `00000000`; carry=1, signed-overflow=0 |
| 32-bit signed `2147483647 + 1` | `80000000`, interpreted as -2147483648; signed-overflow=1 |
| Invalid input | Binary digit2, excessive width, signed decimal out of range, empty operand: clear error and no stale answer |
| Navigation | Returning to Boolean preserves its expression/manual table/DC/form; module EXIT returns to tool selector |

These examples passed the actual model/target fixture checks. Host-only oracle testing passed **2,359,296 assertions**: every 8-bit pair for both operations and modes (262,144 cases), plus 32,768 deterministic 16/32-bit cases. Shared checks passed **20,779 numeric**, **6,516 Boolean** and **218 legacy** assertions on both host and SuperH emulator. Exact Boolean regression passed **153,092** cases, emulator CTest5/5. Production verification passed **54 numeric/navigation screenshots and15 interaction invariants**; all **59 accepted Boolean screenshots** matched the archived M2B pixels exactly. Representative numeric screens were visually reviewed. See `TESTING.md` for evidence and limits.

Fresh M3 production: `.phase1/ecen21-m3/artifacts/ECEN21.g3a`, **87,652 bytes**, SHA-256 `f309877c3dcd030bd452f11a97d53ab7ccf651d0fb0dcb35706e7f9406b39aed`. No final compiler/Make warnings/errors; target/header validation passed. Original Logic/Base-N/common/exact-engine files and accepted M2A/M2B artifacts remain unchanged. M3 uses a selector and retains both tools' in-memory states; exiting a tool returns to the selector rather than directly to the OS.

Handoff: [M3 Chinese user acceptance guide](docs/M3-USER-ACCEPTANCE.zh-CN.md). **M3 engineering verification: passed. User acceptance: explicitly passed** (“M3验收完成 开始M4”). **M4 approval: granted.** Physical hardware behaviour is not inferred from emulation or earlier milestone acceptance.

## Milestone 4 — Flip-flop toolkit (engineering complete; hardware user accepted; closed)

The prior planning-only checkpoint is superseded by the explicit instruction “M3验收完成 开始M4”: M3 acceptance passed and M4 implementation approved. The scope and gates below are now authorized. Accepted M3 source, binary, evidence and original helpers are archived in `.phase1/milestones/M3/`. Main Agent owns UI/integration/evidence; one independent model/test subagent owns only flipflop.[ch], flipflop_checks.[ch] and its design note. New outputs use `.phase1/ecen21-m4/`.

Goal: a bounded, single-bit sequential-logic learning tool covering SR, D, JK and T flip-flops. Reuse the existing launcher, gint UI conventions, portable-model/host-test separation and pinned toolchain; retain the accepted Boolean and numeric tools. No upstream Logic/common changes are planned.

Approved scope:

- Characteristic tables: show inputs, current Q, next Q and a short explanation (hold, set, reset, capture or toggle).
- Excitation tables: given current Q and desired next Q, show the required inputs. Explain X as an input don't-care, distinct from an unknown simulated state.
- Single-step simulation: choose initial Q, inputs, rising/falling active edge and a clock event (none, rising or falling). Only the selected active edge updates Q. Use synchronous, active-high SR semantics; this is not an asynchronous SR latch.
- SR with S=R=1 at an active edge reports an invalid/undefined next state. Do not present a usable binary next state or silently continue the sequence; require reinitialization before further stepping. A retained last-valid value is recovery information, not a claim about physical hardware behaviour.
- Bounded history: at most 16 steps showing inputs, clock event, previous/next state and status. Reject further steps with a clear history-full message until the user clears/reinitializes it. A reset action sets the simulation's initial state and clears history; it is not a physical asynchronous reset pin.
- Add a third launcher entry. Preserve in-memory state when switching tools and provide explicit history clearing and return-to-home actions.

Excluded: asynchronous preset/clear, unknown-state propagation, multiple interconnected flip-flops, counters/FSM synthesis, continuous timing diagrams, electrical propagation/setup/hold/metastability analysis and Verilog generation. These require separately approved scope.

| Stage | Work | Required evidence |
| --- | --- | --- |
| M4.1 | Portable transition and excitation model | Exhaustive independent checks over binary states/inputs and clock events; expand excitation X entries and verify their meaning |
| M4.2 | Tables, explanations and bounded simulation UI | Exact input/output fixtures; invalid-state recovery and history-limit checks |
| M4.3 | Launcher integration and regression | Boolean/numeric regressions, shared target fixtures, emulator interactions and layout inspection |
| M4.4 | Build and acceptance handoff | Fresh production `.g3a`, source manifest/hash/resource observations and seven-part Chinese user guide; stop for acceptance |

Acceptance examples (engineering checks passed):

| Case | Expected result |
| --- | --- |
| Rising-edge D, Q=0, D=1, no event or falling edge | Q stays 0; a subsequent rising edge gives Q=1 |
| JK, Q=0, J=K=1, two active edges | Q changes 0 -> 1 -> 0 |
| T, Q=0, T inputs 1,0,1 over three active edges | Q sequence 1,1,0 |
| SR, Q=0, S=1,R=0, then S=R=1 on active edges | First Q=1; then invalid next state, with further stepping blocked until reinitialization |
| Excitation, JK desired transition 1 -> 0 | J=X,K=1; either J value satisfies the transition |
| History/navigation | Exactly 16 entries fit; step 17 is rejected clearly; switching tools preserves state; clearing restores usable simulation |

All four flip-flops' complete characteristic/excitation tables and both active-edge choices must pass exhaustive model checks. Host, emulator and physical results will be recorded separately. User acceptance remains a distinct final gate.

All four M4 engineering stages passed on November12,2026. The independent model/test subagent finished its isolated files; Main Agent reviewed the truth-table oracle, fixed integration counter reporting with the agent, independently reran host checks and completed UI/target verification. Shared host and SuperH reports match: **3,849 flip-flop +20,779 numeric +6,516 Boolean +218 legacy** assertions. Numeric host oracle passed **2,359,296** assertions; Boolean exact regression **153,092** cases; emulator CTest **5/5**. Production checks passed **54 flip-flop screenshots/22 interaction invariants**, **54 numeric/navigation screenshots/15 invariants** and exact archived-reference matches for **59 Boolean** and **51 numeric-script tool** pages. Tables, invalid recovery and all history pages were visually reviewed.

Fresh M4: `.phase1/ecen21-m4/artifacts/ECEN21.g3a`, **93,804 bytes**, SHA-256 `165a5a2230b075352046e0f666b9b9d981d47b99c69af06443373fcadefcff6f`. Target/header/source-manifest validation passed, no final compiler/Make warnings/errors. Original upstream sources, accepted Boolean/numeric models/views, exact engine and archived artifacts remain unchanged. Resource observations and verification limits are in `TESTING.md`.

Handoff: [M4 Chinese user acceptance guide](docs/M4-USER-ACCEPTANCE.zh-CN.md). **M4 engineering verification: passed. M4 real fx-CG50 user acceptance: explicitly passed; closure approved. M5 implementation approval at M4 closure: not granted (subsequently approved; see M5 record below).** Hardware acceptance is user-reported; no numerical latency/peak-memory or agent-operated physical measurements are inferred. Stable source/artifact/evidence is archived in `.phase1/milestones/M4/`.


## Milestone 5 - Counter & Sequential Design (engineering complete; acceptance pending)

Approved by “开始M5吧”. Implemented the bounded four-mode workflow in
[the approved M5 plan](docs/M5-PLAN.md):2-3Q bits plus0-1E, per-bitD/T/JK/SR,
RETURN/HOLD/DC, custom cycles/reverse expressions/optional outputs/E timelines.
Reused accepted Boolean parser/minimizer/K-map and flip-flop characteristic/
excitation functions unchanged. Main completed native UI/integration and reviewed
both isolated model/test and connection/test subagents; their owned files are
frozen. No M3/upstream or overall framework rewrite was performed.

Required state/excitation/output tables, exact per-pin SOP/POS with separately
replayed DC completions, Gray maps/group details, traces/recovery/traps and
explicit shared gate/pin wiring are implemented. Connection text is independently
parsed/executed by tests, including Counter2's oneXOR3+oneNOT constraint.

Actual verification passed: **2,216 sequential +128,986 connection +3,849 flip-flop
+20,779 numeric +6,516 Boolean +218 legacy** shared assertions on sanitized host
and SuperH emulator with identical reports; **916,124** host sequential assertions
across **5,408** models; prior numeric2,359,296 assertions, exact Boolean153,092
cases and emulatorCTest5/5. Native scripts produced **100 M5 screenshots/20
invariants**, retained all59 Boolean/51 numeric/53 M4-script tool reference pixels,
and reran15 numeric/22 flip-flop invariants. Detailed results/limits are inTESTING.

Fresh production `.phase1/ecen21-m5/artifacts/ECEN21.g3a`: **114,596 bytes**, SHA-256
`3cf6c16f69e8517e09ebaf0949e3fd739bb08f2fc31cbe6a25e93f6433dac97f`. Clean production/test/benchmark builds,
all12 header checks per add-in and93-input source manifest validation passed.
Separate maximum-domain program passed16 rows/32 form-row checks/28 nodes and
returned after534,076 emulated instructions including initialization/report I/O;
this is one representative whole-program bound, not physical/worst-case latency.
ELF text85,456/data464/BSS8,720; dynamic/runtime resources and physical peaks are
not measured. Accepted M4 checkpoint and all prior artifacts remain intact.

Handoff: [Chinese M5 user acceptance guide](docs/M5-USER-ACCEPTANCE.zh-CN.md).
**Engineering verification: passed. M5 user acceptance: pending. M6 approval: not
granted.** Coverage rows remain Partial until user acceptance, with implemented
engineering evidence explicitly distinguished from future features.

## Product route (M5-M8 engineering complete; M9 implementation unapproved)

The user replaced the earlier loose roadmap with the connected problem-solving
requirements in `PRD.md`. This supersedes the previous M7-Verilog/M8-release
numbering: **M7 is combinational design, M8 Verilog, M9 integration/release**.

| Stage | Required scope | Acceptance emphasis |
| --- | --- | --- |
| M5 |State sequence, mixedFFs, E-controlled counters, reverse pin/output equations; present/next/excitation tables, exact equations/maps, sequence/recovery verification and circuit connections |Minimum all three-bit examples; recommended2-3state bits plus0-1E; detailed fixturesM5-A1 throughR1 in linked plan |
| M6 |D/T/JK/SR, latches, +/-edges,master/slave,connected clocks,sync/async reset,initial states,event tables/waves/explanations |ReviewTiming1-5, correct level/edge distinctions, derived-clock settling and explicit ideal-model assumptions;4signals x8event view proposal |
| M7 |Half/full adders,ripple structure,MUX,4to2/8to3encoder,7segment,table/equation -> gate/MUX connections |P4.12, fulladder andencoder end-to-end; reuse boundedBoolean core without pretending8input encoders fit4vars |
| M8 |Parameterized behavioral/structuralFA,encoders,D/T/JK,universalshift,moduloN syncreset,ring |P3.21/P4.20/P4.21/P5.13/P5.20/P5.23; generated code compiles and behaves correctly in host tests |
| M9 |Counter+output joint design,state-table -> equations -> circuit/simulation,P5.26,UI consistency,regressions,hardware measurements,guides/versionedrelease |End-to-end comprehensive fixtures including half-cycle clock-qualified output; formalECEN21.g3a release |

M5 proposal: structured cycle/equation entry -> old-state row generation -> reuse
M4 excitation and M2B minimization -> all result views -> independent row/vector
replay -> explicit connections. DefaultunusedRETURN0, optionalHOLD/DC with clear
recovery guarantees. Common-clock simultaneous updates; maximum16rows/6pin
functions;0-2optional outputs. Higher state-bit support is deferred pending
resource evidence; no core expansion or M3 changes are assumed. Technical stages
M5.1-M5.6 are now approved by the user's “开始M5吧”. Main Agent owns UI, integration and evidence; one isolated portable-model/test subagent owns sequential.[ch], sequential_checks.[ch], its exhaustive driver and design note. Outputs use `.phase1/ecen21-m5/`. See [M5 requirements, tasks and exact
acceptance fixtures](docs/M5-PLAN.md).

Custom bit widths were subsequently explicitly authorized and delivered in the bounded follow-up below. Future Backlog only: sign-magnitude/ones-complement,comparison,
additionalM3 arithmetic, general large-variable synthesis/automaticFSM encoding,
analog delays and graphical circuit editing. These are excluded from M5-M9 and
must not displace the approved-at-the-time problem solver scope.

## Environment and subsequent work

**M4 formally closed after explicit real fx-CG50 user acceptance.** M5/M6 engineering complete; hardware acceptance explicitly deferred. Keep both guides available for later joint testing and stop before M7 without separate approval. M3 remains frozen. Preserve all accepted artifacts/evidence. Physical peak-stack/worst-case-latency measurements remain separate unverified hardware work.

Use `bash scripts/toolchain-build-logic.sh` for verified Logic builds; host `make app APP=logic` still requires a native fxSDK installation. Native installation is an optional environment improvement while official forge endpoints remain unavailable. The user reports accepted M4 hardware operation. Detailed performance/peak-memory measurements remain separate; do not infer them from acceptance.

## M7 design proposal — awaiting explicit approval

The user reaffirmed M5/M6 physical verification deferral and requested an M7 design. [M7 bounded scope, work order and planned acceptance](docs/M7-PLAN.md) is now documented. This is planning only: no M7 source work or new verification results; M3 remains frozen. Proposed modes: adders/popcount,1..8bit ripple structure,2..4input truth/equation gate/MUX implementation,ordinary4to2/8to3 encoders and BCD seven-segment logic. M7 implementation remains unapproved.

## Authorized M3 follow-up — custom1..32bit widths

User request: custom Binary width, maximum32. This overrides the M3 freeze only for this change; no M7 implementation. Engineering passed; user acceptance pending independently of original accepted M3. M5/M6 hardware remains deferred. Added F4 numeric editor with EXE/F6 apply,EXIT cancel,F1/F2/F3 presets; partial-nibble HEX formatting/range guard and signed/unsigned1bit boundaries. See [Chinese guide](docs/M3-CUSTOM-WIDTH.zh-CN.md). Fresh production129,960bytes, SHA256 `b5ea831eb34a9c287c0cb5e4c7de765b4aad70894db300d3366a5085fd7f76cb`;97,433 shared checks and5,941,888 independent host arithmetic checks passed;22custom screenshots/6invariants and prior regressions passed. M6 engineering source/artifacts/evidence/helpers are preserved separately. Next action: user acceptance of this follow-up; do not infer M7 approval.

## M7 explicit authorization — November 12, 2026

User: “批准进入M7”. Implement the bounded scope and tests in `docs/M7-PLAN.md`. This explicit next-stage instruction authorizes M7 without inventing acceptance of the custom-width follow-up or M5/M6 hardware. Those acceptances remain pending/deferred. Preserve all previous cores/UI except launcher integration. Default single Main Agent. M8 remains unapproved. Current M7 engineering status: in progress.

## M7 engineering completion — user acceptance pending

Approved bounded scope delivered:2..4input table/expression through exactSOP/POS,
K-map groups,actual gates and2/4/8:1MUX;HA/FA/popcount;1..8bit ripple stages/wires;
ordinary4/8input encoders with VALID;all7BCD segments/both polarities/explicit
invalid policies. Separate MUX_Sum/MUX_Carry meet P4.12 pin assignments. No old
core/UI source changed except launcher; custom-width source remains intact.

Actual23912shared host/SuperH checks and3861088extended independent host checks
passed;91M7screens/21invariants. Prior59Boolean/51numeric/53M4/99M5/93M6/22width
references match,with previous interaction checks. Full earlier host regressions
andCTest5/5 pass. Fresh production144556bytes, SHA256 `53fb9653735a11cd4e2b7d4ff2bfd96f39559d92f280849d216346371f897a72`.
121source inputs match. Representative family benchmark18165877whole-program
emulated instructions/79peaknodes; static ELF text115416/data464/BSS15840.
These are engineering observations,not physical performance or acceptance.

Delivered [seven-part Chinese guide](docs/M7-USER-ACCEPTANCE.zh-CN.md).
**M7 engineering passed; M7 user acceptance pending; M8 unapproved.** Preserve
M5/M6 hardware deferral and custom-width acceptance independently. Next action:
M7 acceptance/fixes; M8 proposal only until separately authorized.

## M8 explicit authorization

User: “继续做M8”. M8 implementation is authorized under the bounded PRD HDL contract. M7 acceptance remains pending; M5/M6 physical verification stays deferred and custom-width follow-up acceptance stays pending. This user-directed next-stage exception does not fabricate earlier acceptance. Preserve `.phase1/milestones/M7-engineering/` and prior cores/UI except launcher. M9 remains unapproved. Single Main Agent; no subagents requested.

## M8 engineering completion — user acceptance pending

All approved families delivered in isolated portable HDL generator/export modules
and native UI. WIDTH1..32,ordinary4/8input encoder,D/T/JK edge/reset choices,universal
shift priority,modulo1..65535/SYNC reset,safe width/extra bits,ring seed/direction.
All complete sources are pageable and exported as bounded8.3 `.v` files with
explicit confirmation/slot and no overwrite. Shared FF/encoder references reused;
prior engines/UI preserved except launcher. M7 engineering snapshot retained.

Actual92120shared host/SuperH checks;343HDL compiler/simulator cases/280704behavior
checks.17actual production native exports byte-matched and all consumed in a
second343-case matrix.70UIpages/19interaction checks;old reference images pass,
including90M7tool screens. Read-only native storage failure and portable IO fault
checks pass.141source inputs,three fresh target builds/12package checks each;
158376byte user artifact SHA256 `a893609cb6a6757a5b20dbd253495339f3d9c2e6e1a5a13fa7c2ce01480824df`.
Representative36family setups:peak1260source bytes/51visual lines,26415073whole-
program emulated instructions. Static ELF text129224/data464/BSS24304,not physical
peak resources. Existing independent suites andCTest5/5 passed. Icarus13.0 was
installed for actual host HDL verification; pinned fxSDK unchanged.

[Chinese seven-part M8 guide](docs/M8-USER-ACCEPTANCE.zh-CN.md) delivered. M8 user
acceptance pending,M9 unapproved. M7/custom-width acceptance remain pending;
M5/M6 hardware deferred. Next concrete task is user acceptance/fixes; M9 integrated
problem workflows need separate approval. No physical calculator/FPGA result claimed.

## P0 local protection result

Created code checkpoint `840a04d5f9ff9669767d87e9b4f4574eb17d38cd` on
`codex/p0-m8-version-backup`,annotated tag `engineering/m8-recovered-p0`.
Preserved all60original commits;no old milestone history invented. Current source
matches all141M8engineering inputs. Ten shared host suites pass;10recovery
archives all verified/restored into empty directories. Independent Git bundle
clone also passes archive/source/binary verification. Staged188file scan found
no credentials after excluding Finder metadata;5push policy checks passed.
Git fsck passed;working tree clean after code checkpoint. See
[record](tools/versioning/P0-REPORT.json). Browser reauthentication is pending;
remote M8 backup is NOT yet claimed. No new features or M9 work.

## P0 authenticated remote backup — verified

Browser OAuth completed asCapt-cai;personal fork Capt-cai/casioCalcApps grantsADMIN.
Confirmed parentjoserodpt/casioCalcApps;upstream push remainsDISABLED. Origin/main
was still12d6b4e;atomic ordinary fast-forward push published code checkpoint
840a04d5f9ff9669767d87e9b4f4574eb17d38cd to personal main and
codex/p0-m8-version-backup,plusnew annotated engineering/m8-recovered-p0.
No force,history rewrite,remote work overwrite or upstream push occurred.

Independent fresh GitHub clone verified141current M8source inputs,all10recovery
archives and the identical158376byte user binary;its SHA256 remains
`a893609cb6a6757a5b20dbd253495339f3d9c2e6e1a5a13fa7c2ce01480824df`.
This verifies a real remote backup,not just a push command attempt. P0 report
metadata follow-up is committed/pushed separately;the M8code checkpoint hash is
immutable. Earlier401/pending lines describe initial audit,now resolved.
Local bundle also passed independent clone/restore. Permanent task-start Git,
branch,commit,engineering/accepted distinction and safe-push rules are inAGENTS.
All new features remain paused;M9 not started. M5/M6deferred,M7/M8/custom-width
pending;originalM2A-M4user-reported hardware acceptance retained.

## Chinese complete user manual — documentation-only follow-up

User requested a complete Chinese manual for the current M8 engineering build.
Delivered docs/USER-MANUAL.zh-CN.md and a printable 31-page PDF under output/pdf/,
covering all seven tool entries, installation, context-sensitive keys, examples,
limits, export handling, troubleshooting and separate acceptance states.
Added four existing genuine emulator screenshots and a reproducible ReportLab
renderer (Python3 + reportlab + an installed CJK TrueType font; --font override).
No application source, binary, architecture or milestone acceptance changed.
M9 remains unapproved; all new feature development remains paused.

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
