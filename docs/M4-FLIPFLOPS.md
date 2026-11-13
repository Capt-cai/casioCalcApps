# M4 portable flip-flop model

## Domain and API

`apps/ecen21/src/flipflop.[ch]` is portable C11, independent of gint/fxSDK,
with no heap allocation, recursion, file I/O or clock timing dependencies.
The model supports one synchronous, binary-state SR/D/JK/T flip-flop at a time.
SR inputs are active-high S/R; it is not an asynchronous SR latch.

`ff_evaluate` is a pure transition query. A result contains previous Q, inputs
`a/b`, selected active edge, event, next Q, `next_valid` and an explanation status.
Both inputs must be binary even when b is unused (D/T). Only the selected active
edge samples inputs; NONE or the other edge yields HOLD. D always yields CAPTURE
on an active edge, even if the sampled value equals previous Q. SET/RESET likewise
describe the operation rather than requiring a value change.

| Type | a / b | Active-edge function |
| --- | --- | --- |
| SR | S / R | 00 hold; 01 reset; 10 set; 11 invalid |
| D | D / unused | Capture D |
| JK | J / K | 00 hold; 01 reset; 10 set; 11 toggle |
| T | T / unused | 0 hold; 1 toggle |

For SR11 at an active edge, `next_valid=false`; `next_q` is an unusable placeholder,
not a predicted binary next state. Inactive-edge SR11 holds under this synchronous
functional model. No electrical-delay/metastability behaviour is modelled.

`ff_characteristic_count` and `ff_characteristic` enumerate all characteristic
rows: Q-major, then a, then b. SR/JK have eight rows; D/T have four, with b=0.
Characteristic rows assume an active rising event; both edges have the same
logical table. Invalid row/type queries return false without altering output.

`ff_get_excitation(type, Q, desired_Q, out)` returns inputs in {0,1,FF_X}.
FF_X=2 means either binary input satisfies the requested transition; it does not
mean an unknown simulated Q. D/T b=0 is an unused placeholder and should not be
shown as another required input. Each returned cube describes all and only the
valid input assignments for that binary transition.

## Stateful simulation

`ff_init` starts D, Q=0, inputs0, rising active edge and rising event.
`ff_state` is public for bounded UI rendering. Mutate settings through setters;
queries and stepping reject invalid enum/bit arguments. API functions handle
NULL pointers without dereferencing them.

- `ff_step` records at most 16 transitions. Every accepted event counts as one
  record, including HOLD and the first invalid SR event. It returns true for
  valid transitions and false for invalid SR or rejected actions.
- Invalid SR sets `q_valid=false` and blocks subsequent steps. The `q` field
  retains the last-valid value only for recovery context; it must not be shown
  as the actual current Q while `q_valid=false`. The invalid history row has no
  usable next Q. Always guard both current/next displays by their validity flags.
- Changing inputs/edge/event does not clear history or unlock invalid Q.
  Same-type selection is a strict no-op. Setting errors may be cleared by a
  successful setter, but `q_valid` remains false: UI invalid-state indication
  must depend on `q_valid`, not only error text.
- `ff_reinitialize(state, initial_Q)` sets both initial/current Q, clears history
  and invalid lock, and retains type, inputs, active edge and event. This is a
  simulation restart, not an asynchronous reset pin.
- Explicitly changing type starts a fresh simulation at initial Q, clears
  history/lock, resets inputs0 and event RISING, and retains the active edge.
- Step17 is rejected without overwriting any record. Reinitialization (or an
  explicit type change) starts a new history. Changing other settings cannot
  bypass the limit. History snapshots include their original event and active
  edge, so changing settings later cannot reinterpret prior rows.

`last_status` describes the most recent step/rejected action; `error[64]` gives
bounded fixed text. The UI owns presentation, selected view and pagination.

## Independent shared verification

`apps/ecen21/tests/flipflop_checks.[ch]` exposes
`int ff_run_checks(unsigned *checks)`, returning zero or the failing source line.
It uses literal truth tables independent of production branch logic and expands
every excitation cube against independent valid assignments, checking soundness
and completeness rather than only one representative input.

Agent-local strict host execution passed **3,849 assertions** with Clang C11,
`-Wall -Wextra -Werror -pedantic -O1 -g -fsanitize=address,undefined`.
Evidence and temporary runner: `.phase1/m4-model/` (ignored). Coverage:

- All 192 combinations of four types, binary Q/a/b, both active edges and three
  events, including ignored D/T b values; pure queries and stateful execution.
- All 24 characteristic rows, including SR forbidden rows.
- All 16 type/Q/desired-Q excitation queries; every binary assignment is checked
  for equivalence with the returned input cube, including each X expansion.
- D capture/hold, repeated JK toggle, T1/0/1 sequence, SR invalid lock,
  settings/no-op bypass attempts, restart and explicit type-change recovery.
- Exactly16 records, rejected step17, immutable snapshots including edge/event,
  recovery after full history, argument/null/enum/bounds errors and UI labels.

These are host-only results at the model handoff. Main Agent owns independent
review, shared SuperH execution, production UI/emulator checks and final build
provenance. No physical calculator execution or resource/latency measurement is
claimed here.

## Main integration verification

Main Agent independently reviewed the oracle and reran the sanitized host suite.
The same3,849 flip-flop assertions passed as actual SuperH code alongside accepted
Boolean/numeric/legacy fixtures; target and host reports match exactly. Production
validation passed54 flip-flop screenshots/22 interaction invariants, with unchanged
accepted Boolean/numeric tool pixels. Detailed evidence/limits are in `TESTING.md`.
GCC reports ff_step68, ff_evaluate8, ff_characteristic28 and flipflop_run120-byte
frames; the full ELF has text64,664/data464/BSS2,672. These compiler observations
are not physical stack/latency measurements. M4 user acceptance remains pending.
