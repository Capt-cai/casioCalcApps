# M7 bounded combinational model and verification contract

M7 explicitly approved November12,2026. No earlier physical acceptance inferred.

## Local composition

New portable `combinational.[ch]` separates computational specifications from
native `combinational_ui.[ch]`. It calls unchanged exact_boolean/solver interfaces.
No sequential/global representation refactor, no M3/core changes, no M8 Verilog.
A retained selected-output model contains problem parameters, truth/DC data,
SOP/POS results, up to8 MUX cofactors and a deduplicated acyclic gate graph.
One selected output is solved at a time to bound resources; family parameters
persist across output changes. Invalid edits discard derived graph/verification.

Domain:2..4 variables for general truth/expression/exact minimization; one generic
output,2adder/count outputs,7BCD outputs,2/3encoder code outputs and separate VALID.
Generic expressions reuse the existing79-character A-D parser. Encoder family
inputs are4or8 and are never routed through an eight-variable Boolean minimizer.
Ripple chains allow1..8bit unsigned inputs/Cin0or1. No heap or new recursion.

## Connections and execution

512 bounded nodes: constants,input,NOT,AND,OR,XOR; binary gates reference only earlier
nodes, identical gates share references. All actual output and data pins identify
node IDs. The node list may include reusable/unused alternative derivation nodes;
no minimum physical gate-count promise. Gate equations remain exact minimum
cube/literal SOP/POS using the existing core. Adder families also emit the actual
XOR two-half-adder structural output (Sum=P XOR Cin, Carry=AB+P*Cin).

MUX2/4/8:1 has1..3 ordered distinct select variables, all within active inputs.
First displayed slot is highest select bit; Dk uses its binary address. Freeze
the selected Boolean form's explicit DC completion, substitute each selector
assignment, minimize each cofactor, and build that pin's gate network. Thus all
MUX/gate values agree even on chosen DC completions. Display cofactors and their
node connections; no silent unsynthesized arbitrary data function remains.
Runtime replay evaluates actual emitted nodes, including selected MUX data paths,
and compares every cared output to its problem table. Host fixtures additionally
interpret the graph independently and check displayed cofactor equations.

P4.12 selects x0,y0. Sum D0..3=c0,!c0,!c0,c0; Carry=0,c0,c0,1. Formula labels explicitly
map A=x0,B=y0,C=c0. Half/full-adder/popcount truth uses input count and output bits;
Count1,Count0 correspond to FA Carry,Sum. Ripple stages expose each carry wire,
input values and assembled low-width result/Cout; runtime verifies wider integer
sum equals assembled output. No measured propagation delay.

Ordinary encoder code pins OR binary indices of all asserted Ii. VALID is
(any input) AND NOT(any pair asserted). Zero/multiple inputs are invalid; actual
code pins are deterministic and disclosed but cannot be interpreted as priority.
All16/256 rows remain inspectable with ZERO/MULTI/one-hot diagnostic.

BCD decimal glyphs use a=top,b=UR,c=LR,d=bottom,e=LL,f=UL,g=middle; standard decimal
0..9, logical segment bits inverted for active-low pins. Invalid10..15 policy:
zero physical pins, blank (all polarity-off pins), or explicit don't-care.
Active-low zero physical pins illuminate segments, so blank is a distinct choice.
All7 output functions and selected segment gate/MUX paths use the same settings.

## UI and exceptional cases

Launcher6/F6. OPTN1..7 replaces the problem; OPTN9 help,other keys cancel. Six
function keys expose SET,TABLE,EQ,KMAP,CIRCUIT,RUN. EXE in CIRCUIT cycles wires,
MUX-data,verification. Settings/output/source/form edits invalidate results;
RUN is explicit. Manual truth cells cycle0->1->X->0. Expression editor is bounded,
append-only with DEL/AC,EXE applies,EXIT cancels. Number editors reject out-of-range
or excess digits without applying. An invalid expression cannot display a stale
compiled truth table. Invalid/dirty equations/maps/connections are unavailable.

Tables page6rows; encoder256rows wrap and page with arrows. Ripple pages5stages.
Connection pages7lines; formulas wrap46characters and page; all source characters
remain available. K-map cells show exact binary row ID in Gray order and highlight
the selected ON cube (SOP) or OFF cube (POS); UP/DOWN selects groups. MUX pages
show one complete cofactor (at most3 residual variables). Input/order/pin aliases,
output/form,invalid policies and actual DC completions remain visible. Help/menu/
reentry retain state; closing the application is not persistent file storage.

## Verification layers

Shared target/host assertions use literal P4.12 cofactor expectations, independent
decimal glyph strings, all encoder patterns and separate graph interpreter.
Host extension exhausts every2/3-input Boolean function with all legal ordered
select configurations/MUX sizes and both forms; deterministic4-variable/DC
fixtures and an all-DC model cover bounded integration. It also exhausts every
operand/Cin combination for each1..8bit ripple width with per-stage checks.
Existing core's separate exhaustive suite covers all4-variable fully specified
functions and its independent exact-cost oracle; M7 does not claim exhaustive
4-variable gate/MUX configuration testing. Native UI scripts are complementary
interaction/pixel evidence, not mathematical proof or physical-hardware tests.
