# ECEN21 Toolkit product requirements

Status: Phase1/M2A/M2B/originalM3/M4 accepted. M5/M6 engineering passed,hardware deferred. Custom-width follow-up/M7 engineering passed,user acceptance pending. M8 explicitly authorized and engineering-verified,user acceptance pending. M9 unapproved. Historical requirements below are preserved;current evidence is in TESTING and coverage matrix.

## Product outcome

A complete, offline, problem-type-based Digital Logic Solver for Casio fx-CG50.

The intended user selects a coursework problem type, enters structured conditions,
and obtains a verified answer plus the intermediate tables, minimization and
connection instructions needed to understand and reproduce the design. A table
or formula viewer alone is not an end-to-end solver. No network is required by
the delivered calculator app; development-time host tools may verify results.
“Complete” is an eventual outcome within published limits, not a description of
M4. [Problem coverage](docs/PROBLEM-COVERAGE.md) is the coverage authority.

## Baseline and scope protection

Phase1, M2A, M2B, M3 and M4 are accepted. The user explicitly confirmed real
Casio fx-CG50 acceptance and closure of M4. The stable checkpoint is documented
in [M4 checkpoint](docs/checkpoints/M4-ACCEPTED.md).

The user subsequently authorized a bounded M3 follow-up: custom1..32bit widths are engineering-verified, user acceptance pending. Original8/16/32 presets remain. Extra
number encodings and arithmetic features are Future Backlog only; they are not
part of M5-M9. Do not refactor/reimplement M3 to make coverage statistics look
better. Upstream Logic/Base-N/common and accepted engine behaviour are preserved.

## Common problem workflow and result contract

1. Select problem type and specify widths, labels, polarities, initial state,
   control values, flip-flop types or device parameters as appropriate.
2. Validate the complete specification: ranges, conflicting rows, syntax,
   unsupported variable counts and ambiguous timing must produce clear errors.
3. Build a bounded truth/state/event table using the specification's assumptions.
4. Derive next states or excitations; call existing verified Boolean engines when
   the variable domain fits. Never relabel a partial result as a complete design.
5. Show simplified equations, K-maps/groups where applicable, and a structured
   list of components, pins and connections. Link each equation to its source
   column/table; offer verification against every specified row.
6. Retain the same problem data across result views. Editing invalidates all
   dependent answers; navigating must not silently replace a problem with a demo.
7. Report assumptions, unused/illegal-state policy and coverage limits alongside
   the result. A verified equation need not match a textbook's spelling if its
   specified behaviour and declared optimization objective are equivalent.

Proposed reuse path (no new implementation in this planning task): structured
problem -> portable row/event model -> existing flip-flop truth/excitation and
Boolean evaluation/minimization -> verification -> native views. M5 can compose
these existing modules through local adapters; it does not require a framework
rewrite. M6/M7/M8 later consume compatible tables/equations. A new global IR,
parser rewrite or shared-engine domain expansion needs architectural review.

## Milestone roadmap

| Milestone | Required solver outcome | Completion boundary |
| --- | --- | --- |
| M5 Counter & Sequential Design | State sequence -> present/next table -> per-bit D/T/JK/SR excitation -> exact SOP/POS/K-map -> sequence verification -> circuit connections. Mixed types, E-dependent sequences and reverse equations -> state/output/transition analysis included | All four counter categories and three-bit examples pass; M5 guide delivered; user explicitly deferred hardware acceptance while approving M6 |
| M6 Timing Diagram & Sequential Simulation | D/T/JK/SR, level-sensitive latches, positive/negative edges, master-slave pairs, connected devices, sync/async reset, initial states; signal/state tables, waves, intermediate values and event explanations | Review Timing1-5 fixtures and model assumptions validated; no propagation-delay claim |
| M7 Combinational Circuit Design | Half/full adders, ripple carry structure, MUX implementation,4-to2/8-to3 encoders,7-segment logic, truth table -> equations -> gates/MUX connections | Full P4.12, MUX, encoder and full-adder workflows, not just reusable Boolean fragments |
| M8 Verilog Code Generator | Parameterized behavioral/structural full adder;4-to2/8-to3 encoder;D/T/JK;universal shift register;modulo-N synchronous-reset counter;ring counter | Generated source actually passes host syntax/behaviour tests, with parameter edge cases and delivery method verified |
| M9 Integration, Comprehensive Problems & Release | Counter plus output logic, state table -> minimization -> circuit equations, P5.26-class complete design, consistent UI, regression, hardware performance/memory, guides and versioned ECEN21.g3a | End-to-end cross-module fixtures plus measured hardware limits and explicit release acceptance |

M5's proposed bounds/semantics and test IDs are in [M5 plan](docs/M5-PLAN.md).
Higher stages still require their own approved bounds and detailed acceptance
plans. The user subsequently approved bounded M5 implementation with “开始M5吧”; later stages still require separate approval.

## M6 model and display (implemented; hardware acceptance deferred)

Model ideal zero-delay digital behaviour, not analog device behaviour. Explicitly
specify enable polarity, active clock edge, reset polarity/value/priority and
initial state per device. A transparent latch follows D while enabled and holds
when closed; an edge-triggered flip-flop samples only at its active edge. A
master/slave pair is two opposite-enable latches with visible Qmaster/Qslave,
not an arbitrary replacement by a single D flip-flop.

Timing1 (p21) requires a transparent-high master and transparent-low slave.
Timing2 (p22) compares a high-level latch, rising D and falling D on shared D/CLK.
Timing3 (p23) uses cascaded derived clocks: Q0 clocks the next T device. It is not
a single-clock synchronous counter. Timing4 (p24-25) requires four synchronous T
stages and intermediate T1/T2/T3. Timing5 (p26-27) explicitly says soft-reset at
the next clock trigger despite bubbles in the symbol: preserve the stated
synchronous active-low reset, and test asynchronous reset separately.

Proposed first bound: at least4 state elements (or two master/slave pairs),
12 named visible signals and64 external event positions. Validate resources in
M6 before advertising final limits. Derived-clock events need bounded settling
phases at the same logical time; synchronous stages snapshot inputs before any
state commits. Reset priority, simultaneous input/clock changes, latch opening/
closing and nonconvergent zero-delay feedback require explicit ordering or an
ambiguity/error result. Never resolve races by iteration order. Unknown/invalid
states need a declared representation; M4's binary-only model is not sufficient
for every timing task. M6 implements an explicit three-value X model with documented atomic-input-first ordering and bounded phase settling, without changing the M4 binary model.

396x224 screen proposal: freeze a left signal-label column and top time ruler;
show4 signal rows by8 event slots, with high/low rails and vertical transitions.
LEFT/RIGHT scroll time, UP/DOWN scroll signals, EXE opens a cursor with exact
before/after values and trigger reason. Page counts, time range and off-screen
continuation marks remain visible. Offer the corresponding numerical table and
an event-detail page; a waveform must never be the only readable result. M6 now implements this bounded waveform/table/cursor workflow; see `docs/M6-PLAN.md` and `docs/M6-MODEL.md` for as-built ordering/limits.

## M7 constraints — implemented within approved bounds

Reuse M2B within its2-4-variable domain. P4.12 must yield two4:1 MUX connection
lists with selectors x0,y0: sum data [c0,!c0,!c0,c0], carry [0,c0,c0,1]. Show the
select/data ordering explicitly. Full-adder sum/carry and three-bit popcount
(P3.21) use the same truth tables/structural components. Ripple-carry analysis
shows each carry link and stage value, without claiming physical delay analysis.

For encoders distinguish one-hot ordinary encoding (zero/multiple asserted inputs
invalid) from optional priority encoding; do not silently infer priority. An
8-input encoder exceeds the Boolean core's four-variable domain: use its
structured one-hot family semantics, exhaustive256-input validation and explicit
invalid cases, not an unapproved general8-variable minimizer. For7-segment logic
state segment order a-g, active polarity, digit coding and invalid-code policy;
at least BCD0-9 with outputs a,b,c via MUX is required, with all seven equations
available for the defined mapping. No arbitrary graphical circuit editor.

## M8 generation and verification contract — implemented within bounds

Generate usable Verilog (proposed Verilog-2001 subset) from named parameters,
not a fixed code gallery. Specify ports, widths, reset polarity/synchrony,
shift direction, mode priority, one-hot/priority encoder behaviour and unsupported
parameters. Support parameterized n-bit adders (including the review's5-bit
case), shift/ring widths and modulo-N count width. A one-bit full-adder primitive
and its structural n-stage chain must have consistent carry wiring. T/JK state
updates use the same accepted characteristic semantics as other solvers.

Host syntax/elaboration and independent testbenches are mandatory; select an
available Verilog simulator only when M8 is authorized, record tool versions and
actual results. Check all full-adder combinations, encoder one-hot/invalid
inputs, D/T/JK event/hold, all shift modes including simultaneous mode requests,
modulo12 wrap/reset, N=1/non-power-of-two handling and ring one-hot rotation/reset
for n=1 and representative larger values. No calculator-side compiler is
required. Plan paginated code view plus USB-accessible text export with clear
file naming/capacity errors; the export mechanism must be verified in M8 and is
not already implemented.

## M9 integration and definition of done

Use the same problem data from state specification to equations, simulated
behaviour and generated circuit description; connect existing solver outputs
rather than requiring manual retyping. P5.26 requires Clock-dependent output f
in addition to state-based g, with Start's synchronous reset semantics and
half-cycle simulation. A state-only table is insufficient. Derive tables first,
verify equations independently, then reproduce the requested complete timing
trace and connections. Consolidate UI without losing accepted functionality.

Every milestone requires important-module automated tests, freshECEN21.g3a,
actual emulator interactions, accepted-feature regressions, source/artifact
provenance, resource observations, updatedPLAN/PROGRESS/TESTING and the seven-part
Chinese physical-user guide. User acceptance and approval of the following
milestone are separate gates. M9 additionally needs hardware timing/memory
measurements with procedure, device/OS and measured limits; the M4 acceptance
report does not supply those measurements. Default one Main Agent; delegate only
independent useful tasks with isolated ownership. No automatic phase crossing.

## Future Backlog, excluded from M5-M9

Custom number bit widths1..32 (including5/10/12) were separately authorized and engineering-verified; follow-up user acceptance pending. Remaining backlog: sign-magnitude/ones-complement,
comparison-specific tools,64-bit/multiply/divide/shift extensions and M3 redesign.
Also defer general symbolic state minimization/automatic state encoding, arbitrary
large-variable Boolean synthesis, analog delay simulation and graphical circuit
editing unless separately approved. This backlog is not a delivery promise.
