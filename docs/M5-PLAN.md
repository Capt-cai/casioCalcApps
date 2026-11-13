# M5 detailed proposal - Counter & Sequential Design Solver

Status: **Approved for implementation** by the user's “开始M5吧”. M4 is accepted
on real fx-CG50 and closed. Engineering verification is complete; M5 user acceptance is pending and M6 is unapproved. Tasks below belong to this single approved milestone, not permission
to advance to M6. See [PRD](../PRD.md) and [coverage](PROBLEM-COVERAGE.md).

## Implemented approved limits

Solve four problem classes through one table/equation/verification workflow:
sequence design, mixed flip-flop sequence design, E-controlled sequence design,
and reverse analysis from flip-flop/output equations. A preset is a reproducible
example, not a substitute for editing arbitrary in-domain inputs.

Implemented **2-3 state bits**, **0-1 binary control input E**,2-8 unique states per
cycle (up to2^n for the selected width), per-bit D/T/JK/SR and0-2 optional output
equations over present Q and E. At most16 present-state/control rows and6 input
pin functions. Reverse expressions use the accepted79-character grammar and
explicit variable mapping (Q2,Q1,Q0,E -> A,B,C,D for three-bit controlled tasks).
For two bits, Q1,Q0[,E] map to A,B[,C]. No implicit letter/bit permutation.

Rationale: the accepted exact core is2-4-variable only. Three state bits plus E
fits exactly; three bits without E and two bits with/without E also fit. This is
a compatibility argument, not new runtime evidence. Do not advertise4-bit
controlled synthesis (five variables) or extend the core in M5. Four-state-bit
uncontrolled designs could fit its truth domain but need separate resource/UI
verification; keep them outside the recommended first M5 acceptance boundary.
The largest first-version domain must run on target and have recorded compiler,
emulator and later hardware observations before calling it supported.

## Structured input and validation

- Mode A/B/C: state-bit count, Qn-1..Q0 bit order, common active edge, per-bit
  flip-flop type, ordered decimal state list(s), initial state and unused-state
  policy. With E, enter a separate cycle for E=0 and E=1; E selects behaviour, not
  automatically an enable/hold signal. Missing transitions outside each cycle
  use that E branch's explicit unused-state policy.
- A trailing repeat of the first state closes a cycle and is normalized once;
  internal duplicate states are rejected rather than guessing conflicting
  successors. Reject out-of-range/empty/over-capacity lists and malformed values;
  show the exact row/position. Singleton hold cycles may be represented through
  HOLD policy, not silently interpreted as a general cycle of length2.
- Mode D: one equation for each required input pin (D/T, J and K, S and R),
  optional Z0/Z1 equations and initial state. Variables reference present Q/E
  only; reject missing pins, next-Q feedback references and malformed syntax.
  Use existing Boolean syntax (!,AND,XOR,OR,parentheses/constants) through an
  explicit mapping adapter, without modifying accepted parser behaviour.
- Changing any input/settings invalidates all derived equations/tables/trace and
  clears any prior success indicator. Preserve the problem when paging or
  switching tools. Samples replace a problem only on explicit selection.

## Exact semantics

All stages share one clock and one selected edge. Evaluate every pin and output
from the same old state and E, then commit all next-Q bits together. A chained
clock/ripple event is not M5 synchronous updating (it belongs to M6). No hidden
reset pin: initial Q starts a trace; hardware initialization requirements are
listed in the connection description.

Unused-state choices must be visible in input and output:

1. **RETURN** (default): one specified in-range recovery state, initially0.
   Every unused row must transition there, even when the recovery state itself
   is outside the intended cycle; report if this produces a trap rather than
   entry to the intended cycle. Review Counter1 uses RETURN0, not don't-cares.
2. **HOLD**: unused rows hold their present state. Report isolated/self loops.
3. **DC**: unconstrained unused next-state rows permit input minimization.
   Clearly distinguish these from excitation X. Evaluate the chosen completed
   equations on every unused state and report their actual behaviour; no
   self-start/recovery guarantee is allowed. Possible SR11 must be reported as
   undefined, not converted to a valid state or silently simulated onward.

An excitation X means multiple input choices legally realize a specified
transition. Minimize each pin independently using ON/DC masks; for specified
rows, recombine the resulting inputs through characteristic functions and verify
that the entire next-state vector matches. Reject/diagnose SR11 on cared rows.
Exact means minimum product/clause count then literals **per pin** in the existing
core's domain, not globally minimum circuit area/shared gate count. Provide SOP
and POS views, source ON/DC rows, K-map groups and semantic equivalence checks.
A compact XOR/XNOR display is an additional verified representation, never a
substitute for exact SOP/POS evidence. Recognize bounded parity/complemented
parity and reuse its signal in connection lists to meet Counter2's one3-input
XOR plus one NOT constraint; do not claim arbitrary technology mapping.

Optional reverse outputs Z0/Z1 are evaluated from present Q/E, labelled before
update; a separate next-row value is not the same output instant. Without
output equations display 'not specified', not zero. Reverse SR-invalid rows are
shown with no valid next vector and stop any trace reaching them. Analysis does
not silently replace the supplied input equations by newly synthesized ones;
optional minimized equivalent views retain the original equations.

Trace state sequences until the augmented state repeats: for fixed E, at most
2^n distinct states before repetition; report transient prefix, cycle and period.
For a user-supplied E timeline (proposed max32 steps), apply E per step before
sampling all old-state equations. Do not infer a cycle from repeated Q under
different E events. List every unused-state starting trajectory for fixed-E
branches, including traps and undefined states.

## Required result views and connections

One structured problem owns paginated views: specification, present/next table,
excitation table, per-pin SOP/POS with costs, K-map/groups, optional output table,
trace/recovery report and connection list. A view shows row/column or page count;
no long equation or column may disappear without continuation/navigation.
Proposed table pages show4-6 rows with horizontal selection of next-state/input
columns; equation selection opens its own map/explanation. Follow accepted
native UI conventions; exact key layout is an implementation choice after approval.

Connection list includes one named flip-flop per Q bit, common CLK/active edge,
each named input pin's equation/net, feedback sources, constants/inverters,
shared parity signals where applicable and specified outputs. Every required
pin must be driven. Do not output a graphical editor or an unevaluated 'connect
according to equation' placeholder. Show initialization and unused-state policy.

## Reuse and isolated technical tasks (engineering complete)

| Task | Deliverable | Verification / integration gate |
| --- | --- | --- |
| M5.1 Specification | Portable bounded state/cycle/control model, input validation, variable map, policies | Literal table fixtures; no gint or M3 changes |
| M5.2 Forward design | Per-bit excitation via M4, ON/DC generation, existing exact core composition, SOP/POS and parity view | Independent next-state oracle for all rows/type choices; pin equations replay into next-state vector |
| M5.3 Reverse analysis | Existing expression evaluation plus simultaneous vector update, optional output tables, traces/recovery | Manual goldens, E-changing traces, SR invalid paths and order-independent updates |
| M5.4 Native workflow | Editable A/B/C/D problem entry, all result pages/maps/connections, retained navigation | Production key scripts and readable full-domain screens; invalidation/no truncation |
| M5.5 Integration | Fresh production/test add-ins, source manifest/hash, resource/latency observations, old-tool regressions | Shared host/target fixtures, pinned-emulator execution, accepted screenshots or documented intentional changes |
| M5.6 Handoff | Updated records/coverage plus seven-part Chinese guide | Engineering passed, user acceptance pending; stop before M6 |

Reuse `flipflop.[ch]` truth/excitation queries, `eb_minimize`/`eb_evaluate` and
existing Boolean expression/K-map semantics via local adapters. Keep new
controller/drawing separate; no M3 changes, core rewrite or framework migration.
Tests use independent literal/math oracles, not just production self-evaluation.
Default one Main Agent; optional independent model/test delegation only if useful,
with explicit non-overlapping file ownership and Main review.

## Mandatory acceptance fixtures (engineering checked; user acceptance pending)

| ID | Input | Required independently checked outcome |
| --- | --- | --- |
| M5-A1 |3bit all T;0,3,5,7,0;RETURN0| All8 rows including1/2/4/6->0; cycle0->3->5->7->0; T vectors below; all equations reproduce cared rows; maps and connections available |
| M5-A2 |3bit all JK;0,1,4,7,3,5,0;RETURN0| Cycle exactly as entered; unused2/6->0; legal J/K excitations and simplified per-pin equations |
| M5-B1 |3bit Q2=D,Q1=T,Q0=JK;0,3,1,2,5,4,7,6,0| All8 transitions; for000->011: D2=0,T1=1,J0=1,K0=X; for111->110: D2=1,T1=0,J0=X,K0=1; no serial-update contamination |
| M5-C1 |2bit all T;E0:0,2,3,1,0;E1:0,1,3,2,0| All8(E,Q) rows; T0=E XOR Q1 XOR Q0, T1=NOT(T0); one3-input XOR plus oneNOT with shared wiring; both branches and changing-E trace correct |
| M5-C2 |3bit + E with distinct valid cycles and mixed types| All16 rows; full existing4-variable domain exercised; unsupported5-variable request rejected clearly |
| M5-D1 |Review p33: T1=Q0+!Q1;J0=!Q0*Q1;K0=!Q0+Q1;Z=Q0|00->10 Z0;01->11 Z1;10->11 Z0;11->00 Z1; from00 cycle00,10,11,00; table/trace/output match without missing overbars |
| M5-D2 |D1=Q0,D0=Q1;initial01|One simultaneous step ->10 (a serial-update bug gives11 or00); no-edge hold; repeat works for either chosen common edge |
| M5-E1 |Same partial cycle under RETURN/HOLD/DC|Policies produce correctly constrained rows and distinct labelled guarantees; independently replay all completed equations; DC never advertised as safe recovery |
| M5-E2 |SR pins11 in reverse mode, or DC completion causing SR11|Row is undefined, trace stops without fabricated binary state; forward cared rows can never contain invalid SR |
| M5-E3 |Duplicates/conflicting cycle, missing input equations, bad syntax/range, too long entry, edit after solve|Precise bounded errors; no stale result/success; documented trailing closure normalization |
| M5-U1 |All result views/full16-row and long equations; home/tool/help roundtrips|All data accessible, retained problem, no clipping/silent truncation; field errors visible |
| M5-R1 |Accepted M2B/M3/M4 tests and UI plus freshM5 build|Host/target/emulator checks pass; original/M3 files/artifacts unchanged; emitted target report matches host; artifact/hash/resource records and Chinese guide delivered |

A1 expected next-state/T rows, independent of equation spelling:

| Q | Q+ | T2 T1 T0 |
| --- | --- | --- |
|000|011|011|
|001|000|001|
|010|000|010|
|011|101|110|
|100|000|100|
|101|111|010|
|110|000|110|
|111|000|111|

The review p28-29 equations are useful equivalent references under RETURN0:
T0=!Q2*!Q1 + Q2*Q1*Q0;
T1=!Q2*!Q0 + Q2*Q0 + Q1;
T2=Q2*!Q0 + Q1*Q0. Do not require these exact strings if another minimum
solution is equivalent; parity form and per-pin optimum are separate objectives.

Before completion, exhaustively test at least all two-bit deterministic next-state
maps with all4^2 per-bit type assignments (256*16 maps/types), and all2/3-bit
excitation transitions/policies. For three-bit designs use the required fixtures
plus deterministic generated maps/controls; record sample counts rather than
claim exhaustive eight-state machines. Compare pin minimum costs with an
independent all-cube oracle for a bounded representative domain, and run existing
exact-core exhaustive regressions. Shared target checks may be smaller than
host sweeps, with that limit recorded. Measure the largest-domain solve with a
reproducible emulator bound and compiler stack estimates; no invented physical
measurement or pre-test latency promise. A resource blocker must be resolved
within scope or brought back with measured evidence, not silently reduce the
mandatory three-bit controlled/mixed cases.

## Completion and stop gate

All four modes, every required result view and fixtures A1-R1 must pass; no
'Full Coverage' promotion based only on a related engine. Build actualECEN21.g3a
and emulator-test it, rerun relevant accepted regressions, update records/coverage,
provide Chinese installation/keys/results/pass-fail/failure-report/next-step guide
and stop for explicit M5 hardware acceptance plus separate M6 approval.

All engineering tasks/fixtures passed. Shared tests2,216 sequential/128,986 connections
also executed as SuperH code. Host exhaustive916,124 assertions cover5,408 models;
production100 screenshots/20 invariants and accepted-tool regressions pass. The
maximum4-variable example has a separate measured emulator instruction bound.
See rootTESTING for exact evidence, sampling/resource limits and artifact identity.
No physical M5 result is claimed. [Chinese user guide](M5-USER-ACCEPTANCE.zh-CN.md)
is delivered; stop for M5 acceptance and separate M6 approval.
