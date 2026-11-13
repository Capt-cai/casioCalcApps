# M6 - Timing Diagram & Sequential Simulation

Approved by “开始M6吧 实机验收后面一起做”. M5 engineering is complete, but its
hardware acceptance is explicitly deferred, NOT passed. This authorizes M6 only;
M7 remains unapproved. M5 source/binary/evidence/helper snapshots are preserved
under `.phase1/milestones/M5-engineering/`; M4 is still the accepted checkpoint.

## Approved implementation outcome

A structured offline event simulator, independent of gint, plus native device/
gate/event editors, waves, signal tables and selected-event explanation. Reuse
accepted flip-flop characteristic rules without changing M3 or earlier modules.
No global framework/Boolean-parser rewrite. One independent core/test subagent
owns timing.[ch],timing_checks.[ch],M6-MODEL; Main owns product presets, independent
preset integration checks, native UI, builds/emulator/records and final review.

Bounds:4external inputs I0-I3,4state cells Q0-Q3,4combinational nodes G0-G3,
64external event batches plus initial column,32settling phases per batch. Constants
0/1 have explicit source IDs. Signal references and gate/device properties are
editable, not presets-only. Support edge D/T/JK/SR, transparent D/SR latches,
rising/falling edges, enable polarity, synchronous/asynchronous reset with
polarity/value, and binary/unknown initial Q. Master/slave uses two actual
opposite-enable D latches, with both internal states visible.

Each external event specifies a4bit change mask and binary values; unmasked
inputs hold. Atomic batch rule: apply the batch's external inputs, settle
combinational values with old Q, then sample/commit all triggered edge cells
simultaneously. Data arriving in the same batch as a clock edge is explicitly
sampled AFTER that external input update; a separate preceding event represents
setup time. This is a declared ideal ordering, not an analog race measurement.

Initialization settles latches and asynchronous reset without inventing a clock
edge. A closing latch holds its pre-event state; an opening/transparent latch
follows the new data after simultaneous commits settle. Derived clock changes
schedule further generations at the same logical event. Synchronous reset samples
pre-commit feedback on the active edge; asynchronous reset has priority in its
phase and may react between clock edges. Unknown values use an explicit X;
clock uncertainty never creates a definite edge. Stable X->X is not an observed
edge. Invalid SR may propagate X and subsequently recover through definite
set/reset/capture; this timing model does not silently reuse M4's binary-state
invalid lock. Gate-only cycles are rejected, device feedback has bounded settling
and explicit nonconvergence errors. No real propagation delay/metastability claim.

## Required results and small screen

Frozen signal-name column/time ruler;4signals x8event slots with high/low/unknown
rails and cursor. UP/DOWN selects signal (pages of4),LEFT/RIGHT selects time
(pages of8), page counts/continuation marks remain visible. Exact numeric signal
table complements the waves. Selected-event view shows input changes, before/
after state, each device's clock/latch/reset reason and phase count. On-demand
bounded phase replay gives intermediate signal values (especially ripple and
feedback reset) without keeping64x32full snapshots permanently. Editing any
specification/event invalidates old results; navigation/help retains problem.

## Engineering tasks and acceptance gates

| Task | Deliverable | Required evidence |
| --- | --- | --- |
| M6.1 |Portable three-value devices/gates/event engine|Independent device/reset/edge truth/event oracles;validation,unknowns,no-op and bounded-feedback tests |
| M6.2 |Actual product Review1-5 and additional presets|Independent literal golden vectors using visually checked PDF polarity/bit order; no just screenshot-presence claim |
| M6.3 |Editable cells,gates,initial states and64event timeline|All source IDs/settings accessible; clear bounds/errors and stale-result invalidation |
| M6.4 |Wave,table,event/phase explanation UI|Native rendered layout/cursor/page tests; X clearly distinct from0/1; all intermediate sources viewable |
| M6.5 |Fresh production/shared-target builds and regressions|Pinned image/emulator,report matches host,source/hash/resource evidence; M2B/M3/M4/M5 functionality preserved |
| M6.6 |Records/coverage and joint Chinese handoff|M6 engineering passed vs M5/M6 hardware-deferred kept separate; stop for acceptance/M7 authorization |

Mandatory cases:

- Timing1 p21: transparent-high master/low slave; Qmaster follows highCLK while
  Qslave holds, then transfers at falling boundary; mid-level D changes matter.
- Timing2 p22: high D latch,rising D,falling D; first rise samples1,second rise0;
  first fall samples0,second fall1. Include complements as intermediate gates.
- Timing3 p23: T0 clockI0,T1 clockQ0,T2 clockQ1,T=1; initial000; after8rises
  conventionalQ2Q1Q0 is7,6,5,4,3,2,1,0. Source labels Q0-first:111,011,101,001,
  110,010,100,000. Selected-event phases must expose cascaded internal edges.
- Timing4 p24-25:4common-clock T cells,initialQ0Q1Q2Q3=1110 (numeric7);
  T0=1,T1=Q0,T2=Q0Q1,T3=Q0Q1Q2; numeric8..15,0..7. No serial-update contamination.
- Timing5 p26-27:3common-clock T cells,initial000,T0=1,T1=Q0,T2=Q0Q1;
  feedback reset NAND(Q0,Q2),active-low SYNCHRONOUS per slide's explicit soft-reset
  wording. Active-edge states1,2,3,4,5,0,1,2. Sixth rise explains pre-edge reset0
  even though settled reset returns1 after clearing. Do not infer async from bubble.
- Reset comparison: reset asserted between edges leaves sync Q unchanged but
  clears async Q; active-edge reset dominates data; deassertion alone isn't capture.
- SR invalid/unknown, known resolution, initialX and unknown-clock uncertainty;
  no-op batches/unchanged values cannot fabricate edges.
- D/SR latch enable polarities and ungated reference constants; impossible
  transparent feedback fails within32phases, no old successful wave survives.
- Maximum64events/65columns/12signals and all four cells; event65 rejected clearly;
  signal/time/table/phase pages all reachable and return/cancel preserve state.

Host and SuperH mathematical checks must be distinguished from UI pixel relations
and later physical results. Re-run relevant accepted/model/target/UI regressions;
keep generated artifacts isolated. Measure compiler/static-resource observations
and representative bounded target execution where feasible, without claiming
physical performance. After software completion provide reusable seven-part
Chinese M6 guide plus deferred M5 checklist; user will perform joint hardware
acceptance later. No M7 implementation is authorized by this task.

## Engineering completion

All six task groups passed. Shared host/target core15,247 and actual product
preset381 assertions pass;93production timing screenshots/19invariants and all
prior-tool regressions pass. Independent model fixture and product-preset goldens
cover Review1-5 plus boundary/unknown/reset/nonconvergence cases. Main reviewed
the possible-edge reset explanation, added132 assertions through the engine
owner, and reran target execution after the fix. Final packaging/provenance,
artifact identity,resource estimates and coverage limits are in rootTESTING.

M5/M6 hardware acceptance remains explicitly deferred, not passed. The
[Chinese joint-use guide](M6-USER-ACCEPTANCE.zh-CN.md) is delivered for later testing.
M7 is unapproved; no further milestone implementation starts automatically.
