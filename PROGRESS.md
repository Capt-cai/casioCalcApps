# ECEN21 Toolkit progress

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


Updated November 12, 2026 (America/Los_Angeles).

**Phase 1 software verification: COMPLETE.** A fresh unchanged-source Logic build succeeded and its resulting binary passed emulator verification. Native macOS fxSDK installation and physical fx-CG50 validation remain unverified.

## Current milestone — M8 engineering complete; user acceptance pending

Explicit “继续做M8” authorized M8 without inventing M7 hardware acceptance. Delivered
parameterized behavioral/structural adders,ordinary encoder,D/T/JK,universal shift,
modulo-N and ring generators,native code/port pages,confirmed `.v` text export with
slot/no-overwrite/error cleanup. New portable modules/local gint adapter; shared
M4 characteristic/M7 encoder references reused. Existing engines/UI unchanged
except launcher. M7 engineering snapshot preserved. Single Main Agent,no subagents.

Actual92120shared host/target checks;Icarus13.0-g2001 runs343independent HDL cases/
280704checks.17native-production exports exactly match host bytes and all appear
in the exported-source bench matrix.70UIshots/19invariants;old59Boolean/51numeric/
53M4/99M5/93M6/22width/90M7 references match. Numeric5941888,sequential916124,
combinational3861088,exactBoolean153092 andCTest5/5 reruns pass.

Fresh user file `.phase1/ecen21-m8/artifacts/ECEN21.g3a`,158376bytes,SHA256
`a893609cb6a6757a5b20dbd253495339f3d9c2e6e1a5a13fa7c2ce01480824df`.141inputs match;no final compiler warnings/errors;
all12package checks for each of3builds pass. Representative36family benchmark:
26415073whole-program instructions,1260peak source bytes/51visual lines;not a
physical or universal worst-case timing claim. ELF text129224/data464/BSS24304.

[Chinese M8 acceptance guide](docs/M8-USER-ACCEPTANCE.zh-CN.md) ready. M8 user
acceptance pending,M9 unapproved. M7/custom-width pending andM5/M6 deferred states
retained. Actual USB/physical calculator/FPGA behavior unverified. No M9 started.

## Historical M7 engineering completion — user acceptance pending

Explicitly approved by “批准进入M7”. Delivered all bounded combinational families
and connected truth->minimization->gate/MUX->verification workflow,including two
named P4.12 MUX instances and local node scopes. New portable modules/UI; old
computational/UI modules preserved except launcher. No M3 follow-up code change.
Single Main Agent implementation/review/integration; no subagents spawned in M7.

Actual23912shared host/SuperH checks and3861088extended host checks passed;
91production UI screenshots/21interaction invariants. Previous reference images
59Boolean/51numeric/53M4/99M5/93M6/22width match. Independent numeric5941888,
sequential916124,exactBoolean153092 andCTest5/5 pass.121build inputs verified.
User artifact `.phase1/ecen21-m7/artifacts/ECEN21.g3a`,144556bytes,SHA256
`53fb9653735a11cd4e2b7d4ff2bfd96f39559d92f280849d216346371f897a72`. Representative benchmark18165877instructions,
79peaknodes; static ELF text115416/data464/BSS15840; not physical measurements.
Final build without compiler warnings/errors. Initial GCC indentation error and
emulator-script PLUS key spelling corrected; evidence retained in logs.

[Chinese M7 guide](docs/M7-USER-ACCEPTANCE.zh-CN.md) delivered. M7 user acceptance
pending; M8 unapproved. M5/M6 hardware still deferred; custom-width follow-up
acceptance pending,not silently closed. M4 accepted stable checkpoint and prior
engineering snapshots remain preserved. Stop for user acceptance/next authorization.

## Historical custom-width follow-up — engineering passed; user acceptance pending

The user explicitly authorized M3 custom widths up to32. Implemented1..32 with an F4 modal editor and8/16/32 presets, robust partial-HEX range checks, signed extension and truncation feedback. Actual checks:97,433 shared host/target assertions;5,941,888 independent host arithmetic checks;22custom UI images/6invariants plus old tool regressions. Final129,960byte artifact is `.phase1/ecen21-width/artifacts/ECEN21.g3a`; Chinese guide `docs/M3-CUSTOM-WIDTH.zh-CN.md`. Original M3 acceptance is preserved; this follow-up acceptance is pending. M5/M6 hardware remains deferred and M7 is unapproved. M6 engineering snapshot preserved in `.phase1/milestones/M6-engineering/`.

## M6 engineering complete; hardware acceptance deferred

The user explicitly said “开始M6吧 实机验收后面一起做”. M6 is authorized and M5
hardware acceptance deferred, not passed. This is a user-directed exception to
the usual acceptance-before-next-stage rule for M6 only; M7 is unapproved. M5
engineering source/artifact/evidence/helpers are preserved in
`.phase1/milestones/M5-engineering/`. M4 remains the accepted stable checkpoint.
M3 and earlier model/UI source are frozen. Main owns M6 UI/integration/evidence;
one isolated subagent owns the timing core/tests/model note. New outputs go to
`.phase1/ecen21-m6/`. M6 engineering is now complete with actual verification below.

Delivered: editable4inputs/4cells/4gates/64events; D/T/JK/SR plusD/SR latches,
master/slave,derived clocks,sync/async reset,initialX,ideal zero-delay phase model;
4x8wave/table pages,clock/level markers,input before/after,phase snapshots,explicit
unknown versus disabled rows,bounded-feedback errors and retained navigation.
Independent Review product goldens verify ordered events/bit labels and all
intermediate required feedback/reset behaviour. No analog timing promise.

Actual checks: core**15,247** and product presets**381** shared assertions pass
host/SuperH with all six earlier suites (eight passing reports). Main independently
reviewed/retested the isolated engine and added a132-assertion follow-up fix for
possible-edge reset explanations. Timing93screens/19invariants; old pixel matches
59Boolean/51numeric/53M4-script/99M5-engineering and15/22/20older invariants pass.
Existing host numeric2,359,296,sequential916,124 andexactBoolean153,092 plusCTest5/5
pass. GCC UI guard warnings and crowded footer/disabled-display/rail gaps were
resolved before final fresh build. Pure gate cycles and nonconvergent transparent
feedback produce distinct errors without stale waves.

Final `.phase1/ecen21-m6/artifacts/ECEN21.g3a` **129,304 bytes**, SHA-256
`a5ebfb19d0eddf81020cfe2e3778b4b23faf6a83b30cf0907b6f81587cd1e0f8`.
No final build warnings/errors; all target/package/107-input checks pass. Separate
65column/12signal/4cell whole-program benchmark returned after292,929 emulated
instructions including replay/checks/report I/O, not physical latency. ELF
text100,164/data464/BSS10,992; phase engine has no new heap/recursion. Physical
speed,peak memory/stack,keyboard,USB,OS return and stability remain unmeasured.

[Chinese M6 guide with deferred M5 checklist](docs/M6-USER-ACCEPTANCE.zh-CN.md)
is ready for later unified testing. **M5 and M6 engineering passed; both hardware
acceptances deferred, not passed; M7 unapproved.** Stop here; no M7 features started.



## M5 engineering completion (hardware subsequently deferred)

The user approved “开始M5吧”. M4 remains accepted/closed with an immutable stable
checkpoint. The bounded M5 solver is now implemented and engineering-verified;
M3 computational/UI sources and original upstream apps/shared/exact core remain
unchanged. No M6 work is approved.

Delivered: sequence/mixed/control/reverse modes;2-3state bits/optionalE; custom
cycles and79-character equations; RETURN/HOLD/DC with explicit actual completions;
all present/next/excitation/output tables; independent exact per-pin forms;
K-map groups;32-stepE timeline/fixed-E prefix/cycle; all-start recovery/traps;
explicit gate/FF/pin/output wiring and parity sharing. New modules compose the
accepted cores through local adapters, with a fourth launcher entry and retained
problem state. Reverse SR-invalid analysis stops instead of fabricating states.

Two useful independent subtasks completed isolated model/tests and connection/
tests. Main reviewed independent literal/cube/min-cost and emitted-gate-language
oracles, reran all checks, implemented UI and target integration, and visually
reviewed representative pages. Shared host/SuperH **2,216 sequential +128,986
connections +3,849 FF +20,779 numeric +6,516 Boolean +218 legacy** assertions pass;
host exhaustive **916,124** assertions cover5,408 models; numeric2,359,296,
exactBoolean153,092 cases andCTest5/5 pass. UI **100 M5 screenshots/20 invariants**,
old numeric15/FF22 invariants and archived pixels59Boolean/51numeric/53M4-script
pages pass. Menu cancel/help now retain nonzero page, covered by explicit checks.

Fresh `.phase1/ecen21-m5/artifacts/ECEN21.g3a`, **114,596 bytes**, SHA-256
`3cf6c16f69e8517e09ebaf0949e3fd739bb08f2fc31cbe6a25e93f6433dac97f`. Final builds have no compiler/Make
warnings/errors; target/package/93-input provenance validated. Maximum-domain
separate program passed16 rows/32 form-row checks/28 nodes in534,076 emulated
whole-program instructions; no physical latency claim. Compiler ELF85,456text/
464data/8,720BSS bytes; stack estimates and limits are inTESTING.

Iteration issues resolved: GCC guard-format warnings; emulator's500ms script-tail
cutoff before fixtures finish (pending bounded event now keeps it alive, report
and actual program return required);512-step/8192path-byte script limits (three
independent scenarios); revised Gray labels/invalid excitation/output pagination;
help/cancel page retention. Failed/initial build evidence is retained where noted;
no failed/missing report is treated as success.

[Chinese M5 acceptance guide](docs/M5-USER-ACCEPTANCE.zh-CN.md) gives installation,
exact keys/inputs/results, boundaries/errors and failure reporting. **M5 engineering
passed; user acceptance pending; M6 approval not granted.** Stop at this handoff;
no physical calculator was operated by the agent. No inferred hardware speed,
peak memory or long-run stability results.

## M4 closure and M5 planning checkpoint (subsequently approved)

The user explicitly reported that M4 passed realCasio fx-CG50 acceptance and
formally approved closure. Phase1/M2A/M2B/M3/M4 are accepted. No new physical
measurements were supplied and the agent did not operate the device. The
unchanged93,804-byte M4 binary,77-input source copy,original helpers and test/UI
evidence are archived as [stable checkpoint](docs/checkpoints/M4-ACCEPTED.md).
Historical engineering reports remain immutable; separateacceptance.json records
later hardware acceptance. This is a local archive, not a Git tag/remote release.

Planning-only work: inspected the supplied41-page Exam#2 Review (including rendered
circuit/waveform/code pages); created [PRD](PRD.md), [problem coverage matrix](docs/PROBLEM-COVERAGE.md)
and [detailed M5 proposal](docs/M5-PLAN.md). Product route is M5counter/sequential,
M6timing,M7combinational,M8Verilog,M9integration/release. NoM5 source implementation,
M3 changes, new app build or architectural migration was performed. User-requested
custom bit widths/otherM3 improvements are FutureBacklog only, outsideM5-M9.

M5 recommends2-3state bits plus0-1E (max4 Boolean variables), mixedD/T/JK/SR,
explicit unused-state policies and reverse input/output equations. Mandatory
fixtures include all review counters, simultaneous-update traps, recovery/DC
behaviour, exact minimization/maps and connection descriptions. Review overbars
and state-bit ordering were visually checked; no solver coverage was promoted
merely because a reusable engine exists. Requirements/test cases are proposed,
not passed implementation tests. Integrity review passed:77 build inputs,438
unchanged application/core/test/helper files,299 archive member hashes and45 local
links. SeeTESTING for checkpoint/document checks.

**M4 engineering: passed. M4 real-device user acceptance: passed; closed.
M5 implementation approval at that historical checkpoint: NOT granted.** This was subsequently superseded by “开始M5吧” and the current M5 engineering completion above.

## M4 engineering completion record (user subsequently accepted and closed)

The user explicitly stated “M3验收完成 开始M4”: M3 user acceptance passed and M4 approval granted. The scoped flip-flop plan has now passed engineering verification. Accepted M3 source/artifact/evidence and original helpers were archived under `.phase1/milestones/M3/` before integration changes. Main Agent owns the UI, integration and verification; one independent subagent owns the portable flip-flop model/tests/design note. New outputs are isolated under `.phase1/ecen21-m4/`. M5 remains unapproved.

Delivered four flip-flop types, complete characteristic/excitation tables, selected rising/falling active edge and NONE/RISE/FALL events, preview separate from stepping, explicit SR-invalid state/lock, bounded16-entry history with immutable edge/event snapshots, initial-Q restart, help and retained state across three tools. Existing Boolean/numeric views/models and original upstream applications/shared code remain unchanged.

Actual verification: **3,849 flip-flop +20,779 numeric +6,516 Boolean +218 legacy** shared assertions passed both sanitized host and SuperH execution, with identical reports. Independent numeric oracle **2,359,296** assertions, exact Boolean **153,092** cases and emulator CTest **5/5** passed. Production **54 flip-flop screenshots/22 invariants** and **54 numeric/navigation screenshots/15 invariants** passed; all59 Boolean pages and51 numeric-script tool pages match accepted archived pixels. Representative tables, state/error/lock/recovery/history/help/selector screens were visually reviewed. Initial verifier correctly refused stale build provenance after a test-runner output path changed; final clean rebuild and complete verification passed.

Fresh M4 artifact: `.phase1/ecen21-m4/artifacts/ECEN21.g3a`, **93,804 bytes**, SHA-256 `165a5a2230b075352046e0f666b9b9d981d47b99c69af06443373fcadefcff6f`. Final compiler/Make/header/manifest checks passed. ELF text64,664/data464/BSS2,672; step68-byte compiler frame, flip-flop UI120-byte frame. No physical peak/latency claim is made. [Chinese M4 acceptance guide](docs/M4-USER-ACCEPTANCE.zh-CN.md) provides exact install/keys/results and failure reporting. **M4 engineering passed; hardware user acceptance subsequently passed and closure approved; M5 implementation unapproved.** Original results are retained; see current closure/checkpoint record above.

## M3 engineering complete; user subsequently accepted

The latest user instruction “允许进入M3” explicitly authorizes the scoped numeric milestone, superseding the prior plan-only choice. M2B is user accepted. Its source/binary/evidence and helper snapshots are preserved under `.phase1/milestones/M2B/`; M3 output uses `.phase1/ecen21-m3/`.

One independent numeric-model/test subagent completed its owned files; Main Agent reviewed the code, independently reran the suites, implemented launcher/numeric views, extracted Boolean UI without changing its computations, and verified integration. No upstream Logic/Base-N/common or exact-engine modifications were needed.

Delivered: 8/16/32-bit DEC/HEX/BIN input/conversion; signed/unsigned reinterpretation; two's-complement negatives; A/B addition/subtraction; distinct carry/borrow/signed-overflow and selected-mode wrapping; sign/zero extension; meaningful truncation warnings; explicit invalid/empty/range errors with stale-output clearing; eight numeric samples; tool selector preserving both tools' in-memory states. Module EXIT returns to selector; selector EXIT returns to calculator. No disk persistence or out-of-scope64-bit/multiply/divide/shift features were added.

Verification passed: shared **20,779 numeric +6,516 Boolean +218 legacy** checks on both sanitized host and SuperH emulator; **2,359,296** independent host arithmetic-oracle assertions (all8-bit pairs/operations/modes plus sampled16/32); Boolean exact regression **153,092** cases; emulator CTest **5/5**. Production generated **54** numeric/navigation screenshots with **15** interaction invariants, and **59** Boolean screenshots exactly matched the accepted M2B reference. Representative input/result/error/negative/32-bit/truncation/selector pages were visually inspected.

Fresh output: `.phase1/ecen21-m3/artifacts/ECEN21.g3a`, **87,652 bytes**, SHA-256 `f309877c3dcd030bd452f11a97d53ab7ccf651d0fb0dcb35706e7f9406b39aed`. Final build has no warnings/errors and both add-ins pass packaging checks. ELF text=58,512 bytes, data=464, BSS=2,048. Numeric parse/format/solve frames are64/84/40 bytes; numeric view128, launcher44; these are compiler observations, not measured physical peaks. The numeric model uses no heap/recursion.

Accepted M2A/M2B and Phase1 evidence are preserved; current build aliases route to M3 output so earlier delivery folders are not overwritten. Handoff: [M3 Chinese acceptance guide](docs/M3-USER-ACCEPTANCE.zh-CN.md). **Engineering verification: passed. M3 user acceptance: subsequently passed. M4 approval: subsequently granted.** This preserves the M3 delivery record; current M4 status is above. Hardware OS return, speed, stability, USB and peak stack remain unmeasured by the agent for this version.

## M4 planning-only checkpoint

The user requested an M4 plan after M3 delivery. `PLAN.md` now proposes single-bit SR/D/JK/T characteristic/excitation tables and bounded edge-based simulation, with explicit invalid-state handling, exclusions, stages and acceptance fixtures. At that checkpoint no M4 implementation/tests were started, M3 acceptance was pending and M4 unapproved. This historical plan-only state was subsequently superseded by explicit M3 acceptance/M4 approval and the completed engineering work above.

## M2B engineering completion record (user subsequently accepted)

The user explicitly confirmed M2A acceptance and approved M2B ("M2A验收通过 批准进入M2B"). M2A engineering evidence is preserved under both `.phase1/ecen21/` and `.phase1/milestones/M2A/`; no new physical measurements are inferred from this acceptance statement.

M2B acceptance criteria are in `PLAN.md`. A single portable-model/test subagent owns solver/parser and its fixtures; Main Agent owns native UI, integration, build verification and handoff. The exact minimizer, original Logic/shared sources and checked-in binaries remain unchanged. Fresh M2B artifacts/evidence use `.phase1/ecen21-m2b/` to preserve M2A.

Delivered: 79-character expression/cursor editor; syntax diagnostics; active 2/3/4 variables; editable 0/1/X truth table and synchronized Gray-order K-map; exact SOP/POS with cared-row verification; all-group navigation and cube-to-term/clause explanations; term/clause-aware long-result wrapping; samples/help and exit. EXPR/MANUAL source is explicit, and navigation cannot silently restore saved text. Manual variable shrink retains low rows only, while expression variable changes re-evaluate text.

The independent subagent completed portable model/parser and tests; Main Agent reviewed/integrated them, added six malformed maximum-depth cases, reran the sanitized host suite, implemented native views and verified SuperH execution and screenshots. Final checks: **6,516 model + 218 legacy checks** pass on both host and target emulator; **153,092** exact-core cases pass; **59** production screenshots and **16** interaction invariants pass; emulator CTest **5/5**. Representative screens including errors, 2/3/4-variable maps, wraparound groups, eight-group navigation, all-DC and long POS were visually inspected. Final build has no compiler/Make warnings/errors, and both add-ins' header checks pass.

M2B deliverable: `.phase1/ecen21-m2b/artifacts/ECEN21.g3a`, **80,124 bytes**, SHA-256 `05cca11650ee7ebe21debf62f2a471bc6a5df2f1a2ce03bfbf19d233e5de15c8`. Resources: ELF text=50,984 bytes, data=464, BSS=1,856. Main frame=224 bytes; parser/minimizer estimates are detailed in `TESTING.md`. Tests execute bounded deepest malformed expressions on SuperH; physical peak stack/latency remain unmeasured.

M2A original/archive binaries and Phase 1 evidence remain intact. The generic `toolchain-build-ecen21.sh` now forwards to the M2B build driver so it cannot overwrite the accepted M2A output path; the original M2A helpers are archived under `.phase1/milestones/M2A/reproduction/`. No upstream Logic/common/exact-engine changes were made.

**M2B engineering verification: passed. M2B user acceptance: explicitly passed.** The prior M3 planning-only checkpoint was superseded by the explicit approval and current M3 results above. The [Chinese M2B acceptance guide](docs/M2B-USER-ACCEPTANCE.zh-CN.md) is retained with prior evidence. This historical paragraph preserves the planning-only handoff; current authorization/results are recorded above.

## M2A engineering completion record (user subsequently accepted)

Milestone 2A finished on November 12, 2026. Work used one Main Agent, consistent with the economical workflow; no subagents or new native-toolchain research were needed.

- Added repository `AGENTS.md` covering communication language, milestone approval, autonomous execution, economical delegation, verification and preservation rules.
- Added `apps/ecen21/`: native gint UI, portable input/controller, custom menu icons, user documentation, shared host/target controller fixtures and emulator key script.
- The app directly links the existing verified Boolean engine in `experiments/boolean/`; no algorithm copy, original Logic changes or shared-source changes were required.
- Users can enter/edit a four-digit hexadecimal ON-mask, compute exact SOP, load the known counterexample or constants, inspect cost, open About and exit. Each successful target calculation verifies all 16 truth-table rows. POS/DC editing, expression editor, tables and K-maps remain outside 2A.
- Fresh `ECEN21.g3a`: **72,532 bytes**, SHA-256 `a1a9705e202ca0e33329b1e0c899e13084c5899e9583b29b164a8bc4b0a6fbc5`, at `.phase1/ecen21/artifacts/ECEN21.g3a`. The pinned ARM64 Docker route built production and a separate test-only add-in successfully.
- Host exact-core regression passed 153,092 minimizations / 2,237,318 cared-row checks with ASan/UBSan. The controller passed 218 sanitized host checks and the identical 218 checks executing as SuperH code in the emulator, with its output file checked against the passing host report.
- Production emulator smoke passed: 12 screenshots, six automated interaction invariants, normal exit and visual layout review. `007E` actually computes `A'C'D+A'BD'+A'B'C` (three terms/nine literals); `6996` shows eight terms/32 literals with readable wrapping. Emulator infrastructure CTest passed 5/5.
- The initial bind-mounted build emitted Make clock-skew warnings. Building on Linux tmpfs resolved them; the final clean build has no compiler or Make warnings/errors. The known emulator trailing-unused-key diagnostic remains documented.
- Static target estimates: text 43,392 bytes, data 464, BSS 1,280; main 104 bytes, controller solve 36, minimizer 892 and recursive visit 112 bytes/frame. Physical peak stack, worst-case latency, OS return and USB remain unverified.
- Hash verification confirms tracked upstream Logic/common files and all existing distribution binaries are unchanged. Phase 1 artifacts/evidence were preserved. See `TESTING.md` and `apps/ecen21/README.md` for reproduction.

**Engineering verification: passed. User acceptance: explicitly passed. M2B approval: explicitly granted.** Added the permanent seven-part Chinese acceptance-guide policy to `AGENTS.md` and delivered [the M2A physical-calculator acceptance guide](docs/M2A-USER-ACCEPTANCE.zh-CN.md). Its installation instructions were checked against Casio's official add-in installation guide and its application expectations against the actual code/test evidence. The existing artifact's SHA-256 was reconfirmed; no application rebuild or new physical test was performed for this documentation change. Prior evidence is preserved.

The user has now provided both required confirmations, so work advances only to M2B. Its completion will require a new acceptance handoff and approval gate.

## Baseline

- Repository revision: `12d6b4e1086746534636c3bb87703deeef5734a8`.
- Existing Phase 1 changes were preserved: report, host audit script, emulator helper and `.phase1/` ignore rule.
- The earlier prebuilt Logic demo and five emulator infrastructure tests passed; those results do not demonstrate a source build.
- Exactly two subagents were actually launched in parallel: Toolchain Engineer and Algorithm Engineer. Their files have separate ownership; the lead owns integration and status documentation.
- Before delegation work, the lead recorded SHA-256 baselines for tracked Logic/shared files and the distribution binary under `.phase1/lead/`.

## Phase 1 verified results (historical record)

The following records describe the completed Phase 1 handoff; the 2A section above supersedes its earlier calculator-integration status.

Agent A resolved the source-build blocker with a complete pinned **ARM64 Linux container on Docker Desktop**. Tool versions: fxSDK 2.11.0, GCC 14.1.0, binutils 2.42, gint 2.11.0 and fxlibc 1.5.1. The image manifest is `manawyrm/fxsdk@sha256:292176e823e272b87b74f3e87b0c2ee26d4ada17abec017e98002c207a456227`. Source copy, compiler host and SuperH target were separately verified. A fresh build succeeded without compiler warnings/errors; header checks passed. The original distribution binary was preserved.

Fresh deliverable: `.phase1/toolchain/artifacts/Logic.g3a`, **85,716 bytes**, SHA-256 `100b83beaa5dd6051cebdecfb816df8946d60bd6afa66ea884691ca56fd98b25`. Both Agent A and the lead ran this final artifact in the native arm64 emulator: exit 0, all five screenshots exactly matched reference pixels. The lead also visually inspected fresh expression/K-map output. The initial amd64-container build was separately successful; ARM64 is the preferred verified route.

Agent B finished an isolated portable C exact minimizer in `experiments/boolean/`, with SOP, POS and don't-cares for 2–4 variables. The lead reviewed the search completeness, prime restriction, polarity and pruning rules, then independently reran its sanitized suite: **153,092 minimizations and 2,237,318 cared-row checks passed**. The known `0x007e` counterexample yields three terms/nine literals, matching the lead's independent all-cube subset calculation. Full four-variable don't-care coverage is sampled, not exhaustive. Calculator integration and target resource measurements remain pending.

The lead also reran the existing Logic audit (65,808 equivalent covers, 2,288 nonminimal four-variable covers) and the five available emulator tests successfully. The new exact core compiled cleanly to SuperH with strict warnings: 1,524 text bytes, zero data/BSS, static stack estimates 892 bytes for `eb_minimize` and 112 bytes per recursive frame. No calculator application links this experimental core yet.

Both agents finished before final integration. The lead reviewed their code/design and confirmed all 49 isolated build inputs matched workspace originals. Tracked Logic/shared source and the checked-in prebuilt still match the entry hash baselines. Integrated additions are the isolated experiment, build/verification helpers, detailed design/toolchain reports and these project status documents; no ECEN21 UI or existing Logic feature changes were made.

## Remaining work after 2A (historical; current state above)

- Full native macOS fxSDK/GCC installation is unverified; official forge HTTP requests still time out. The working container route avoids this blocker for application source builds.
- Physical fx-CG50 execution, OS return and USB operation remain untested.
- Exact-core peak stack and worst-case latency need calculator runtime measurements; all four-variable ternary specifications were not exhaustively tested.
- The existing Logic app retains its original greedy algorithm. ECEN21 M2B exposes the complete bounded expression/table/K-map/SOP/POS/DC workflow; the user has accepted this version.

**Next task:** keep M5/M6 guides/artifacts for deferred joint hardware testing; fix reported defects and rerun affected checks. Require separate M7 approval before further milestone implementation; M3 remains frozen. See [PLAN.md](PLAN.md).

Detailed initial findings remain in [the historical Phase 1 report](docs/ECEN21-PHASE1.md). Current evidence is in [TOOLCHAIN.md](docs/TOOLCHAIN.md), [BOOLEAN-MINIMIZATION.md](docs/BOOLEAN-MINIMIZATION.md) and [TESTING.md](TESTING.md).

## M7 planning update — November 12, 2026

Recorded the user's renewed M5/M6 hardware deferral. Prepared [M7 design proposal](docs/M7-PLAN.md), including inputs/outputs,bounds,shared-core adapters,small-screen views,exam fixtures and acceptance tests. No M7 implementation/build/tests performed; M7 remains unapproved. Prior M6 delivery manifests remain snapshots of that engineering handoff.

## M7 explicit authorization — November 12, 2026

User: “批准进入M7”. Implement the bounded scope and tests in `docs/M7-PLAN.md`. This explicit next-stage instruction authorizes M7 without inventing acceptance of the custom-width follow-up or M5/M6 hardware. Those acceptances remain pending/deferred. Preserve all previous cores/UI except launcher integration. Default single Main Agent. M8 remains unapproved. Current M7 engineering status: in progress.

## M8 explicit authorization

User: “继续做M8”. M8 implementation is authorized under the bounded PRD HDL contract. M7 acceptance remains pending; M5/M6 physical verification stays deferred and custom-width follow-up acceptance stays pending. This user-directed next-stage exception does not fabricate earlier acceptance. Preserve `.phase1/milestones/M7-engineering/` and prior cores/UI except launcher. M9 remains unapproved. Single Main Agent; no subagents requested.

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
