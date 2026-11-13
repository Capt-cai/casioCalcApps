# M7 proposal — Combinational Circuit Design Solver

Status: APPROVED by explicit user instruction “批准进入M7”. Engineering verification passed; user acceptance pending.
M5/M6 hardware stays deferred; custom-width follow-up acceptance remains pending.
M4 is the accepted stable checkpoint. Width/M6 engineering snapshots are preserved.

## Product workflow

Select a problem type -> enter structured requirements -> inspect truth/stage
 tables -> minimize each output -> inspect actual gate/MUX connections -> replay
all cared inputs to verify equivalence. Outputs must share one problem model;
examples alone or equations without connections do not satisfy this milestone.

## Proposed bounded scope

| Mode | Inputs | Required outputs and bounds |
| --- | --- | --- |
| Half/full adder | HA/FA selection, x,y,carry-in for evaluation | Complete 4/8-row truth table; sum/carry equations; SOP/POS and K-map; XOR/AND/OR implementation and gate replay |
| Three-input population count | Three input bits | Count 0..3; two output tables/equations; explicit full-adder cout,sum interpretation; P3.21 circuit aspect only |
| Ripple-carry structure | Width 1..8, unsigned operands and carry-in | One full-adder per bit, low-bit-first carry chain, each stage x/y/cin/sum/cout, assembled width-bit result and carry-out; no M3 modification or propagation-delay prediction |
| Generic truth/equation design | 2..4 inputs; one selected output at a time; truth values 0/1/DC or existing expression syntax | Truth table, exact SOP/POS, K-map and gate connections; constants and complement handling; invalid input clears stale results |
| MUX implementation | 2..4-variable function, 2:1/4:1/8:1 MUX and distinct ordered select variables, within available input count | Every data pin's cofactor expression/connection, select order, residual gates and verification over all cared rows; no guarantee of globally minimal gate/MUX count |
| Ordinary encoder | 4-to-2 or 8-to-3; active-high one-hot inputs | Input-pattern evaluation, code, separate valid flag, one-hot design equations/connections, zero/multiple input diagnostics, and paged full 16/256-pattern table; no inferred priority |
| Seven-segment display | Fixed decimal BCD mapping 0..9, polarity, invalid-code policy | Explicit a..g layout and all seven truth columns/equations/K-maps/gate connections; selectable segment MUX implementation, especially a,b,c; invalid10..15 policy is zero output pattern, blank display, or explicit DC |

Widths above are proposed initial resource bounds, subject to actual target
compile/run measurements. A failed resource check must be reported with a
reviewable smaller-bound proposal, not silently drop required exam cases.

### Conventions that must be visible

- Truth variables have an explicit most-significant-to-least-significant order.
  MUX data Dk uses the binary value of the displayed select order, S1 then S0.
- P4.12: S1=x0, S0=y0; sum data [c0,!c0,!c0,c0], carry data
  [0,c0,c0,1]. Connect two actual 4:1 MUXes and replay all eight input cases.
- Ordinary encoder asserts valid exactly for one active input. Code pins are
  meaningful only when valid; deterministic actual invalid-case pin values must
  still be disclosed. They must never be presented as priority encoding.
- Encoder family equations describe one-hot design semantics rather than an
  eight-variable exact minimization. Verify emitted gates on all256 patterns,
  including the validity condition and actual invalid outputs.
- Seven-segment a=top,b=upper right,c=lower right,d=bottom,e=lower left,
  f=upper left,g=middle. Show both logical segment-on pattern and physical pin
  polarity. Distinguish all-zero pins from blank when active-low is selected.
- DC means unconstrained input rows. Show the chosen implementation's actual
  completion; correctness is mandatory on cared rows, not on an imagined DC
  value. Minimized gate and MUX completions may differ on DC rows.
- No claim of minimum physical gate count, analog delay or hazard-free design.

## Reuse and isolation

Reuse exact_boolean's verified2..4-variable SOP/POS engine and K-map layout,
existing expression parsing through a local adapter, and proven navigation/
connection formatting patterns. Preserve old computational/UI modules, upstream
Logic/shared code, accepted artifacts, and M3 unchanged. The existing
seq_connections API is coupled to sequential models; do not refactor it into a
new global representation just to implement M7. Prefer new bounded portable
combinational model/mapping modules with a local UI. Independently replay the
emitted gate/MUX descriptions rather than checking only the minimized equations.

Conceptual pipeline (not a newly approved global architecture): bounded problem
specification -> tables -> existing minimizer or structured family equations ->
local connection description -> independent equivalence checks -> paged UI.
Do not automatically connect every M7 design to M6: cross-module simulation and
comprehensive composition remain M9. Verilog generation remains M8.

## Small-screen interaction proposal

Add a Combinational entry; select problem families inside it. Keep input settings,
truth/stage tables, equations, K-map, connections and verification as paged views
of the same retained problem. Use fixed column labels and explicit bit order;
show one selected output's derivation at a time, with segment/output switching.
Connection lines name components and individual select/data/carry pins; support
scrolling rather than clipped equations. Mark unsolved/invalid/stale states and
capacity rejection visibly. Reentry/help must preserve problem and cursor.
Exact keys/page allocation are implementation choices after scope approval.

## Work order after explicit approval

1. Freeze the M6 engineering source/artifact/evidence and original build helpers;
   keep hardware-deferred labels. Establish isolated M7 outputs and baseline.
2. Implement portable adder/general-function models and independent truth fixtures.
3. Implement gate and MUX mapping with independent connection evaluators; verify
   P4.12 before UI work. Keep adapters local to the new module.
4. Add bounded ripple chains, ordinary encoders and BCD seven-segment families;
   make all exceptional/invalid policies explicit and testable.
5. Integrate retained-state, paged native UI, structured examples and editor checks.
6. Run full host/target/production-emulator checks; review resources and regressions;
   update source-linked coverage and deliver a Chinese seven-part hardware guide.

Default one Main Agent; delegation only if separately permitted/required and useful.
Do not interpret these technical work steps as independently approved milestones.

## Planned acceptance tests — not executed results

| ID | Required check |
| --- | --- |
| M7-A1 | Exhaustive4HA/8FA rows; FA sum=x XOR y XOR cin, carry=xy+x*cin+y*cin; popcount equals number of asserted bits |
| M7-A2 | All operand/cin combinations for each width1..8; verify every per-stage carry and assembled result against independent integer arithmetic |
| M7-A3 | P4.12 exact two-MUX select/data assignment above; all8 rows replay through the emitted connections |
| M7-A4 | Exhaust every Boolean function through3 variables, all legal select orders/MUX sizes; independent representative4-variable and DC fixtures; compare emitted gates/MUX to cared rows, including constants |
| M7-A5 | All16/256 encoder patterns: code/valid/diagnostic, gate replay, no implicit priority or misleading invalid code |
| M7-A6 | Independent decimal glyph reference for all seven segments; both polarities and every invalid-code policy over16BCD values; a,b,c MUX replay, plus selected other segments |
| M7-A7 | Edit invalidation, invalid/duplicate selector rejection, long equations, table/output/connection paging, help/cancel/reentry, error recovery and previous-tool retention |
| M7-A8 | Fresh production ECEN21.g3a and SuperH target fixtures; host sanitizers; native emulator arithmetic/equivalence and UI scripts; prior tool regressions; resource observations and manifest/hash provenance |

Acceptance reports must distinguish host checks, actual target execution,
production UI evidence and physical calculator testing. The user has deferred
M5/M6 physical testing; M7 hardware status and any later-stage authorization
must be explicitly recorded rather than inferred. No test count or coverage
status is promoted to passed by this plan.

## Exclusions and next decision

No M3 custom widths, general5..8-variable minimizer, priority encoder, arbitrary
circuit drawing, analog timing, Verilog generation or M9 integration in M7.
Next concrete task after approval: preserve M6 engineering checkpoint and build
the portable HA/FA model with independently evaluated truth/connection fixtures.

## As-built engineering scope

Implemented the approved bounded modes in new portable combinational modules and
native UI, with local adapters to unchanged exact Boolean/parser interfaces.
Selected-output workflow preserves family requirements. A Sum MUX and a Carry
MUX have separate named instances; gate node IDs are local to that output.
K-map groups highlight ON cubes for SOP/OFF cubes for POS. Generic expressions
are bounded79characters; connection/formula pages preserve content. Runtime
verification evaluates actual gate/MUX pin connections and cared truth rows;
DC completion is disclosed and identical for gate/MUX paths in this implementation.
Encoder has explicit validity circuitry and no implicit priority. Ripple exposes
all carry links/stages. Existing cores/UI remain unchanged except launcher.
Final actual results/artifact are recorded in root TESTING/PROGRESS and the
Chinese acceptance guide; no physical acceptance is inferred from these checks.
