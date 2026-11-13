# M6 portable timing/event model

`apps/ecen21/src/timing.[ch]` is a bounded portable C11 simulation module.
It does not depend on gint, heap allocation, files, wall-clock time or new
recursive functions. Existing M4 `ff_evaluate` is reused unchanged for concrete
SR/D/JK/T characteristic transitions; no upstream or M3 code is changed.
M5/M6 hardware acceptance is deferred by explicit user instruction, not passed.

## Domain and signal references

- Four external binary inputs I0..I3 occupy references0..3. Input word bit0 is
  I0, bit1 I1, etc.; this is deliberately explicit and not an MSB-first string.
- Four state-cell Q outputs occupy references4..7, labelled Q0..Q3.
- Four combinational outputs G0..G3 occupy references8..11.
- References12/13 are constants0/1, not additional waveform rows.
- Cell types: SR,D,JK,T edge-triggered flip-flops; transparent-high/low D latch;
  transparent-high/low gated SR latch. SR inputs are active-high S/R and11
  produces X while enabled/on the selected edge. An ungated latch can use the
  corresponding constant enable reference.
- Gates: NOT,AND,OR,XOR,NAND; one/two inputs. Forward references among gates
  are resolved topologically. Gate-only self/mutual cycles are rejected; state
  feedback is allowed and checked by bounded settling.
- Up to64 atomic external events, each a4-bit update mask and4-bit value word;
  unmasked inputs retain their previous value. Column0 is settled initialization,
  columns1..event_count follow events0..event_count-1. `completed` counts successful
  columns including column0, so a completed64-event run has65 columns.
- Values0/1/X=2. Explicit initial Q may be X. Disabled outputs are stored as X; the native UI shows disabled rows as off/dashes, distinct from a live unknown X;
  active circuitry referencing a disabled Q/G source is rejected as unavailable.

## API and invalidation

`tm_init` defines one rising D cell, I1->D,I0->CLK,Q0initial0,reset disabled;
all other devices/gates disabled and no events. Native product examples are a
separate Main-owned module; this computational module does not embed examples.

Use `tm_set_cell`, `tm_set_gate`, `tm_set_initial_inputs`, `tm_set_event` and
`tm_set_event_count`, or mutate a bounded specification and call `tm_invalidate`.
Every setter invalidates derived data. Invalid setter arguments return false
without applying the rejected value. Cell/gate full specifications and their
source references are validated by `tm_run`; unavailable signal/type/edge/reset,
range/count and combinational-loop errors prevent a valid result.

`tm_run` computes all requested columns. Only `model.valid=true` authorizes result
views. Failure reports `error`, `error_column` and `error_cell`; previous array
contents are not old answers to reuse. `completed` may identify an earlier
successfully settled prefix, but a failed run is not a usable stable full trace.
Unknown/invalid cell values are legitimate explicit X values in an otherwise
valid abstract simulation; failure to settle is a different model failure.

`tm_phase_log` stores at most33 snapshots for one selected column. `tm_explain`
replays from initialization through that column without modifying source/result
storage and without copying the whole model onto the stack. Snapshot0 is after
that column's external input batch and combinational evaluation, before any new
state commit; later snapshots show simultaneous state commits followed by
combinational evaluation. The main table supplies the preceding settled column
when before-external-input values are needed. Reasons are per cell and snapshot.
A no-state-change event still has one snapshot and records its evaluated action.
The four aggregate column reasons retain an actual capture/toggle/reset cause;
subsequent no-edge holds do not erase it.

## Ideal event ordering and resets

This is an ideal zero-delay three-value event model, not propagation-delay,
setup/hold, metastability or analog simulation. One event is an atomic batch:

1. Apply all masked external input values simultaneously.
2. Evaluate the acyclic combinational network using current/old Q.
3. For every cell, calculate its candidate output from that same snapshot.
   Edge devices compare their previous clock level with the current level;
   latches use current enable polarity. No cell observes another cell's candidate.
4. Synchronous reset is sampled from that precommit snapshot only when the cell
   has an active edge, or while a latch is enabled. Asynchronous reset has
   priority independent of clock/enable and overrides normal/invalid SR results.
5. Commit all candidates simultaneously. Re-evaluate combinational signals and
   process newly generated clock edges/transparent paths in the next generation.
6. Finish only when a generation produces no Q changes. At most32 generations
   are attempted. Nonconverging known-state feedback fails explicitly.

Data and CLK changes placed in the same external batch are, by convention,
input updates before sampling; the simulator does not invent physical timing
margins. With a high-transparent master and low-transparent slave, falling CLK
closes the master without sampling newly changed D, while the opening slave
sees the held master. Rising CLK opens the master after the slave closes. A
clocked device connected to another simultaneously updating cell samples its
old Q; a genuinely derived Q clock causes a later generation instead.

Initialization settles transparent latches and asynchronous resets with edge
captures suppressed: initial Q is an explicit starting condition, not an invented
t0 clock edge. Initial latch outputs may therefore differ from configured Q
when their enable is open; async reset may likewise override initial state.

Review Timing5 uses active-low **synchronous** reset from NAND(Q0,Q2). If that
NAND falls after a counter update, reset is not applied until the next clock.
The aggregate cause of that reset remains 'Sync reset' after the NAND returns
high in a subsequent settling generation. A symbol bubble alone never changes
explicitly configured reset synchrony.

## Unknown/invalid behaviour

Gates implement three-value logic:0 AND X=0,1 OR X=1,NOT X=X; XOR with X=X.
Cell active operations enumerate concrete interpretations and merge outcomes;
known results may be recovered from X when all interpretations agree (e.g D
captures1, SRset,JKreset). SR11 is X; a later definite set/reset can recover.
This differs intentionally from M4's binary-only invalid-state lock.

Only an observed abstract clock-level change is an event. Stable X->X is not an
edge, including no-op input events or unrelated data changes. For an observed
known/X transition whose interpretations may include the chosen edge, merge
possible hold/capture and label 'Possible edge'; do not fabricate a definite
edge. Ambiguous latch enable similarly merges hold/active behaviour. Unknown
reset activation merges reset and normal behaviour and reports uncertainty,
not a definite reset even if both outcomes happen to have the same binary value.
A definite active synchronous-reset level also does not establish that the
activating edge/enable occurred: possible-only FF edges keep 'Possible edge',
and uncertain latch enables keep an uncertainty reason even when reset and hold
agree. Definite asynchronous reset still overrides clock/enable uncertainty.

The abstraction cannot detect hidden physical transitions within a steady X.
Likewise, X can form an abstract fixed point in a feedback loop; successful
abstract settling is not proof of physical stability. Known-state oscillating
latch feedback is diagnosed by the32-generation bound. All these assumptions
belong in user-visible model help and the acceptance guide.

## Actual isolated checks

Strict Apple Clang C11 `-Wall -Wextra -Werror -pedantic -O1 -g` with ASan/UBSan
passed **15,247 assertions**, failure0. Shared entry:
`int tm_run_checks(unsigned *checks)` resets its count and returns0 or the failed
source line. Isolated runner/logs are `.phase1/m6-engine/{main.c,tests,results.log}`.

Independent test references include literal device tables (not production
ff_evaluate), concrete expansion of three-value gates/data, hand-derived state
vectors and fixed Review1-5 sequences. Coverage:

- All six cell types, binary Q/a/b/clock-before/clock-after and edge/enable
  polarities; synchronous/asynchronous reset polarity/value/edge combinations.
- All3-value gate inputs and active device data states; all observed abstract
  clock transitions for every FF type,Q state and both edge choices.
- Possible-only rising/falling activations with constant active synchronous reset,
  both current/reset bits equal and opposite; every phase avoids falsely claiming
  a definite reset. Equivalent gated D/SR latch uncertainty and definite async
  override are checked separately.
- No-op masks/unchanged clock, unknown-clock hold/possible-edge, unknown-reset
  explanations, recovery of X by definite D capture and SR latch set/reset.
- Master/slave internal transparency and atomic falling-clock/D change;
  mixed latch/rising/falling comparison; three-stage Q-clock ripple and its
  intermediate000->001->011->111 snapshots; four simultaneous T stages;
  synchronous NAND feedback reset producing1,2,3,4,5,0,1,2.
- Independent D-swap test, initialization/async reset priority, phase log replay
  equality with final table columns,0/64-event boundaries and invalid indices.
- Forward acyclic gates, unavailable sources, bad enums/ranges, gate-only cycles,
  one/two-latch oscillators, explicit nonconvergence and null/label guards.

This is exhaustive over specified local domains, not all possible four-cell
networks or64-event streams. Main-owned actual product presets have separate
integration fixtures, and Main owns target compilation/execution, native UI,
production provenance and hardware handoff. No target/physical pass is claimed
by this isolated subtask.

Host Clang `-Os -fstack-usage` reports simulate192,settle304,signals64-byte frames,
excluding caller/library overhead. There is no new heap or recursive call chain.
These are host compiler observations, not SuperH or physical stack measurements.

## Main integration verification

Main independently reran15,247core checks and381product-preset golden assertions;
all eight shared suites ran as SuperH and matched host reports. Product models
are independently checked against visually transcribed Review1-5 vectors and
intermediate phases, not just the engine agent's own setup fixtures. Native
93screens/19invariants and all older pixel/function regressions pass. TargetGCC
frames simulate136,settle164,signals48,tm_run12,tm_explain8,timingUI192 bytes;
these exclude caller/helpers/interrupts and are not physical peaks. A separate
65column benchmark has a292,929 whole-program emulator-instruction observation,
including replay/checks/I/O; no physical/worst-case claim. See rootTESTING for
artifact/provenance/coverage details. M5/M6 physical acceptance is user-deferred.
