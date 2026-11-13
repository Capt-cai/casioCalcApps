# ECEN21 Toolkit — M8 Verilog generation/export

Launcher7 opens parameterized Verilog-2001 generation. Nine families: behavioral/
structural WIDTH1..32 adders,ordinary4/8input encoder,D/T/JK FF,universal WIDTH1..32
shift register,synchronous modulo1..65535 counter,width1..32ring/seed/direction.
Clock edge/reset mode/polarity and FF reset values are explicit. State has no
assumed power-up value. Shift priority reset>load>right>left>hold. Counter WIDTH
is safe derived size plus EXTRA_BITS so selected wider widths remain defaults.
Ring requires reset;WIDTH1 has no invalid slice. No priority encoder/FPGA timing.

F1 settings/arrows/EXE edits;F2 full source pages/arrows;F3 port/behavior contract;
F4 export review;F5 orOPTN family1..9,0help;F6 BUILD or SAVE while export is open.
EXIT cancels export review before leaving the tool. Numeric editors reject range
and preserve original settings on cancel. Changed source parameters invalidate
old code; export slots may change without invalidating identical source. Export
also checks document/config compatibility. Generated source is bounded8191bytes,
NUL-free and even-sized for BFile. Visual wrapping never changes exported bytes.

Each family exports a fixed8.3 name plus slot00..99 (eg ECAB00.v,ECSR01.v). Existing
files are never overwritten. BFile create/open/write/close failures are explicit;
new incomplete files are cleaned where possible. No previous files are removed.
OS calls run in gint_world_switch. Actual production exports are byte-compared
and host-compiled/simulated; physical USB remains unverified. Generator comments
use unchanged M4 characteristic and M7 one-hot encoder references. Verilog runtime
benches are independently written,not copies of generator strings.

Build/test: scripts/toolchain-build-m8.sh,scripts/verify-m8.py,tests/run-host.sh,
tests/run-hdl.sh. Icarus13.0 with-g2001 verifies actual HDL,including native exports.
See docs/M8-PLAN.md and docs/M8-MODEL.md. Outputs .phase1/ecen21-m8;M7 engineering
snapshot and all previous accepted/core/UI modules remain preserved except launcher.

---

# ECEN21 Toolkit — M7 combinational design

M7 adds native retained-state combinational solving: HA/FA/three-input popcount,
1..8bit ripple structure,2..4variable truth/expression -> exact SOP/POS/K-map ->
actual shared gate and2/4/8:1MUX connections,ordinary4/8-input encoders,decimalBCD
seven-segment mapping with both polarities/explicit invalid policies. One output
at a time shares one structured problem; no graphical editor/analog delay/global
minimum-gate-count claim. No Verilog generation in M7.

Launcher6/F6 opens Combinational. OPTN1..7 selects problem families and replaces
current work; OPTN9 opens help; other menu keys cancel. F1 settings (UP/DOWN
field,EXE edit/cycle),F2 truth/stage table,F3 equations,F4 Gray map,F5 wires,F6 solve.
In wires EXE -> MUX data -> verification -> wires. UP/DOWN pages wires/data/groups;
TABLE arrows select rows,EXE cycles0/1/X only in generic manual-table mode. SET
selects output/form/ordered selectors. Expression editor reuses A:x-theta,B:log,
C:ln,D:sin; +OR,multiplyAND,powerXOR,divideNOT,decimal-point complement,brackets.
Editors apply with EXE; EXIT cancels. Modified settings invalidate derived results.

P4.12 maps S1=x0,S0=y0; Sum data c0,!c0,!c0,c0; Carry0,c0,c0,1. Encoder code is OR
of active input indices even on invalid cases; VALID checks exactly one input.
Seven-segment a..g=top,UR,LR,bottom,LL,UL,middle; distinguish active-low blank from
zero physical pins. Gate and MUX paths agree on selected DC completion; tables
show actual values. Existing engines/numeric custom widths remain unchanged.

Build/test: scripts/toolchain-build-m7.sh, scripts/verify-m7.py,
tests/run-host.sh,tests/run-combinational.sh. Source/input limits and acceptance
requirements: docs/M7-PLAN.md. Current M7 outputs .phase1/ecen21-m7; previous
engineering snapshots remain separate and are not newly hardware-accepted.

---

# ECEN21 Toolkit - M6

Offline native fx-CG50 Boolean,numeric,flip-flop,counter/sequential and timing
solvers. M4 is user-accepted/closed. M5 engineering passed; the user explicitly
deferred physical acceptance and approved M6 (“开始M6吧 实机验收后面一起做”).
M5/M6 hardware acceptance is not inferred from engineering checks. M7 unapproved.

## Navigation and preserved tools

Home1/F1 Boolean,2/F2 Binary,3/F3 singleFF,4/F4 Sequential,5/F5 Timing. Arrows+EXE
also select. Tool EXIT returns home retaining in-memory state; home EXIT returns
to OS. Fresh launch resets all tools, without disk persistence. Previous module
controls/behaviour remain unchanged; originalM5 sources/binary/helper/evidence
are preserved under `.phase1/milestones/M5-engineering/` (not marked accepted).
The user-authorized M3 follow-up supports custom1..32bit widths. F4 opens a width editor: digits then EXE/F6 apply, F1/F2/F3 select8/16/32, EXIT cancels. Width reduction retains low bits and warns if values change; signed widening sign-extends. HEX uses ceil(width/4) digits and rejects high bits outside width. See docs/M3-CUSTOM-WIDTH.zh-CN.md for the new acceptance guide. Other tools remain unchanged.

## Timing model

`timing.[ch]` is a portable bounded ideal event simulator using accepted
flip-flop characteristic rules.4external inputsI0-I3,4state cellsQ0-Q3,
4NOT/AND/OR/XOR/NAND gatesG0-G3,64masked input events plus initial column,
32settling phases per event. Cells support D/T/JK/SR edgeFFs,D/SR level latches,
rising/falling edges,transparent-high/low enable,sync/async reset polarity/value
and initial0/1/X. Explicit references include every enabled signal andconstants0/1.
Gate-only dependency cycles and disabled-source references are rejected;
transparent/state feedback has a bounded nonconvergence diagnostic.

Column0 settles initial latches/async reset without an invented FF edge. Each
atomic event first applies external inputs, evaluates gates from oldQ, then
samples/commits simultaneously triggered cells. Data in the same event as clock
is sampled after the external input change; use a preceding event to model prior
setup. A closing latch holds, an open latch follows data; master/slave uses two
actual opposite-enable latches. Derived clocks create further bounded phases at
the same event. SYNC reset samples precommit feedback at an FF edge (for latches,
qualified by transparent enable); ASYNC reset dominates its cell in each phase
and can act between edges. X propagates conservatively; stableX->X isn't an
observed edge. SR-invalid next states becomeX and may later resolve through
definite set/reset/capture, unlike the separate binary-onlyM4 invalid lock.

The time axis is an ordered event grid, not measured real seconds; slot width is
layout, not an analog timing promise. Hold events can pad a desired logical tick
grid. No propagation delay,metastability or electrical race prediction. Explicit
model assumptions accompany the Chinese guide and detailed model note.

## Editable UI

| Key | Action |
| --- | --- |
| F1 SETUP | LEFT/RIGHT select external initial levels,Q0-Q3 orG0-G3;UP/DOWN field;EXE cycles value |
| F2 EVENT | Input timeline;UP/DOWN event,LEFT/RIGHTI0-I3;0/1 sets,2 restores hold,EXE cycleshold/0/1 |
| F5 inEVENT | Append held-input event, up to64;DEL removes selected,AC clears timeline |
| F3 WAVE |4signals x8event slots;UP/DOWN selectsignal,LEFT/RIGHT column;fixed labels/ruler and page/continuation markers |
| F4 TABLE |Exact0/1/X values using same cursor/pages;disabled sources shown '-' |
| EXE inWAVE/TABLE |Event summary:input before/after levels,settled values,eachcell cause andphase count |
| EXE inDETAIL |Replay intermediate phase snapshots;UP/DOWN phases;LEFT/RIGHT event |
| F5 inresults |Event summary |
| F6 RUN |Validate/edit-dependent invalidation,simulate entire timeline;show error instead of oldwave if invalid |
| OPTN |Digits0-9 explicitly load examples;EXE opens help;EXIT/otherkeys cancel |
| EXIT |Home, retaining timing problem,cursor andview |

SETUP shows all fields, including conditionally unused ones. Device fields:
enabled,type,initialQ,dataA/dataB,clock/enable,FFedge,latchpolarity,resetmode/source/
polarity/value. Gate fields:enabled,operation,inputA/inputB. NOT ignoresB;D/T and
D-latch ignoreB;edge applies toFFs,level to latches. References to unused inactive
pins are ignored by validation. Source edits invalidate derived views untilRUN;
failed append65 leaves the previous problem unchanged with explicit feedback.
Disabled rows use '-' or grey dashed rails; enabled unknownX uses red crosses.
On-demand phase replay avoids permanently storing64x32intermediate snapshots.

### Product examples (OPTN, digit, F6)

| Digit | Case |
| --- | --- |
|1|ReviewTiming1:high-master/low-slaveD latches;Q0=master,Q1=slave,G0=!Q1|
|2|ReviewTiming2:Q0highD-latch,Q1risingD,Q2fallingD;G0/G1/G2 complements|
|3|ReviewTiming3:T0clockI0,T1clockQ0,T2clockQ1;afterrisesQ2Q1Q0=7,6,5,4,3,2,1,0|
|4|ReviewTiming4:four common-clockT;initialQ0Q1Q2Q3=1110,numeric7;then8..15,0..7;G0=T2,G1=T3|
|5|ReviewTiming5:syncactive-lowNAND(Q0,Q2)reset;states1,2,3,4,5,0,1,2;G0=T2,G1=reset|
|6|D flip-flops Q0SYNC/Q1ASYNC;resetI2 asserted between edges distinguishes behaviour|
|7|SR invalid ->X and downstream synchronous propagation,then known recovery|
|8|SR level latch:hold,set,invalid,clear,hold,set|
|9|Transparent NOT feedback,expected nonconvergence error|
|0|Maximum64events/65columns,4synchronousTcells/12visible signal slots|

Presets model the ordered input changes from review diagrams, not their exact
printed x-coordinate spacing. Bit order is always explicit:Review3/4 printQ0
first while conventional numeric vectors use highestQ first. Initial and
intermediate/settled states must not be confused.

## Build and verify

```sh
bash apps/ecen21/tests/run-host.sh
bash apps/ecen21/tests/run-sequential-exhaustive.sh
bash apps/ecen21/tests/run-numeric-exhaustive.sh
bash experiments/boolean/run-tests.sh
bash scripts/toolchain-build-m6.sh
/opt/homebrew/bin/python3.11 scripts/verify-m6.py
ctest --test-dir .phase1/emulator-build --output-on-failure
```

Pinned cached ARM64Docker toolchain/native emulator are reused;host screenshot
comparisons require Pillow. Current outputs are isolated under `.phase1/ecen21-m6/`.
Only `artifacts/ECEN21.g3a` is a user app;EC21Test/EC21Bench are test-only. Source
manifests and actual target-written reports are required; headless script-tail
termination isn't success. Existing build aliases forward current source intoM6;
older helper snapshots remain archived. New targeted preset goldens validate the
actual product configs, independently of core-local fixture configs. OldBoolean/
numeric/FF/M5scripts are rerun against archived tool pixels, excluding intentionally
changed home/about pages. M6scripts split into three independent runs to fit the
pinned emulator's bounded script storage. A separate64-event benchmark includes
initialization,simulation,phase replay,checks andI/O;its instruction count is not
physical latency or a worst-case analog timing claim.

Design/testing: `docs/M6-PLAN.md`,`docs/M6-MODEL.md`,rootTESTING. Deferred joint
physical acceptance: `docs/M6-USER-ACCEPTANCE.zh-CN.md` plus retainedM5 guide.
No source-framework rewrite or futureM7/M8/M9 implementation is included.
