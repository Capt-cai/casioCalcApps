# ECEN21 Toolkit testing

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


Tests run on macOS Apple Silicon. A host test or prebuilt emulator run is not evidence of successful calculator source compilation.

## M8 engineering verification — passed; user acceptance pending

Explicit user “继续做M8” authorizes this stage without marking M7/custom-width
accepted or closing deferred M5/M6 hardware. M9 unapproved. Single Main Agent;
prior M7 source/artifact/evidence/helpers frozen in `.phase1/milestones/M7-engineering/`.
No physical calculator/USB/FPGA verification was performed.

Reproduce from repository root; Icarus13.0 host dependency installed for M8:

```sh
bash apps/ecen21/tests/run-host.sh
bash apps/ecen21/tests/run-hdl.sh
bash scripts/toolchain-build-m8.sh
/opt/homebrew/bin/python3.11 scripts/verify-m8.py
ctest --test-dir .phase1/emulator-build --output-on-failure
```

Source/language/export semantics: [M8 model](docs/M8-MODEL.md). As-built user guide:
[Chinese M8 guide](docs/M8-USER-ACCEPTANCE.zh-CN.md).

| Check | Actual result |
| --- | --- |
| User artifact |`.phase1/ecen21-m8/artifacts/ECEN21.g3a`,158376bytes |
| SHA256 |`a893609cb6a6757a5b20dbd253495339f3d9c2e6e1a5a13fa7c2ce01480824df` |
| Shared host/SuperH |**92120M8assertions**,plus all nine prior suites; ten matching passing reports |
| C verification |Address/undefined sanitizers,strict host warnings;actual target generation/paging/IO mock execution |
| Generated HDL host tests |Icarus13.0,Verilog-2001-g2001;**343cases/280704checks** |
| Production target exports |**17actual `.v` files** from native UI/BFile,byte-identical to matching host emission |
| Recompiled exported HDL |Second343case/280704check matrix consumes all17exports;per-file source use counts recorded,other variants still host-generated |
| Native M8 UI |**70screenshots/19invariants**,including read-only-storage create failure;representative visual review |
| Previous exact pixels |59Boolean/51numeric/53M4-script/99M5/93M6/22width/**90M7**;launcher home intentionally changes |
| Existing interaction invariants |15numeric/22FF/20sequential/19timing/6width/21combinational pass |
| Earlier independent host reruns |5941888numeric;916124sequential/5408models;3861088combinational;153092Boolean minimizations/2237318cared rows |
| Infrastructure |CTest5/5 |
| Fresh builds/provenance |Production/shared-test/M8benchmark;12package checks each;141copied inputs and complete workspace input set match;no final compiler warnings/errors |
| Representative maximum settings |36configs across9families/clock/polarity;peak1260source bytes/51visual lines;**26415073whole-program emulated instructions** |

Portable checks cover every width1..32,clock/reset variants,even source size,
line wrapping with an independent reference,invalid width/modulus/seed/form,
filename capacity/slot,stale config,odd/malformed export source,existing-file
protection and injected lookup/create/open/short-write/write/close/cleanup errors.
File operations only remove a newly created failed file,never previous files.
Code is NUL-free and even-sized per pinned gint BFile requirement. Selected
module/config matching excludes only export slot. No new heap/recursion.

Independent Verilog benches use Python wide integer adder goldens,all16/256encoder
patterns,FF edge/hold/reset/unknown and JK known-state transitions,all shift request
combinations/serial data,modulo1/2/3/12/31/65535 and ring1/2/5/8/32 widths/directions/
seeds. Explicit overrides and invalid-parameter guards are compiled/simulated.
No-resetT unknown state is tested rather than fabricated as0. Binary input
coverage is not exhaustive arbitrary four-state HDL semantics or FPGA synthesis.
Icarus13prints informational `$finish called...` lines; benches require PASS and
reject FAIL/incorrect guards,not an exact entire stdout match.

Native scripts `m8-a/b/c/d.script` generate/export defaults and1bit/8input/negative-
edge/asynchronous-low/reset-value/ring-seed variants. Existing ECAB00.v remains
its original5bit source after a1bit overwrite attempt. Cancel/help/reentry retain
state; edits invalidate code/export. Metadata `m8-exports.json` supplies exact
expected parameters/file set. A second bench matrix reads actual exports where
compatible and records uses per file;it does not claim every343test used every
file. Read-only emulated storage produces visible create failure(IO-5),no `.v`
file,and permissions restored afterwards. Mock create-space failure covers the
portable error branch;actual storage exhaustion and USB remain physical tests.

Shared target returns after1195565830instructions,including ten full suites and
report I/O. Separate representative36config generation/paging program returns
PASS after26415073instructions,including startup/checks/report. These are emulated
whole-program counts,not pure generation latency,seconds,physical measurements or
universal worst-case bounds. Source cap8192 includesNUL;1260is the observed peak
for representative bounds,not a claimed maximum for every setting combination.

Production static ELF:text129224/data464/BSS24304bytes. Individual SuperH frames:
hg_generate72,hg_line20,export48,OS adapter36,HDL UI176bytes;callee formatting/gint/
OS/interrupt/runtime/VRAM allocations omitted. No physical peak-resource claim.
Existing recursive parser/minimizer code is unchanged. No physical calculator,
USB/OS return,long-run stability or FPGA synthesis/place-route/timing check.

Installed native ARM64 Homebrew icarus-verilog13.0 for M8 HDL tests;fxSDK remains
the same pinned image. No shell profile or native fxSDK installation. Primary
language flag reference: [official Icarus flags](https://steveicarus.github.io/iverilog/usage/command_line_flags.html).
Initial verifier iteration stopped on duplicate screenshot name,renamed before
final success. Icarus13finish diagnostics were accommodated without weakening
behavior checks. Application source compiled successfully;no unresolved build
blocker. Iteration evidence retained separately from final results.

Earlier independent suite reruns use isolated copies of original runners with
ROOT/OUT changed in `.phase1/ecen21-m8/host-tests/run-*.sh`;exact core ran unchanged.
Evidence: `.phase1/ecen21-m8/{source-manifest.json,verification.json,hdl-verification.json,
hdl-export-verification.json,final-review.json,delivery-source-manifest.json}`,
`logs/`,`host-tests/`,`hdl-verification/`,`hdl-export-verification/`,
`verification/{target,benchmark,hdl-a,hdl-b,hdl-c,hdl-d,hdl-exports,storage-failure}/`
and final contact sheets. Artifact hashes and original checkpoints remain intact.

## M7 engineering verification — passed; user acceptance pending

User explicitly approved “批准进入M7”. All M7 checks below actually ran on this
Apple Silicon Mac with the pinned ARM64 image and native emulator; no physical
calculator was operated. M5/M6 hardware remains deferred,custom-width follow-up
acceptance pending,M8 unapproved. See [model](docs/M7-MODEL.md),[plan](docs/M7-PLAN.md)
and [Chinese guide](docs/M7-USER-ACCEPTANCE.zh-CN.md).

```sh
bash apps/ecen21/tests/run-host.sh
bash apps/ecen21/tests/run-combinational.sh
bash scripts/toolchain-build-m7.sh
/opt/homebrew/bin/python3.11 scripts/verify-m7.py
ctest --test-dir .phase1/emulator-build --output-on-failure
```

| Check | Actual result |
| --- | --- |
| Production artifact |144556bytes, `.phase1/ecen21-m7/artifacts/ECEN21.g3a` |
| SHA256 |`53fb9653735a11cd4e2b7d4ff2bfd96f39559d92f280849d216346371f897a72` |
| Shared host/SuperH M7 |**23912** assertions; exact matching passing report |
| Extended host M7 |**3861088** assertions under address/undefined sanitizers and strict warnings |
| Earlier shared suites |218controller,6516Boolean,97433numeric,3849FF,2216sequential,128986connections,15247timing,381presets; all nine reports match |
| Native M7 UI |**91screenshots,21interaction invariants** |
| Previous exact pixels |59Boolean,51numeric,53M4-script,99M5-engineering,93M6-engineering,22custom-width; previous15numeric/22FF/20sequential/19timing/6width invariants pass |
| Independent earlier host suites |5941888numeric arithmetic;916124sequential/5408models;153092Boolean minimizations/2237318cared rows |
| Emulator infrastructure |CTest5/5 |
| Fresh target builds |Production/shared-target/M7benchmark;12package checks each; no final compiler/Make warnings/errors |
| Provenance |121source inputs and complete copied-file set match workspace; source/artifact preservation audit recorded |
| Representative maximum families |4variables/8:1MUX,8inputencoder,all7segments/polarities/policies,8bitadder;**18165877 whole-program emulated instructions**,peak79nodes |

Shared fixtures independently replay emitted gates and each MUX data path,check
cofactor formulas,literal P4.12 pin functions,adder/count truth,all16/256encoder
inputs/validity,independent a-g decimal glyph strings,both polarities/all invalid
policies,48deterministic4var/DC fixtures,all-DC completions,invalid settings/
expressions and every ripple carry connection. Host extension exhausts every
2/3input Boolean function,legal ordered select set/MUX size and both forms; every
operand/Cin combination for each1..8bit ripple width is checked per stage against
wider integer arithmetic. This is not exhaustive4variable mapping configuration
testing; the unchanged independent minimizer suite separately covers all65536
fully specified4variable functions and randomizedDC specifications.

Production scripts `m7-a/b/c/d.script` exercise all families,actual P4.12 data
pins,SOP/POS,K-map group changes,8:1MUX,79/80character boundary/cancel,zero/multiple
encoder cases,paging through256rows,seven output selection,both polarities and
zero-pin/blank/DC differences,ripple error/recovery,duplicate/too-many selectors,
invalid expression stale-result removal,manualDC completion,help/cancel,reentry,
custom-width/timing coexistence. Main visually reviewed representative full-size
screens/contact sheets; pixel invariants complement mathematical fixtures and do
not claim OCR correctness of every character or physical UI verification.

The shared target program returns after304873139emulated instructions,including
all old/new suites,report I/O and startup. The representative benchmark returns
PASS after18165877instructions,including initialization,all family setups/checks
and BFile output. Neither is pure solver latency,real seconds,physical timing or
an arbitrary-function universal worst-case bound. Nodes are bounded512;79 is the
observed peak in that representative test,not a proven global maximum.

Production static ELF text115416/data464/BSS15840bytes. Target frame estimates:
cb_solve844,graph evaluator556,combinational UI236,table260,MUXview352bytes. These
are individual frames,not total stack/heap/VRAM/interrupt or measured peak usage.
No heap/new recursion introduced; existing parser/minimizer recursion preserved.
Physical keyboard/display/USB/OSreturn,latency/peakresources/long-run stability
remain unverified. Known trailing unused-key diagnostics occur after actual
program return; verifier requires program return/report matching and rejects
headless script-tail termination.

M6 and custom-width engineering snapshots/helpers/evidence are preserved. Old
product models/UI match the width snapshot except launcher; upstream/common/
exact-core and accepted artifacts remain unchanged. New outputs are isolated in
`.phase1/ecen21-m7/`. Initial build failed two GCC misleading-indentation errors;
fixed without disabling warnings. One emulator script used unsupported ADD key
name; corrected to PLUS. Iteration logs remain distinct from final success.

Independent earlier regression reruns used isolated copies of the existing
numeric/sequential host scripts with only ROOT/OUT locations changed; copies are
in `host-tests/run-{numeric,sequential}.sh`. ExactBoolean test script ran unchanged,
its final report copied to `logs/exact-core-host.log`. No dependency/toolchain
installation or architecture rewrite occurred.

Evidence: `.phase1/ecen21-m7/{source-manifest.json,verification.json,final-review.json,
delivery-source-manifest.json}`, `host-tests/`, `logs/`, `artifacts/`,
`verification/{target,benchmark,comb-a,comb-b,comb-c,comb-d}/` and contact sheets.

## Custom Binary width follow-up — engineering passed; user acceptance pending

User explicitly requested custom width up to32; this overrides only the bounded
M3 freeze. M7 remains unapproved, M5/M6 physical verification remains deferred.
M6 original engineering inputs/artifacts/evidence/helpers are frozen under
`.phase1/milestones/M6-engineering/`; accepted M3/M4 binaries remain unchanged.
New outputs are isolated in `.phase1/ecen21-width/`.

Reproduce current checks:

```sh
bash apps/ecen21/tests/run-host.sh
bash apps/ecen21/tests/run-numeric-exhaustive.sh
bash scripts/toolchain-build-width.sh
/opt/homebrew/bin/python3.11 scripts/verify-width.py
```

Actual checks: all eight host/SuperH reports match, including **97,433 numeric
assertions** (previous20,779 expanded), with sanitizers/strict warnings on host.
1..32 widths exercise independent wider-integer arithmetic, DEC signed/unsigned
range, BIN/HEX round trips, partial-nibble masks, truncation and signed extension.
Direct review goldens:4bit -5=1011;5bit -8+-9=01111 with overflow;
10bit478/-281/-2 and12bit73/1906/-95/-63. Invalid0/33/64 widths rejected.
The parser previously subtracted a digit from an unsigned small limit; now it
explicitly rejects digit>limit before subtracting, essential for1..3bit ranges.
HEX digit bounds/padding use ceil(width/4), while numeric value must fit exact bits.
Independent host arithmetic **5,941,888** assertions exhaust all operand pairs
for1..8bits and sample9..32bits across add/subtract and signed/unsigned modes.

Native production UI: **22 custom-width screenshots /6 interaction invariants**;
width0/33 visible rejection,EXIT cancel preserves complete result,custom5/1/31/32,
legacy presets,non-nibble HEX rejection,too-long BIN,carry/signed overflow,
reentry retention and representative full-size visual review. Existing M3 script
now explicitly selects preset keys in the modal; its51accepted tool images and
15interaction invariants still match. Boolean59,M4-script53,M5-engineering99 tool
references and their prior relationships pass. M6's93screens/19invariants pass.
No old nonnumeric computational/UI source changed. Production/test/benchmark
fresh builds pass12package checks each; source manifest contains108 inputs.

Delivered user program: `.phase1/ecen21-width/artifacts/ECEN21.g3a`,129,960bytes,
SHA256 `b5ea831eb34a9c287c0cb5e4c7de765b4aad70894db300d3366a5085fd7f76cb`.
Static target ELF text100,820/data464/BSS10,992 bytes; no final build warnings/errors.
Existing timing maximum-domain fixture still passes. No physical performance,
resource peak,USB/keyboard/OS-return verification or user acceptance is claimed.

Evidence: `source-manifest.json`, `host-tests/{results,numeric-exhaustive}.log`,
`verification.json`, `verification/{target,width,numeric,tim-a,tim-b,tim-c}/`,
`logs/build.log`, final source/artifact review under the isolated output folder.
The [Chinese guide](docs/M3-CUSTOM-WIDTH.zh-CN.md) contains exact user procedures.

## Milestone 6 engineering verification - passed; M5/M6 hardware deferred

Authorization: “开始M6吧 实机验收后面一起做” explicitly approves M6 while deferring
M5 hardware acceptance. This is a user-directed exception to the usual entry
gate, not a fabricated acceptance. M7 remains unapproved. M5 engineering source/
artifact/evidence/original helpers are frozen under `.phase1/milestones/M5-engineering/`;
M4 remains the accepted stable checkpoint. No physical calculator was operated.

Actual checks ran November12,2026 with the existing pinned ARM64 toolchain/native
emulator. Reproduce from current source:

```sh
bash apps/ecen21/tests/run-host.sh
bash apps/ecen21/tests/run-sequential-exhaustive.sh
bash apps/ecen21/tests/run-numeric-exhaustive.sh
bash experiments/boolean/run-tests.sh
bash scripts/toolchain-build-m6.sh
/opt/homebrew/bin/python3.11 scripts/verify-m6.py
ctest --test-dir .phase1/emulator-build --output-on-failure
```

The Pillow-enabled interpreter is required for screenshots. Current/historical
build aliases forward to isolatedM6 outputs; original helpers are archived.
Stale artifacts are cleared before fresh copy/build, Linux tmpfs avoids bind
clock skew, manifests/hash/header reports identify actual inputs/artifacts.
Test/measurement entrypoints are excluded from the production program.

| Check | Actual result |
| --- | --- |
| User artifact | `.phase1/ecen21-m6/artifacts/ECEN21.g3a`, **129,304 bytes** |
| SHA-256 | `a5ebfb19d0eddf81020cfe2e3778b4b23faf6a83b30cf0907b6f81587cd1e0f8` |
| Fresh builds | Production,shared-target,maximum-domain compile/link/package; no final compiler/Make warnings/errors |
| Target/package | ELF32 big-endian SuperH sh4al-dsp,entry0x300000; all12 package checks pass for each add-in |
| Shared host/SuperH | **15,247 timing +381 product preset +128,986 connection +2,216 sequential +3,849 FF +20,779 numeric +6,516 Boolean +218 legacy** assertions; exact matching eight passing report lines |
| Timing production | **93 screenshots**, **19 pixel interaction invariants**, representative full-size visual review |
| Earlier tool pixels |59Boolean/51numeric-script/53M4-script accepted references and99M5-engineering tool images exactly match; home/about intentionally changed |
| Earlier UI invariants |15numeric,22FF,20sequential relationships pass |
| Existing independent host suites |Numeric2,359,296 assertions;sequential916,124 assertions/5,408 models;Boolean153,092 minimizations/2,237,318 cared-row checks |
| Infrastructure | EmulatorCTest **5/5** |
| Maximum-domain representative | **65 columns/12 signals/4 cells**, run plus phase replay/checks/report I/O; **292,929 whole-program emulated instructions** |
| Provenance/preservation |107 build inputs match; old computational/UI modules,upstream/common/exact core,accepted artifacts andM5 engineering snapshot unchanged |

Core verification uses independent literal device/gate truth and event fixtures,
including six cell kinds,observed0/1/X clock transitions,all binary state/input/
edge/reset combinations,three-value data expansion,both latch polarities,
synchronous versus asynchronous priority,unknown reset and no-op stableX clocks.
Concrete FF transitions reuse unchangedM4 logic; expected tests do not derive
answers by rerunning production simulation. Gate forward references/topological
validation,disabled source errors,range/null/capacity guards,simultaneousDswap,
initial edge suppression,phase logs and bounded nonconvergence are covered.
Main independently reviewed scheduling/reset/unknown reasoning and requested a
132-assertion follow-up: a merely possible clock edge or uncertain latch enable
must not claim a definite synchronous reset even when the resulting value is
known; definite asynchronous reset may still dominate. Final15,247 count includes
this fix and target execution, not the earlier15,115 count.

Main's381-check product suite independently verifies actual shipped preset
configs:Review1 master's/slave's entire ordered vector and complement,Review2
latch/rising/falling outputs/complements,Review3 numeric7..0 and internal
000->001->011->111 generations,Review4 numeric7 then8..15,0..7 andT intermediates,
Review5 states1,2,3,4,5,0,1,2 plus pre-edge reset0/post-clear reset1,between-edge
sync/async comparison,SR-X propagation/recovery,gatedSR latch,customJK11,maximum
64events,empty timeline and oscillation/error recovery. Review polarity/bit order
was visually checked against existing rendered PDFp21-27. These are discrete
ordered-event goldens, not matching the printed diagram's exact x-coordinate
spacing or measured physical time.

Native scripts `screenshots/m6-a/b/c.script` test manual atomic event editing,
result invalidation,append/delete/clear,64/65 bound,time/signal/phase pages,SR/X
versus disabled rows,latch polarity,resetmode,initialX,customJK11,gate-cycle and
transparent-feedback errors/recovery,help/cancel and cross-tool retention.
Nineteen pixel relations complement mathematical target checks; no OCR assertion
of every rendered character is claimed. Wave/table share cursor and final-settled
values; selected-event phase pages expose intermediate commits. Main visually
reviewed representative first/last/ripple/reset/unknown/config/table screens,
fixed footer spacing and continuous rails,displayed off rows as grey/dashes rather
than liveX,and made clock/enable source markers/input before-after readable.

Shared target program exits after **209,714,318 emulated instructions**, including
all eight suites/report work. It has a pending bounded script event to avoid the
pinned emulator's500ms script-tail cutoff; verifier rejects tail termination or
missing/stale/mismatching reports. Separate `timing_benchmark_main.c` checks every
max-example state column plus selected-event replay and returns after292,929
instructions. This whole-program representative observation includes startup,
checks and BFile I/O; it is not pure simulation latency,worst-case arbitrary
feedback runtime,real seconds or a physical calculator benchmark. The no-stable-
state bound is32phases per event. No heap/new recursion is used by the engine.

Compiler observations: production ELF text**100,164**,data**464**,BSS**10,992** bytes.
SuperH stack frames:timing UI192,tm_run12,tm_explain8,simulate136,settle164,signals48;
launcher40. Existing parser/minimizer recursion remains unchanged. Frames omit
callee helpers/interrupts and dynamic gint/VRAM/heap; physical peak resources,
long-run stability,USB/keyboard/display/OS return remain unmeasured forM5/M6.
The ideal3-value model explicitly fixes atomic-input-first ordering; it does not
predict metastability,setup/hold violations,analog propagation or hidden edges
inside a stable abstractX. Event-slot width is a layout convention, not a timing
measurement. SYNC reset on a latch is transparent-enable-qualified.

Early native UI builds failed GCC misleading-indentation warnings; fixed without
weakening warnings. An ordinary Docker socket attempt needed the established
access escalation; no new dependency/toolchain installation occurred. Initial
iteration evidence is retained in `logs/iteration1-build-failure.log`. Final
fresh build and complete verification pass. Known trailing unused-key diagnostics
remain after actual program return; not every pending/release event is claimed
consumed. Earlier source snapshots/records remain immutable, including their
then-current acceptance-pending state.

Evidence: `.phase1/ecen21-m6/{baseline.json,source-manifest.json,image-inspect.json,
verification.json,final-review.json,delivery-source-manifest.json}`, `artifacts/`,
`host-tests/`, `logs/`, `verification/target/storage/fls0/ECEN21.txt`,
`verification/benchmark/storage/fls0/M6BENCH.txt`, older-tool output directories,
`verification/tim-{a,b,c}/` and contact sheets. Independent engine log:
`.phase1/m6-engine/results.log`; as-built docs:M6-PLAN/M6-MODEL and the Chinese guide.

**M6 engineering passed. M5/M6 hardware acceptances explicitly deferred, not passed.
M7 approval not granted.** [Chinese deferred joint guide](docs/M6-USER-ACCEPTANCE.zh-CN.md)
is ready for later use; its delivery is not physical verification or a demand
to test hardware immediately. Stop before M7 unless separately authorized.

## Milestone 5 engineering verification - passed; hardware subsequently deferred

Explicit authorization: “开始M5吧”. M4 is accepted/closed and its stable checkpoint
remains immutable. Tests below actually ran on November12,2026 using the pinned
ARM64 container and native pinned emulator. No physical M5 interaction occurred.

```sh
bash apps/ecen21/tests/run-host.sh
bash apps/ecen21/tests/run-sequential-exhaustive.sh
bash apps/ecen21/tests/run-numeric-exhaustive.sh
bash experiments/boolean/run-tests.sh
bash scripts/toolchain-build-m5.sh
/opt/homebrew/bin/python3.11 scripts/verify-m5.py
ctest --test-dir .phase1/emulator-build --output-on-failure
```

Use the already cached pinned ARM64 fxSDK image and native emulator. Host screenshot
verification uses Pillow. Current/historical build aliases forward to M5 output;
accepted helpers/artifacts are archived, not overwritten. Production, shared-test
and maximum-domain add-ins are freshly built from the isolated source copy on
Linux tmpfs. Test/benchmark sources are excluded from production. Final93-input
manifest matches workspace inputs. Only ECEN21.g3a is a user program.

| Check | Actual result |
| --- | --- |
| Production artifact | `.phase1/ecen21-m5/artifacts/ECEN21.g3a`, **114,596 bytes** |
| SHA-256 | `3cf6c16f69e8517e09ebaf0949e3fd739bb08f2fc31cbe6a25e93f6433dac97f` |
| Fresh target builds | Production/test/benchmark compile/link/package without final compiler/Make warnings/errors |
| Target/package | ELF32 big-endian SuperH sh4al-dsp,entry0x300000; all12 header checks OK for each add-in |
| Shared host and SuperH report | **2,216 sequential +128,986 connections +3,849 FF +20,779 numeric +6,516 Boolean +218 legacy** assertions; exact matching six passing lines |
| Sequential host sweep | **916,124** assertions across **5,408** model cases; strict C11 warnings/ASan/UBSan clean |
| Numeric host oracle | **2,359,296** assertions, exhaustive8bit/sampled16/32 |
| Exact Boolean regression | **153,092** minimizations, **2,237,318** cared-row checks; sanitized |
| M5 production UI | **100 screenshots**, **20 pixel interaction invariants**, representative visual inspection |
| Accepted-tool regressions |59 Boolean,51 numeric-script tool and53 M4-script tool images exactly match archived RGB pixels;15 numeric and22 FF interaction invariants pass |
| Infrastructure | Emulator CTest **5/5** |
| Maximum-domain representative | Separate test program:16rows/32form-row checks/28nodes, **534,076 whole-program emulated instructions** including init,solve,connections,reference checks and report I/O |
| Preservation | Accepted Boolean/numeric/FF models/views, exact core,upstream Logic/Base-N/common/dist and all earlier accepted artifacts unchanged; M4 bundle unchanged |

The shared model oracle uses literal device truth tables plus independent cube
membership evaluation. All256 two-bit deterministic maps with all16 per-bit type
assignments are tested (4,096 models). An additional576 constant-target designs
cover all2/3bit present/desired transitions across every type assignment;480
partial-cycle designs cover types/policies/control combinations;256 deterministic
three-bit maps (half controlled) sample the larger domain. An independent
all-implicant union-DP checks per-pin minimum group/literal costs for every two-bit
map and representative3-variable cases; four-variable integration is sampled,
with existing exact-core regression supplying its separate oracle coverage.
No exhaustive eight-state/control FSM enumeration or all4-variable ternary
specifications is claimed. Shared target checks deliberately omit the large
host-only sweep, and include all required review fixtures,policy/recovery/SR,
no/wrong-edge holds and maximum79-character malformed/valid expression paths.

Connections are a declarative acyclic224-node/fanin4 bound with explicit actual
Q/E sources,NOT/AND/OR/XOR gates and FF/pin/output drives. Tests independently
parse and execute the emitted gate-language text for every case/row, not just
compare formatter strings or call the builder's evaluator. Counter2 requires
exactly one3-input XOR and one NOT, shared between T0/T1, in both forms. Constants,
all16 two-variable functions, mixed4variable maps/outputs and DC completions are
covered. SOP/POS next-state tables/parity/wiring use their own completed truth
functions; pin equivalence on cared rows is not assumed to constrain unused rows.

Production scripts `apps/ecen21/screenshots/m5-a/b/c.script` cover the T/JK/mixed/
controlled/reverse examples, manual comma-separated cycle entry,duplicate syntax
error and repair,no stale output,cancelled over-capacity edit,HOLD/DC/RETURN traps,
all result views and K-map group details,Counter2 parity wiring,both fixed E
branches,32-entry timeline,16-row/4variable boundary,reverse output equations and
79-character input,D-swap simultaneous update,falling edge,SR invalid stop,
help/sample/menu cancel and state/page preservation across tools. Twenty pixel
relationships complement mathematical target checks; they do not OCR every
rendered value. Representative full-size pages/contact sheets were visually
reviewed, including Gray bit axes,undefined excitations,output page counts and
first/last connection pages. Home/about intentionally change to add tool4; old
module screens remain exact archived matches. Three independent M5 runs keep
within the pinned emulator's512script-step/8192screenshot-path-byte capacities.

The shared target suite returns after **202,502,554 emulated instructions** including
all six suites/report work. A separate `ECEN21_BENCH_BUILD` test-only program
uses the exact maximum-domain UI sample (3Q+E,mixed D/T/JK), independently checks
all16 expected next vectors in both forms and builds its28-node circuit. Its
534,076-instruction whole-program total is a reproducible bound for that one
representative solve path, not isolated solver timing, worst-case arbitrary-input
latency,wall-clock milliseconds or physical timing. It writes M5BENCH.txt; missing
or failing benchmark/target reports fail verification. The benchmark source is
not linked into the user program.

Resource observations: production ELF text**85,456**,data**464**,BSS**8,720** bytes.
Target compiler frames: sequential UI728,expression parser wrapper788,sq_solve200,
synthesis64,minimize/replay128,cn_build92,cn_line92,launcher40. Existing parser/
minimizer recursion remains bounded by accepted limits; no new heap/recursive
framework is added. Compiler sums exclude helper calls,interrupts and dynamic
gint/VRAM/heap allocations; they do not measure physical peak stack/memory.
Deep malformed/valid79-character paths actually ran in the target fixtures.

Resolved iteration issues: GCC misleading-indentation warnings in early guards;
new shared suite exceeding the emulator's500ms headless-script tail (a pending
bounded event keeps it running until actual program return; verifier rejects tail
termination); a single UI script exceeding512steps (split into three runs);
manifest mismatch after script revisions (clean final rebuild); missing Gray
axis labels/undefined excitation distinction/output page counts; help/cancel
incorrectly resetting nonzero pages (fixed and explicitly tested). Initial
failure logs are retained under `logs/iteration1-build-failure.log` and the
original oversized script under `initial-full-ui.script`. The known unused-key
diagnostic remains after completed actual program return; no claim that all
trailing release/pending events were consumed. No weakening of pinned emulator
or warning settings was used to obtain a pass.

Evidence: `.phase1/ecen21-m5/{baseline.json,source-manifest.json,image-inspect.json,
verification.json,final-review.json}`, `artifacts/`, `host-tests/`, `logs/`,
`verification/target/storage/fls0/ECEN21.txt`, `verification/benchmark/storage/fls0/
M5BENCH.txt`, `verification/{boolean,numeric,flipflop,seq-a,seq-b,seq-c}/` and
contact sheets. Independent subtask logs are `.phase1/m5-core/` and
`.phase1/m5-connections/`; design/API notes are `docs/M5-MODEL.md` and
`docs/M5-CONNECTIONS.md`. Main independently reran suites and reviewed critical
oracle/graph code rather than relying solely on subagent reports.

**M5 engineering passed; user acceptance pending; M6 approval not granted.**
Deliver [Chinese M5 user guide](docs/M5-USER-ACCEPTANCE.zh-CN.md) and stop for
physical-user acceptance. Hardware keyboard/display/USB/OS-return,physical
latency/peak memory and long-run stability remain unmeasured for M5.

## M4 hardware acceptance / stable checkpoint / roadmap-only update

Recorded November12,2026. The user explicitly reported **M4 passed real Casio
fx-CG50 hardware acceptance** and formally approved closure. This is accepted
user-reported hardware verification, separate from earlier agent-run host/emulator
checks. No per-step physical log/photos,OS version,latency/peak-memory orlong-run
benchmark was provided; do not invent those values. The agent did not operate
the physical calculator. Phase1/M2A/M2B/M3 are also accepted.

Checkpoint: [M4-accepted](docs/checkpoints/M4-ACCEPTED.md). Acceptedbinary SHA-256
`165a5a2230b075352046e0f666b9b9d981d47b99c69af06443373fcadefcff6f`,93,804bytes.
Archive includes all77 original build inputs,source manifest,ELF,test report,
package dumps,logs,screenshots and original helpers. Original verification.json
and final-review.json still describe the engineering-time acceptance-pending
state; a separateacceptance.json records subsequent closure. Local bundle and
member hashes are separately recorded; no Git tag/remote backup is implied.

This task changes documents/checkpoints only. **No fresh app build, application
regression suite or M5 runtime tests were run**; the previously passed results
below are historical evidence, not newly executed checks. Current checks rehash
the accepted binary and source inputs,compare source/test/helper files before/
after planning,verify archive members and local document links, and check that
M3/upstream files remain unchanged. Final details are logged under
`.phase1/roadmap-review/planning-verification.json`.

Closure integrity checks **passed**:77 accepted build inputs still match;438
application/core/test/helper files match the pre-planning baseline;299 archived
member hashes and the bundle hash verify;45 local document links resolve.
EarlierM2A/M2B/M3 artifact hashes and originalM4 engineering reports match.
No solver implementation/build/runtime test is implied by these integrity checks.

Review provenance:41-page user PDF SHA-256
`ce9aab1958feb3b7736ce5eabbd754efb93ce4929b031d4f2672689cd058a8a3`;
all pages text-extracted and circuit/waveform/code pages rendered for visual
review. Outputs remain in `.phase1/roadmap-review/`. Complement bars,derived
clocks,softreset and Q0-first state labels were checked visually. This inspection
supports requirements/fixture transcription, not a claim of solver test success.
[Coverage matrix](docs/PROBLEM-COVERAGE.md) and [M5 plan](docs/M5-PLAN.md) clearly
label existing evidence versus planned acceptance cases. M3 is frozen; its
unhandled custom-width topics remain FutureBacklog only.

**M4 closed. M5 requirements proposed; implementation unapproved.** Future
milestones require important-module tests,freshECEN21.g3a,actualemulator tests,
accepted-feature regressions,record updates and Chinese physical-user handoff,
then explicit user acceptance and separate next-stage approval.

## Milestone 4 engineering verification — passed; user subsequently hardware accepted

The explicit instruction “M3验收完成 开始M4” accepted M3 and authorized M4. Results below actually ran on November12,2026 using the pinned ARM64 Docker toolchain and native emulator. M3 source/artifact/evidence/original helpers were archived under `.phase1/milestones/M3/` before integration. No physical calculator interaction was performed.

```sh
bash apps/ecen21/tests/run-host.sh
bash apps/ecen21/tests/run-numeric-exhaustive.sh
bash experiments/boolean/run-tests.sh
bash scripts/toolchain-build-m4.sh
/opt/homebrew/bin/python3.11 scripts/verify-m4.py
ctest --test-dir .phase1/emulator-build --output-on-failure
```

Use the existing cached image/native emulator; host screenshot comparisons require Pillow. Current/generic/historical build aliases forward to M4 output, avoiding accepted-output overwrite. Original helpers remain in each milestone's archive. Host fixtures use strict C11 warnings-as-errors, ASan and UBSan. Production excludes fixture/report code. Final source manifest matches workspace inputs; all generated output stays ignored under `.phase1/`.

| Check | Actual result |
| --- | --- |
| User artifact | `.phase1/ecen21-m4/artifacts/ECEN21.g3a`, **93,804 bytes** |
| SHA-256 | `165a5a2230b075352046e0f666b9b9d981d47b99c69af06443373fcadefcff6f` |
| Fresh compilation | Production and test-only add-ins compiled/linked/packaged, no compiler/Make warnings/errors |
| Target/package | ELF32 big-endian SuperH sh4al-dsp, entry0x300000; both add-ins have all12 header checks OK |
| Shared host and SuperH fixtures | **3,849 flip-flop +20,779 numeric +6,516 Boolean +218 legacy** assertions; target storage report exactly matches host |
| Numeric host oracle | **2,359,296** assertions: exhaustive8bit and sampled16/32 arithmetic |
| Exact Boolean regression | **153,092** minimizations, **2,237,318** cared-row checks |
| Flip-flop production UI | **54 screenshots**, **22 pixel interaction invariants**, representative visual review |
| Existing tools | Boolean59 archived pixel matches; numeric script54 screenshots/15 invariants with51 unchanged tool-page matches |
| Emulator infrastructure | CTest **5/5** |
| Preservation | Original Logic/Base-N/common/dist, accepted Boolean/numeric model/UI and exact-engine files unchanged; M2A/M2B/M3 artifacts retained |

The flip-flop oracle uses independent literal truth tables rather than the production branch logic. All192 type/Q/a/b/active-edge/event combinations are checked as pure queries and stateful steps, including ignored D/T b values. All24 characteristic rows and16 excitation queries are checked; expanding every X cube proves both soundness and completeness over binary assignments. Other fixtures cover null/enum/bit/bounds errors, lock persistence under settings/no-op type changes, explicit reset/type-change recovery, exact16-entry capacity and rejected step17, immutable edge/event snapshots, stale-error clearing and labels. Main independently reviewed and reran the agent-owned model suite. A counter-reset omission discovered during host integration was fixed before final target execution; the actual flip-flop count is3,849, not an accumulated earlier-suite count.

Production scripts cover D capture versus no/wrong edge (both edge choices), JK double toggle, T1/0/1, every type's tables, excitation X, SR set/reset/invalid preview/ignored event/lock/recovery, reset1/cancel, three history pages and full feedback, help, tool-switch retention and arrow/EXE navigation/OS exit. Twenty-two automated pixel relationships include current-Q traces, invalid-state indication, preserved simulation, no overwritten full history and old-tool availability. Existing numeric script additionally exercises errors/conversions/flags/width changes and Boolean state retention. All59 M2B tool images and51 M3 numeric-script tool images match archived RGB pixels; selector/about pages intentionally change to include the third tool. Representative full-size screenshots and a contact sheet were visually inspected, including all characteristic/excitation types, invalid history and all three pages. Pixel relationships are not OCR of every character; the independent shared target fixtures supply mathematical checks.

Final target harness exits after **39,563,819 emulated instructions**, including all four suites and BFile report writing; this is not a pure flip-flop or physical performance benchmark. ELF text=64,664/data=464/BSS=2,672 bytes. GCC `-fstack-usage` reports ff_step68, ff_evaluate8, ff_characteristic28, ff_reinitialize12, flipflop_run120 and launcher44 bytes/frame. New model/UI uses fixed bounded state with no recursion/heap; existing Boolean recursive paths remain. Compiler frames exclude library helpers/interrupts and dynamic gint/VRAM/heap; physical peak stack, latency and long-run stability are unmeasured.

The first ordinary Docker invocation lacked socket access; the authorized isolated build completed after tool access escalation. No new dependency installation/native SDK research was needed. Verification intentionally rejected mismatched provenance after an output-path helper changed; the final fresh build and full verifier pass. Python syntax checking initially could not write the system cache; an explicit milestone-local cache succeeded. The known trailing unused-key diagnostic persists after completed screenshots/normal exit, without a claim that every trailing event was consumed.

Evidence: `.phase1/ecen21-m4/{source-manifest.json,image-inspect.json,verification.json,final-review.json}`, `artifacts/`, `host-tests/`, `logs/{build.log,production-*.log,target-test-header.log,exact-core-host.log,emulator-ctest.log}`, `verification/target/storage/fls0/ECEN21.txt`, `verification/{boolean,numeric,flipflop}/` and `verification/contact-sheet.png`. Scripts: `apps/ecen21/screenshots/m4.script`, `scripts/verify-m4.py`. Model handoff/semantics: `docs/M4-FLIPFLOPS.md`, agent-local `.phase1/m4-model/`.

**M4 engineering verification passed; user subsequently reported real fx-CG50 acceptance passed and approved closure. M5 implementation remains unapproved.** [The Chinese M4 guide](docs/M4-USER-ACCEPTANCE.zh-CN.md) and original evidence remain preserved. Physical acceptance is user-reported; numerical speed/peak-resource measurements remain unavailable.

## Milestone 3 engineering verification — passed; user subsequently accepted

M2B is user accepted; the subsequent explicit instruction “允许进入M3” approved this scope after the earlier plan-only choice. M3 results below were actually run on November12,2026. They do not represent physical calculator measurements.

```sh
bash apps/ecen21/tests/run-host.sh
bash apps/ecen21/tests/run-numeric-exhaustive.sh
bash experiments/boolean/run-tests.sh
bash scripts/toolchain-build-m3.sh
/opt/homebrew/bin/python3.11 scripts/verify-m3.py
ctest --test-dir .phase1/emulator-build --output-on-failure
```

Reuse the cached pinned ARM64 Docker image and native emulator; Pillow is required for host screenshot comparisons. Production/test builds use Linux tmpfs and isolated `.phase1/ecen21-m3/`. Generic and historical build aliases forward to current M3 so accepted M2A/M2B delivery directories remain intact. Their original sources/helpers/artifacts are archived under `.phase1/milestones/`; old-stage reproduction instructions below describe that stage's snapshot, not current source. M3 input buffers and arithmetic/conversion core are independent of gint. Only the test add-in contains fixture/report-writing code.

| Check | Actual result |
|---|---|
| Production file | `.phase1/ecen21-m3/artifacts/ECEN21.g3a`,87,652 bytes |
| SHA-256 | `f309877c3dcd030bd452f11a97d53ab7ccf651d0fb0dcb35706e7f9406b39aed` |
| Fresh build | Production and test-only add-in compiled/linked/packaged without compiler/Make warnings/errors |
| Target/header | ELF32 big-endian SuperH sh4al-dsp; both add-ins' signature/control/size/checksum/footer checks passed |
| Host shared fixtures |20,779 numeric +6,516 Boolean +218 legacy checks; strict warnings, ASan/UBSan clean |
| SuperH shared fixtures | Identical three passing reports to host, checked via emulator storage file |
| Numeric exhaustive/oracle |2,359,296 host assertions:262,144 exhaustive8bit cases +32,768 sampled16/32 cases,8 assertions each |
| Boolean exact regression |153,092 minimizations,2,237,318 cared-row checks; ASan/UBSan clean |
| Accepted Boolean rendering | All59 M2B screenshots exactly match archived reference pixels |
| Numeric/navigation rendering |54 screenshots,15 automated pixel interaction invariants; representative layout review |
| Emulator infrastructure | CTest5/5 passed |

The arithmetic oracle uses wider signed mathematics/modulo and range comparisons rather than the production sign-bit overflow formula. Every8bit A/B pair is tested for ADD/SUB and both display modes;16/32 use boundary matrices and deterministic samples, not exhaustive pairs. Shared conversions cover every8bit raw value in all bases/modes with independent bit/nibble/decimal references, parse roundtrips, tiny buffers and formatting lengths. Additional cases include signed32 minimum, all milestone examples, malformed/empty/excess-width/too-long input, stale-result clearing, invalid-setting rejection, raw reinterpretation, sign/zero extension and meaningful truncation persistence. This is not exhaustive input-string fuzzing or full16/32 arithmetic enumeration.

The target harness runs representative/shared checks only, not the large host-only exhaustive loop; it exits after39,252,272 emulated instructions including Boolean/legacy work and BFile report writing. Production excludes test sources and oracle loops. Target fixtures validate the actual numeric core and conversions mathematically; screenshot relationships complement them without claiming OCR of every displayed number.

Boolean production regression uses the unchanged M2B key script with explicit selector entry and a second EXIT for the new navigation level; all59 screenshots match the accepted M2B archive. Numeric production flows exercise manual A/B entry, ADD/SUB, all eight examples, base/sign reinterpretation, signed width roundtrip, unsigned extension, meaningful narrowing and warning persistence, empty/out-of-range/binary-digit errors, blocked settings during invalid input, repair/recompute, manual signed32 minimum, sample return, and data preservation across tools. Fifteen pixel invariants check corresponding state/bit/result preservation and error/no-old-result relationships. Representative selector, conversion, flags,32bit results, invalid input and truncation screens were visually reviewed. M3 intentionally changes module EXIT to return home; home EXIT ends the app.

Resource observations: full ELF text58,512 bytes, data464, BSS2,048. Target frames: numeric parse64, format84, solve40; numeric view128, conversion drawing116, launcher44. There is no numeric heap/recursion; these figures exclude library helpers, interrupts and dynamic gint/VRAM/heap allocations. Existing Boolean recursive paths remain and gain the shallow launcher frame. Physical peak stack, latency/stability, keyboard/OS-return and USB remain unmeasured by the agent. Earlier user acceptance does not certify this new version.

Initial integration smoke correctly refused to publish a final report when the newly added UI script differed from the earlier build-input manifest. After a clean rebuild of the full input set, final provenance and UI verification passed. The known trailing unused-key diagnostic may still occur after normal exit and screenshot completion; verification does not claim consumption of every trailing release/event.

Evidence: `.phase1/ecen21-m3/{source-manifest.json,image-inspect.json,verification.json}`, `artifacts/`, `host-tests/{results.log,numeric-exhaustive.log}`, `logs/{build.log,production-*.log,target-test-header.log,exact-core-host.log,emulator-ctest.log}`, `verification/target/storage/fls0/ECEN21.txt`, `verification/{boolean,numeric}/*.png` and emulator logs. Script source is `apps/ecen21/screenshots/m3.script`; model semantics are in `docs/M3-NUMERIC.md`. The original numeric subagent evidence remains under `.phase1/m3-numeric/`.

**M3 engineering verification passed. The user subsequently accepted M3 and approved M4 with “M3验收完成 开始M4”.** [The Chinese M3 acceptance guide](docs/M3-USER-ACCEPTANCE.zh-CN.md) and this evidence are retained. No individual hardware measurements are inferred from acceptance. Earlier evidence below remains historical and preserved.

## Milestone 2B engineering verification — passed; user accepted

The user explicitly accepted M2A and approved M2B. Verification below ran on November 12, 2026; it does not imply agent-observed physical hardware testing.

Subsequent acceptance update: the user reported “M2B检验成功”, so M2B user acceptance is recorded as passed. No individual unreported hardware measurements are inferred. At the planning-only checkpoint, the user explicitly deferred M3 implementation. That checkpoint did not rerun these suites; later M3 approval and actual execution evidence are recorded above.

```sh
bash apps/ecen21/tests/run-host.sh
bash experiments/boolean/run-tests.sh
bash scripts/toolchain-build-m2b.sh
/opt/homebrew/bin/python3.11 scripts/verify-m2b.py
ctest --test-dir .phase1/emulator-build --output-on-failure
```

Use the existing pinned ARM64 Docker image and native emulator from Phase 1; host verification requires Pillow. `toolchain-build-ecen21.sh` is now an alias for the current M2B build, preserving the accepted M2A output directory. M2A source snapshot, original helpers, binary and evidence are archived under `.phase1/milestones/M2A/`. Historical M2A commands below describe that stage's source, not a way to regenerate the old app from current M2B source.

| Final check | Actual result |
|---|---|
| Production file | `.phase1/ecen21-m2b/artifacts/ECEN21.g3a`, 80,124 bytes |
| SHA-256 | `05cca11650ee7ebe21debf62f2a471bc6a5df2f1a2ce03bfbf19d233e5de15c8` |
| Fresh build | Production and test-only add-in compiled/linked/packaged; no compiler or Make warnings/errors |
| Target | ELF32 big-endian SuperH, sh4al-dsp; packaging signature/control/size/checksum/footer checks passed |
| Host model/controller | 6,516 new model checks + 218 unchanged legacy checks, strict warnings/ASan/UBSan clean |
| SuperH integration fixtures | Same 6,516 + 218 checks passed; target-written report matched host report exactly |
| Exact-engine regression | 153,092 minimizations, 2,237,318 cared-row checks; ASan/UBSan clean |
| Production smoke | 59 screenshots, 16 pixel interaction invariants, normal exit |
| Emulator CTest | 5/5 passed |
| Preservation | Original Logic/common, exact-engine source and existing distribution binaries unchanged; M2A binary/evidence retained |

The model suite checks parser aliases/precedence, literals/constants, malformed/empty input and first-error position, all 256 choices of four variable references in `!(a+b)c^d` against direct Boolean semantics, cursor/79-character boundaries, and both exact forms against cared rows. It exercises manual 0/1/X cycles, source restoration and no accidental restoration, low-row variable truncation, all-DC conventions, exact counterexample/parity/DC fixtures, every 2/3/4-variable Gray cell and wrap adjacency, and cell cardinalities for all possible cubes. Lead-added malformed 79-parenthesis/79-prefix cases check the deeper partial-input recursion reachable during editing. This is bounded coverage, not exhaustive string fuzzing or exhaustive four-variable ternary specifications; the engine's independent oracle scope remains as documented below.

The target harness runs the same source fixtures and emits a checked file through emulated OS BFile calls. It exits after 12,290,670 emulated instructions including report work. The production build excludes the fixture/marker code. Production scenarios cover typed operators/parentheses and cursor edits, syntax/range errors and old-result clearing, table/K-map edits and synchronization, 0/1/X, both forms, source preservation and explicit saved-text restoration, 2/3/4-variable transitions, all-DC, constants, eight-group navigation, long POS, samples/help and exit.

Sixteen automated pixel properties include equivalent-expression result equality, manual table preservation across views, explicit restoration back to the original result, DC removal/restoration, variable roundtrip preservation of low rows, distinct all-DC completions, form differences, group wrap/access beyond six groups, and help return without data loss. These relationships complement the independent mathematical fixtures; they are not an OCR assertion of every rendered character. All 59 images were generated and dimension/nonblank checked; representative editor/table/map/result/group/error/help and long-result layouts were visually reviewed. K-map variable-label spacing and term/clause-aware wrapping were improved during that review. The tested long four-variable POS result fits one page; a multi-page formula fixture was not exercised.

Resource observations from `sh-elf-size`: text=50,984 bytes, data=464, BSS=1,856. Stack estimates: main=224, bs_solve=48, eb_minimize=892, visit=112 bytes/frame. A conservative 17-visit solve-chain illustration is 3,068 bytes before helpers/interrupts. Parser frames: parse_or20, parse_xor20, parse_and24, parse_not16 (atom inlined), recompute64, bs_insert16. An 80-level malformed-parenthesis illustration totals about 6,704 bytes including those main/edit/recompute frames, excluding helpers/interrupts. Deep malformed cases actually ran in the target harness; these compiler sums are not a physical peak-stack measurement. ELF data/BSS exclude dynamic gint/VRAM/heap allocations. No physical latency benchmark was run.

The known production-script trailing diagnostic `script: 1 key left unused (the add-in ended first)` remains after all screenshots and normal exit. No assertion is made that every trailing event was consumed. New M2B physical keyboard/OS-return behaviour, stability, peak stack, timing and USB remain unverified by the agent. User acceptance of M2A is recorded without inventing M2B hardware results.

Evidence under `.phase1/ecen21-m2b/`: `artifacts/`, `source-manifest.json`, `image-inspect.json`, `verification.json`, `host-tests/results.log`, `logs/build.log`, `logs/production-*.log`, `logs/target-test-header.log`, `logs/exact-core-host.log`, `logs/emulator-ctest.log`, and `verification/{target,ui}/`. The target report is `verification/target/storage/fls0/ECEN21.txt`; UI script source is `apps/ecen21/screenshots/m2b.script`. `docs/M2B-MODEL.md` records design/invariants and the original subagent test result.

**M2B engineering verification and user acceptance are complete.** The [Chinese M2B user guide](docs/M2B-USER-ACCEPTANCE.zh-CN.md) and evidence remain preserved. The historical M3 planning-only state was subsequently superseded by explicit approval and the verification section above. M4 and later milestones remain unapproved. All earlier evidence below is preserved.

## Milestone 2A engineering verification — passed; user accepted

Verified November 12, 2026 using the existing pinned ARM64 Docker image and native emulator. Reproduction from the repository root:

```sh
bash apps/ecen21/tests/run-host.sh
bash experiments/boolean/run-tests.sh
bash scripts/toolchain-build-ecen21.sh
/opt/homebrew/bin/python3.11 scripts/toolchain-verify-ecen21.py
ctest --test-dir .phase1/emulator-build --output-on-failure
```

Docker Desktop must be running and the pinned Phase 1 image cached. Verification requires the existing emulator and a Pillow-enabled host Python. CMake directly links the tested `experiments/boolean/exact_boolean.c`; the app's portable controller and production UI are separate translation units. `ECEN21_TEST_BUILD=ON` selects an alternate test main/fixtures; the production binary contains no test BFile/report-writing code.

| Check | Actual result |
|---|---|
| Production add-in | `.phase1/ecen21/artifacts/ECEN21.g3a`, 72,532 bytes |
| SHA-256 | `a1a9705e202ca0e33329b1e0c899e13084c5899e9583b29b164a8bc4b0a6fbc5` |
| Target | ELF32 big-endian SuperH, `sh4al-dsp`, entry `0x300000` |
| Fresh build | Production + `EC21Test.g3a` compile/link/package succeeded; final build has no warnings/errors |
| Packaging | fxgxa signature/control/size/checksum/footer checks passed for both add-ins |
| Exact-core host regression | 153,092 minimizations, 2,237,318 cared-row checks; ASan/UBSan clean |
| New controller host tests | 218 checks; strict warnings + ASan/UBSan clean |
| Same controller fixtures on SuperH | 218 checks passed in emulator; generated report matched passing host report exactly |
| Production UI smoke | Normal exit; 12 screenshots; six interaction invariants passed |
| Emulator infrastructure | CTest 5/5 passed |
| Upstream preservation | Tracked Logic/common files and all existing distribution binaries matched entry hashes |

The shared controller fixture checks known independent results (`007E` = three terms/nine literals; constants; each single variable; `FCCC` = `C+AB`; `6996` = eight terms/32 literals), every row for each case, input replacement/append/delete/clear, four-digit limit, invalid digit rejection, empty input, invalid text and stale-result invalidation. The test-only SuperH add-in runs these same checks, writes `storage/fls0/ECEN21.txt` through the emulated OS, and exits. Verification rejects an absent, stale, failing or mismatched report. This checks target compilation/execution of the actual controller/core, not merely screenshot presence.

The production key script exercises demo calculation, ZERO/ALL presets, EXE/F6 solve, empty input, typed `7E`, deletion/recalculation, rejected fifth digit, long parity output, About/return and EXIT. Automated pixel invariants check: typed/demo result equality; editing clears the result; empty input has distinct feedback; zero/one differ; the fifth digit leaves `FFFF` unchanged; About preserves the full solved screen. All 12 screens were also visually reviewed, including long-result wrapping. Target fixtures supply mathematical output checks independently of these pixel relationships; production output for the known examples was visually confirmed.

The first build on the macOS bind mount emitted sub-millisecond Make clock-skew warnings. The helper now copies source into the container's Linux tmpfs, producing a clean final build. Source manifests record the original inputs; only generated files are mounted writable. The final source manifest was checked against workspace inputs. Build-time date/time metadata means a subsequent rebuild may change artifact hashes.

Static `sh-elf-size` reports text=43,392 bytes, data=464, BSS=1,280 for the production ELF. Compiler stack estimates: `main=104`, `ec_solve=36`, `eb_minimize=892`, `visit=112` bytes/frame. A conservative 17-frame visit chain plus those caller frames sums to 2,936 bytes before library helpers, interrupts and other runtime overhead. ELF BSS/data do not include dynamic gint/VRAM/heap allocations. The target fixture run completed after 2,085,200 emulated instructions, including its file/report work; this is representative execution evidence, not a worst-case solve benchmark or physical timing measurement.

The production smoke retains `script: 1 key left unused (the add-in ended first)` after all screenshots and normal exit. Verification does not claim every trailing event was consumed. Physical fx-CG50 launch/input/OS-return/USB behaviour, peak stack and worst-case latency remain unverified. No physical interaction was attempted. These limits do not prevent the explicitly approved software-only acceptance of 2A.

Evidence under ignored `.phase1/ecen21/`:

- `artifacts/ECEN21.g3a`, `ECEN21.elf`, `EC21Test.g3a`, `SHA256SUMS`
- `source-manifest.json`, `image-inspect.json`, `verification.json`
- `host-tests/results.log`, `logs/exact-core-host.log`, `logs/emulator-ctest.log`
- `logs/build.log`, `production-header.log`, `target-test-header.log`, `production-elf.log`, `production-size.log`, `production-stack.log`
- `verification/target/storage/fls0/ECEN21.txt`, target/UI `emulator.log`, `verification/ui/*.png`, `verification/contact-sheet.png`
- `logs/upstream-preservation.log` and `upstream-baseline.sha256`

**Milestone 2A engineering verification is complete; the user explicitly accepted M2A and approved M2B.** Individual unreported physical measurements remain unverified. The [Chinese user acceptance guide](docs/M2A-USER-ACCEPTANCE.zh-CN.md) provides installation, exact key sequences/expected results, a pass checklist and a failure-report template. No physical result is inferred from its publication. Advancement requires the user's explicit acceptance and explicit next-milestone approval. The Phase 1 evidence below is retained unchanged as historical verification.

## Existing Logic baseline

```sh
python3 scripts/verify-logic-core.py
bash scripts/verify-logic-emulator.sh
```

The first command compiles extracted, unchanged Logic core functions with AddressSanitizer and UBSan. It checks expression examples, all 65,808 truth tables for 2–4 variables, SOP round trips, and an independent minimum-term oracle. It reports known nonminimal covers without changing Logic. Exit 0 means the audit completed, not that all covers are minimal.

The emulator helper uses the checked-in prebuilt by default. To validate a fresh artifact, pass its absolute path explicitly:

```sh
bash scripts/verify-logic-emulator.sh /absolute/path/to/fresh/Logic.g3a
```

It checks emulator revision, builds the emulator, runs available CTest tests, redirects Logic demo screenshots into ignored `.phase1/logic-demo/`, and runs the chosen binary. The script only checks screenshot existence; pixel comparison and visual inspection must be reported separately. fxSDK discovery may enable additional emulator integration builds, which need a complete toolchain.

## Verified fresh source build

```sh
bash scripts/toolchain-build-logic.sh
/opt/homebrew/bin/python3.11 scripts/toolchain-verify-logic.py
```

Prerequisites are running Docker Desktop, the Phase 1 native emulator and a host Python with Pillow. The explicit interpreter is the verified Pillow-enabled Python on this machine. The clean-build helper uses the pinned ARM64 image documented in [TOOLCHAIN.md](docs/TOOLCHAIN.md), deletes stale output, recreates a source copy excluding generated add-ins/build directories, records input hashes, and invokes `fxsdk build-cg`. Original Logic/common files and the checked-in prebuilt are preserved. A container rebuild can change date/time metadata and hence the hash; use the new run's recorded checksum.

| Final artifact check | Observed result |
|---|---|
| Path | `.phase1/toolchain/artifacts/Logic.g3a` |
| Size | 85,716 bytes |
| SHA-256 | `100b83beaa5dd6051cebdecfb816df8946d60bd6afa66ea884691ca56fd98b25` |
| Compile/link/package | Successful; no compiler warnings/errors |
| Toolchain | fxSDK 2.11.0, GCC 14.1.0, binutils 2.42, gint 2.11.0 |
| ELF | ELF32, big-endian, SuperH, `sh4al-dsp`, entry `0x300000` |
| fxgxa validation | Signature/control/size/checksum/footer checks passed |
| Agent fresh-binary emulator run | Exit 0; five exact reference pixel matches |
| Independent lead fresh-binary emulator run | Exit 0; five exact reference pixel matches |
| Build-input preservation | All 49 manifest inputs match workspace originals |

Lead evidence: `.phase1/lead/source-manifest-review.log`, `fresh-arm64-emulator.log`, `fresh-arm64-pixels.log`; agent evidence: `.phase1/toolchain/logs/logic-container-build.log`, `logic-g3a-header.log`, `logic-elf-header.log`, `logic-fresh-emulator.log`, and `.phase1/toolchain/verification.json`. All five images (`main`, `expression`, `truth-table`, `kmap`, `kmap-edited`) were compared after decoding to RGB pixels. The lead also visually inspected fresh output. The initial fresh amd64-container artifact passed a separate lead demo/pixel comparison under `.phase1/lead/fresh-amd64*`.

Both fresh emulator runs retain the diagnostic `script: 1 key left unused (the add-in ended first)` after the five screenshots and normal exit. No claim is made that every trailing script event was consumed. Docker has supplied the build tools; host `make app APP=logic` still lacks native fxSDK on PATH.

## Independent algorithm and baseline verification

The lead independently reran the baseline Logic audit and CTest: 65,808 covers preserved truth-table semantics, 2,288 four-variable covers were nonminimal, and 5/5 emulator tests passed. Logs are under `.phase1/lead/baseline-core-audit.log` and `baseline-emulator-ctest.log`. These checks still use the earlier host/emulator setup and do not establish a fresh source build.

Agent B's exact minimizer passed a separate lead rerun:

```sh
bash experiments/boolean/run-tests.sh
```

Result: **153,092 minimizations; 2,237,318 cared-row checks; ASan/UBSan clean.** The production prime-cover search is compared against an independent all-cube union dynamic program for the lexicographic `(terms,literals)` objective. Coverage includes every fully specified 2–4-variable function in both SOP and POS, every ternary ON/OFF/DC specification for 2–3 variables, and 4,096 deterministic four-variable ternary samples. The complete `3^16` four-variable ternary domain was not tested. Canonical ordering is checked by sorted outputs, repeated calls and an independently identified tie fixture, not an exhaustive tertiary-cost oracle. Constants, POS polarity, invalid inputs, output preservation, formatting and buffer termination are covered.

The lead independently enumerated all valid cube subsets for `ON=0x007e, DC=0, n=4`: minimum cost is three terms/nine literals; `A'C'D + A'BD' + A'B'C` is an optimum. This matches the prototype. Logs: `.phase1/lead/exact-minimizer-tests.log`, `.phase1/lead/counterexample-review.log`; design and target-resource limits: [BOOLEAN-MINIMIZATION.md](docs/BOOLEAN-MINIMIZATION.md).

The lead additionally compiled `experiments/boolean/exact_boolean.c` to a SuperH object with GCC 14.1.0, `-Os -m4-nofpu -mb`, strict warnings treated as errors and `-fstack-usage`. It passed without diagnostics: 1,524 text bytes, zero data/BSS; stack estimates `eb_minimize=892` bytes and `visit=112` bytes/frame. This is a compile/resource check, not a linked calculator runtime test. Evidence: `.phase1/lead/target-core-compile.log`.

Physical calculator behaviour, OS return, USB operation and exact-core target runtime are unverified. These remain distinct from the successful fresh source build and native-emulator checks. The exact core is compiled as an isolated SuperH object only; its host oracle tests do not execute on the calculator.

## Preservation check

The lead captured `.phase1/lead/unchanged-source.sha256` and `.phase1/lead/baseline.sha256` before work. Final hash checks and `git diff -- apps/logic common dist/g3a/Logic.g3a` confirm no changes to the original tracked sources or prebuilt. Generated outputs remain ignored under `.phase1/` and `experiments/boolean/build/`. `git diff --check`, shell syntax checks and Python compilation checks passed.

## M7 planned checks — not executed

M7 planning is recorded in [M7-PLAN](docs/M7-PLAN.md), tests M7-A1 through M7-A8. These are future acceptance requirements, not passing results. This planning update only checked document formatting/links; no application source,build or prior result changed. M5/M6 physical verification remains deferred.
