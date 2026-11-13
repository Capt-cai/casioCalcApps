# M5 portable connection descriptions

`apps/ecen21/src/seq_connections.[ch]` compiles a solved M5 model into a bounded,
shared declarative gate netlist and a paginated native-text description. It has
no gint dependency, heap allocation or new recursive functions. It reuses the
accepted exact engine when optional output equations require minimization.
Upstream/common/M3 and other agents' owned files are unchanged.

## API and ownership

- `cn_build(model, form, &net)` builds all required flip-flop pin functions and
  specified Z0/Z1 outputs from the selected SOP/POS completion. The model must
  have valid solved data. On failure `net.valid=false`; no old valid net remains.
- `cn_line_count(&net)` returns the complete description's line count, or zero
  for an unavailable network.
- `cn_line(&net, index, text, capacity)` returns one complete line, at most46
  visible characters. Invalid index/null output or insufficient capacity returns
  false, with an empty NUL-terminated output when capacity permits. It never
  returns a successful silently truncated line. `CN_LINE_CAPACITY=47` is enough.

The netlist is a standalone snapshot of clock/initial-state/policy/FF metadata,
variable labels, gate nodes and pin/output drives. The UI must rebuild it after
solving or selecting another form and must not display it while its problem
model is invalid. Keep it in static caller storage rather than a deep UI stack.
Do not alias the output netlist and input model storage.

The fixed bound is224 nodes, each with at most4 inputs. On the tested Apple
Silicon C layout a node is16 bytes and the full netlist3,712 bytes. These are host
layout observations, not measured fx-CG50 peak memory. Build results and target
compiler/resource observations belong to Main Agent integration records.

## Concrete implementation meaning

Nodes0/1 are constants, followed by present-state/control feedback sources;
remaining nodes are NOT/AND/OR/XOR gates referring only to earlier nodes. Source
labels are Q2/Q1/Q0/E as appropriate, not unexplained Boolean-letter placeholders.
For example `N5=XOR(N2,N3,N4)` names actual gate inputs; `FF0.T <- N5` names an
actual flip-flop input connection. The description also gives each FF type/output,
common clock edge, required initial Q and unused-state guarantee. Initialization
is a requirement before clocking, not an invented reset pin.

For SOP, each cube becomes an AND of fixed literals and an OR combines the
products. POS complements each zero-cover cube into an OR clause and combines
clauses with AND. Empty groups/covers produce their correct Boolean constants.
Large sums/products fold into bounded-fan-in gates without dropping any term.
Identical nodes and inversions are reused. The result is faithful wiring, not a
claim of globally minimal gate area or arbitrary technology mapping.

Before emitting sum/product gates, the builder checks every row of the actual
selected completed function for XOR/XNOR parity. Matching variable sets reuse
one parity node globally; complements reuse its NOT. It does not optimize from
cared rows alone: DC completions can differ between SOP and POS and those
specific completed behaviours must be preserved. Review Counter2 therefore
uses exactly one3-input XOR for Q1/Q0/E and one NOT shared across the two T pins.
Both supported forms are tested for that restricted connection requirement.

Optional present-state outputs are fully specified masks; the existing exact
engine minimizes each with no DC and the same graph/presentation machinery
implements it. Every required pin and each specified output has an explicit
final drive. Unspecified outputs have no fabricated constant-zero drive.

## Actual isolated verification

Strict Apple Clang C11 build with `-Wall -Wextra -Werror -pedantic -O1 -g`, ASan
and UBSan passed **128,986 assertions**. Evidence/temporary driver:
`.phase1/m5-connections/results.log`, `main.c`, `tests` (ignored).

`cn_run_checks(unsigned *checks)` resets its count and returns zero or a failing
line, for Main's shared host/SuperH integration. The fixtures execute the actual
emitted `Nn=AND/OR/XOR/NOT(...)` gate-language lines with an independent parser
and interpreter, not just a copy of the production formatter. Feedback Q labels
are interpreted by real state-bit positions; E is decoded independently from
row ordering. Every emitted pin/output drive is replayed against selected-form
core truth values across every row. Representative output equations are also
checked independently from the four raw variable bits.

Coverage includes the T counter fixture; complete mixed D/T/JK example; every
FF type under RETURN/HOLD/DC; controlled two-bit parity and exact gate counts;
all16 two-variable functions with complementary D outputs;12 deterministic
mixed-type four-variable maps with two output functions; both forms; constants,
negative literals and legal excitation/DC completions; line-length/capacity and
null/index/invalid-state guards. This is not exhaustive enumeration of all
four-variable circuits or every possible UI input.

An additional strict host `-Os -fstack-usage` compile reports cn_build144,
function160, combine112 and cn_line192-byte frames (caller/library overhead not
included). This module does not claim target compilation, target execution,
physical latency/stack, or user acceptance; Main Agent independently reviews,
builds and runs the actual calculator integration.

## Main integration evidence

Main independently reran the strict host suites and the same shared assertions
as actual SuperH code. The target's six-line passing report exactly matches host.
Production100 M5 screenshots/20 invariants and accepted-tool pixel regressions
pass. Final source/artifact/resource/coverage limits and representative maximum-
domain measurement are recorded in rootTESTING. No physical M5 execution is
claimed; M5 user acceptance is pending and M6 remains unapproved.
