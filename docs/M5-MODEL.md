# M5 portable sequential design model

`apps/ecen21/src/sequential.[ch]` composes the accepted Boolean parser/exact
minimizer and flip-flop characteristic/excitation queries. It does not edit or
replace those engines, M3, upstream Logic/Base-N or shared native UI code. New
model code has no heap allocation, recursion, file I/O or gint dependency.
Existing expression parsing and exact minimization retain their bounded recursive
paths; this wrapper does not claim to remove those paths.

## Domain, specification and mapping

The fixed-size `sq_model` supports2-3 binary state bits,0-1 binary control E,
per-bit D/T/JK/SR, a common rising/falling active edge, RETURN/HOLD/DC unused-state
policy, initial/recovery states, fixed E, optional32-event E timeline and two
optional present-state output expressions. `types[0]` denotes Q0. Default is
three T devices, cycle03570, RETURN0, initial0, E disabled and active rising edge.

Cycles use a bounded31-character field and at most9 state digits including an
optional repeated closing state. Digits0-7 are individual decimal states; spaces,
commas and arrow characters are optional delimiters. Exactly one trailing repeat
of the first state is normalized. After normalization require at least2 distinct
states; internal duplicates, invalid digits/characters and active-width range
violations give a field/character position. The preserved two-cycle fields allow
E0/E1 branches without replacing one branch when editing the other.

Expressions use the accepted79-character Boolean grammar. Variable order is
highest Q first, then lower Q, then E if enabled: three-bit controlled problems
map A=Q2,B=Q1,C=Q0,D=E; two-bit controlled problems map A=Q1,B=Q0,C=E.
Rows use **(Q << controlled) | E**, with E omitted when disabled. Thus E is the
least significant truth-table variable, not an implicit E-first ordering.
`sq_variable_name` exposes the mapping rather than requiring UI inference.

Input equation slots are indexed **bit*2 + input**: input0 is D/T/J/S and input1
is K/R (unused for D/T). Equation text fields are2+that slot; cycle fields0/1,
output fields8/9 and timeline field10. Derived pins are ordered highest Q first,
then input0/input1, with explicit pin names such as D2,T1,J0,K0. This separates
editable fixed slots from the variable-length result pin array.

Setters invalidate all derived query availability. `sq_invalidate` is also
available for a UI controller changing primitive settings. An oversized text edit
is rejected without replacing the old string, and a sticky invalid-field bit
prevents solving that active field until explicitly repaired. This avoids
reinterpreting a truncated prefix or an old value as the rejected input.
Inactive fields are retained but ignored: unused K/R slots, reverse cycle fields,
forward input equations, E1 when E is disabled, and E timelines with E disabled.
Enabling a previously rejected active field requires repair. Optional outputs
are always validated if present. Empty output means 'not specified', never zero.

## Forward design and completed equations

Each E branch's listed cycle defines its explicit successors. Unlisted rows use
RETURN to the configured recovery state, HOLD on current Q, or unconstrained DC.
An unconstrained next-state row is distinct from an excitation X: the latter
means multiple legal input assignments realize one specified transition.

`sq_synthesize_table` is a portable table entry used for independent host fixtures
and possible local composition, not an alternate native user mode. The caller
provides target rows plus disjoint used/DC masks. It rejects masks outside the
active domain and any cared target outside the state width. A specified row uses
M4's excitation query for each bit. Per-pin ON/DC masks then enter the unchanged
exact minimizer independently for SOP and POS.

The model stores **both completed next-state tables** as `rows[r].next[form]`.
Do not assume SOP/POS choose the same values on unconstrained rows. For example,
an all-DC two-bit T fixture yields SOP input0 (hold) and POS input1 (toggle):
next states and recovery trajectories legitimately differ by selected form.
The specified target is separately stored as `target`, guarded by `care`.
`SQ_UNDEFINED=255` is a nonbinary marker for unconstrained requested targets or
invalid actual next state; views must not print it as an ordinary state number.

Every completed pin equation is replayed through characteristic semantics using
one old-state snapshot before committing the next vector. Every cared row must
match its target for both forms. `checked_rows` counts cared **form/row pairs**
(two per constrained row), not the number of distinct states.

Exact cost means the existing per-pin lexicographic cube-count/literal-count
objective; no global shared-gate optimality is promised. On-demand formatting
avoids retaining12 separate256-character equation buffers. Form-specific parity
masks/complement flags are detected only when they equal the actual selected
completed equation on every row, allowing verified XOR/XNOR connection sharing
without changing the SOP/POS evidence. Masks index truth-row bits, so variable
index v corresponds to bit1<<(variables-1-v).

## Reverse analysis and outputs

Reverse mode requires every active input equation. It parses the original text
through the accepted Boolean solver into fully specified input masks, retaining
that text while producing equivalent minimized views. All pins use the same old
Q/E. Each row receives an actual next vector or an explicit invalid SR marker;
invalid rows remain visible rather than making all otherwise useful analysis
unavailable. SOP/POS must agree in reverse mode because input functions are
fully specified. The displayed excitation constraints can contain X even though
the supplied equations choose concrete pin values; `sq_pin_value` exposes the
actual selected equation value separately.

Z0/Z1 expressions are evaluated from present Q/E before the state update.
`sq_output_value` returns-1 when absent/invalid, and output masks are identical
for both minimization forms. No half-cycle Clock-dependent output is inferred;
that belongs to later timing/integration work. Input/output expression evaluation
uses local bounded `bs_model` scratch, avoiding hidden mutable global parser
state. The wrapper adds a measurable stack frame to the inherited parser path.

## Trace, edges and recovery

`sq_next` is a const query. NONE or the wrong edge holds a valid binary current
state; the selected edge reads the completed table. If active-edge SR inputs11
make the next vector undefined, the query fails without overwriting its output.
Negative/unsupported form/event enum values are rejected before indexing arrays.

`sq_trace` returns the initial state followed by computed states. Empty/NULL
E timeline traverses fixed E until a repeated state, with explicit transient
prefix and cycle period; a finite-state map repeats within2^bits transitions.
A nonempty controlled timeline accepts at most32 binary characters and reports
its finite trace without inferring a cycle from repeated Q under different E.
With E disabled the inactive timeline is ignored. Invalid SR stops the trace
without a fabricated next binary state. Views can call this query for every
unused starting state to show recovery, traps and selected-form DC completions.
No hardware reset pin, ripple-clock propagation or latch behaviour is assumed.

## Independent host verification performed

Agent-local strict Clang C11 builds ran with `-Wall -Wextra -Werror -pedantic`,
ASan and UBSan. Actual final results:

```text
PASS: M5 sequential; 2216 checks; failure line 0
PASS: M5 exhaustive sequential; 916124 checks; failure line 0
```

Evidence is ignored under `.phase1/m5-core/`: shared.log,exhaustive.log,temporary
shared main and executables. Shared entry is `sq_run_checks(unsigned*)`; the
counter is initialized independently, and the return is zero or a failing source
line. `SQ_FULL_HOST_TESTS` enables `sq_run_exhaustive`; the full sweep is excluded
from the ordinary target harness.

Shared fixtures cover all mandatory review sequences, mixed D/T/JK, controlled
Counter2 with complementary3-input parity, maximum3Q+E domain, the visually
corrected Counter3B equations/output, transient cycles, simultaneous-update swap,
no/wrong edge, falling edge, SR-invalid trace, all policies, form-specific DC
completion, recovery traps, max32-event timeline and query/settings errors.
They include malformed79-open-parenthesis and79-NOT expressions (errorpos79) plus
a valid78-NOT+A79-character expression through the new wrapper, for later target
stack-path execution. Oversize sticky-field repair and inactive-field semantics
are also exercised.

The host sweep synthesizes/replays5,408 model cases:

- Every256 deterministic two-bit map with all16 per-bit type combinations:4,096.
- All2/3-bit constant-target maps across every16/64 type combination:576,
  covering every binary present/desired transition for each type assignment.
- Every2/3-bit type combination under all RETURN/HOLD/DC policies, with/withoutE:
  480 partial-cycle designs, including recovery/DC rows and invalid DC completions.
- 256 deterministic three-bit maps, half controlled, with reproducible next states
  and varying type assignments. This samples the larger domain, not all eight-
  state maps or all controlled specifications.

An independent literal device truth table replays completed vectors without
calling production characteristic evaluation. A separate cube-membership
interpreter checks equations without calling `eb_evaluate`. An all-implicant
union dynamic program checks minimum per-pin cost for every two-bit map and
representative three-variable fixtures; production instead uses prime-cover
search. Four-variable global coverage remains the existing exact-core regression
responsibility; this subtask does not claim exhaustive4-variable ternary tests.

## Integration and resource limits

Host Clang `-Os -fstack-usage` observed expression wrapper848 bytes,sq_solve272,
synthesis144,minimization/replay208 and trace112. These are host compiler frames,
not SuperH measurements or physical peaks; parser/minimizer recursion and library
helpers add overhead. No GNU host compiler was available; an early Main target
build exposed GCC misleading-indentation warnings, corrected with explicit guard
braces/separate statements before final strict host reruns. Target compilation,
shared SuperH execution, production interaction/netlist checks and final firmware
identity belong to Main Agent and root `TESTING.md`. No physical calculator,
latency or peak-memory result is claimed by this model handoff.

## Main integration evidence

Main independently reran the strict host suites and the same shared assertions
as actual SuperH code. The target's six-line passing report exactly matches host.
Production100 M5 screenshots/20 invariants and accepted-tool pixel regressions
pass. Final source/artifact/resource/coverage limits and representative maximum-
domain measurement are recorded in rootTESTING. No physical M5 execution is
claimed; M5 user acceptance is pending and M6 remains unapproved.
