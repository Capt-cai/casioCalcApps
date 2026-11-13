# ECEN21 Exam Review problem coverage matrix

Assessment point: M4 accepted;M5/M6 hardware explicitly deferred;M7/M8 and custom-width follow-up engineering verified,user acceptance pending. A roadmap target is not an
implemented feature. Current app/code/tests and the supplied41-page review were
inspected; M5/M6 shared/host/target/production checks are recorded in root TESTING; M7 combinational engineering checks are now recorded below; M8 HDL engineering checks are recorded below;future M9 cases remain unexecuted.

Source: `/Users/maggie/Downloads/ECEN21_Exam#2_Reviews.pdf`, titled Review for
Exam #2, dated November10,2026; SHA-256
`ce9aab1958feb3b7736ce5eabbd754efb93ce4929b031d4f2672689cd058a8a3`.
Page numbers below are one-based PDF pages and match slide footer numbers.
Text was extracted from all41 pages; relevant circuit/waveform/code pages were
also rendered and visually inspected. PDF instructions are problem content,
not authorization to implement or to override the user's scope boundaries.

## Status rules

- **Full Coverage**: the exact bounded workflow and required outputs are
  implemented, tested and accepted. It is not a claim about unlimited inputs.
- **Partial Coverage**: implemented components solve part of the problem, but
  required inputs, intermediate outputs or end-to-end steps remain missing, or required milestone user acceptance is still pending.
- **Not Supported**: no direct supported workflow for the task, with no committed
  target in M5-M9 (Future Backlog is not a commitment).
- **Planned**: no complete current solver; a specified M5-M9 target exists but
  implementation is unapproved/unstarted. Related partial reuse is called out
  without promoting the problem to Full Coverage.

M5 implementation is complete in its bounded2-3bit/0-1E domain; M6 bounded timing/event simulation is engineering-verified. Both hardware acceptances are user-deferred, so their rows are not promoted to Full Coverage.

Acceptance column: **Existing** identifies previously passed baseline evidence;
**Planned** identifies a future test, not a result already obtained. M5 counter and M6 Review1-5 fixtures were actually checked; other exact exam fixtures were not all rerun. 'Target' identifies
where missing work would belong, not authorization.

## Accepted reusable domains (not whole exam problems)

| ID / Source Problem or Topic | Required Inputs | Expected Outputs | Required Solver | Target Milestone | Current Coverage Status | Acceptance Test | Known Limitations |
| --- | --- | --- | --- | --- | --- | --- | --- |
| BASE-B; accepted Boolean domain |2-4 variable expression or0/1/X truth rows|Truth table,K-map,exactSOP/POS,groups|M2B Boolean|M2B accepted|Full Coverage|Existing6,516 shared checks and153,092 exact-core regression cases|79-char editor; not a multi-output circuit-design workflow |
| BASE-N; accepted numeric domain |8/16/32-bit A/B,base,sign,ADD/SUB|Conversions,retained result,carry/borrow/overflow|M3 numeric|M3 accepted/frozen|Full Coverage|Existing20,779 shared checks and2,359,296 host assertions|No custom widths, per-bit carry-chain derivation or extra encodings |
| BASE-F; single synchronous flip-flop |SR/D/JK/T,Q,inputs,active edge/event|Characteristic/excitation tables,nextQ,16-step log|M4 flip-flop|M4 accepted/closed|Full Coverage|Existing3,849 shared checks;54 screenshots/22 invariants; user reports realfx-CG50 acceptance|Single binary-state device; no latch,connected clocks,reset pin or waves |

## Number systems and combinational problems

| ID / Source Problem or Topic | Required Inputs | Expected Outputs | Required Solver | Target Milestone | Current Coverage Status | Acceptance Test | Known Limitations |
| --- | --- | --- | --- | --- | --- | --- | --- |
| NUM-1; p5 two's-complement,4-bit examples |Width4,values and conversion direction|Exact4-bit representation plus invert/add-one steps|Numeric representation|Future Backlog only|Partial Coverage|Planned width4 -5=1011 and reverse; existingM3 verifies analogous8-bit case|M3 cannot select4bits or show every derivation; no M3 changes planned |
| NUM-2; p6 Example10.3,p9-10 overflow |5-bit arithmetic;4-bit carry into/out of sign bit|Exact-width result,overflow and intermediate carries|Numeric/carry analysis|Future Backlog; M7 carry structure only|Partial Coverage|Planned5-bit -8+-9 ->01111 overflow; per-bit carry references|Current8bit result/flag is not equivalent to5bit overflow; don't mark Full |
| NUM-3; p7 P3.3 |10-bit raw strings0111011110,1011100111,1111111110|478,-281,-2 at width10|Numeric representation|Future Backlog only|Partial Coverage|Planned direct width10 conversion fixture|Manual sign extension is a workaround, not direct problem coverage |
| NUM-4; p8 P3.4 two's-complement part |73,1906,-95,-63 at width12|Exact12-bit strings|Numeric representation|Future Backlog only|Partial Coverage|Planned four width12 conversions against independently derived values|M3 only8/16/32; slide's last answer row says -1630 while prompt says -63; must not copy it blindly |
| NUM-5; p8 P3.4 sign-magnitude/ones-complement |Width12,encoding choice,signed decimal|Both alternative representations and steps|Representation solver|Future Backlog only|Not Supported|Planned positive/negative/boundary/negative-zero rules if separately approved|No such encoding inM3; excluded fromM5-M9 |
| NUM-6; p11-13 P3.5 and add-only subtraction |8-bit operands,operation;only addition allowed|Result/overflow plus X+!Y+1 conversion and steps|M3 arithmetic plus derivation workflow|M3 frozen; derivations Future Backlog|Partial Coverage|ExistingM3 domain arithmetic tested; planned six review fixtures including117-(-42) overflow and add-only derivation|Answers can be computed, but directSUB does not satisfy the complete add-only explanation task |
| NUM-7; p2 Module11 comparison topic |Signed/unsigned operands and relation|Comparison result and interpretation|Comparison workflow|Future Backlog only|Not Supported|Planned boundary signed/unsigned comparison cases|Topic listed but no detailed comparison question in this PDF; no comparison UI |
| COMB-1; p3 full-adder design |x,y,cin;structured FA selection|8-row table,sum/carry,K-maps,equations,connection list|Combinational design + Boolean|M7|Partial Coverage|Planned all8 rows;sum=x XOR y XOR cin;carry=xy+x*cin+y*cin;netlist replay|M2B can solve each supplied output table, not build the complete FA design from type |
| COMB-2; p4 ripple-carry structure |Width,operands,cin,stage ordering|Per-stage sum/carry and connectedFA chain|Adder structure solver|M7|Planned|Planned verify each carry link and result for exhaustive small widths|M3 returns arithmetic, not structure; no propagation-delay model promised |
| COMB-3; p14-15 P3.21 three-bit popcount |Three binary inputs|2-bit count,table,equations,FA realization|FA/popcount design|M7|Partial Coverage|Planned all8 inputs;count=number ofones;FA cout,sum form output|Standalone Boolean table entry possible; no current structured popcount design |
| COMB-4; user-required half adder |x,y;HA selection|Table,sum,carry,equations,connections|Combinational design|M7|Partial Coverage|Planned all4 inputs;sum=x XOR y,carry=xy|Related Boolean engine only; not a separate Review slide |
| COMB-5; p16-17 P4.12 |Stage0 sum/carry functions,selectors x0,y0|Two4:1 MUX data assignments and connection verification|MUX synthesis + Boolean|M7|Planned|Planned sum[c0,!c0,!c0,c0],carry[0,c0,c0,1];all8 inputs|Simplified equations alone do not cover required MUX implementation |
| COMB-6; p18 seven-segment a,b,c viaMUX |Digit coding,segment order/polarity,invalid-code policy|Truth rows,segment equations,MUX connections|Display/MUX design|M7|Planned|Planned BCD0-9 for a,b,c and allseven segment outputs;invalid10-15 policy|Must not assume common-anode/cathode or silent DC handling |
| COMB-7; p18-20 encoder circuit aspect |4-to2/8-to3,one-hot input convention|Code,validity,truth/equations,connections|Encoder design|M7|Planned|Planned every16/256 input pattern;one-hot codes;zero/multiple asserted diagnostic|8input truth domain exceedsM2B; structured family semantics required; priority not implicit |
| COMB-8; user generic truth/equation implementation |2-4 var table or equations,gate/MUX target,selectors|Equations plus actual gate/MUX connection list|Boolean + combinational mapping|M7|Partial Coverage|Existing truth/minimization;planned netlist replay on all cared rows|Existing grouping explanation is not a complete gate/MUX implementation |

## Sequential design and timing problems

| ID / Source Problem or Topic | Required Inputs | Expected Outputs | Required Solver | Target Milestone | Current Coverage Status | Acceptance Test | Known Limitations |
| --- | --- | --- | --- | --- | --- | --- | --- |
| SEQ-1; p28-29 Counter1 |3bit allT;0,3,5,7,0;unusedRETURN0|State/excitation tables,pin equations/maps,sequence and connections|Counter design + FF + Boolean|M5|Partial Coverage|Engineering passed M5-A1;all8 rows and four-state cycle;unused1/2/4/6->0|Implemented automatic workflow;2-3bits/0-1E; M5 user acceptance pending |
| SEQ-2; p30-31 Counter2 |2bit allT;E0 cycle0,2,3,1;E1 cycle0,1,3,2|Controlled tables,equations,K-map,oneXOR3+oneNOT connections|Controlled counter design|M5|Partial Coverage|Engineering passed M5-C1;T0=E XOR Q1 XOR Q0,T1=!T0;all8 rows and changingE|Actual shared XOR3/NOT graph verified; E is not only hold-enable; M5 user acceptance pending |
| SEQ-3; p32 Counter3A.1 |3bit allJK;0,1,4,7,3,5,0|Complete design and unused-state behaviour|Counter design|M5|Partial Coverage|Engineering passed M5-A2;full cycle,unused2/6->0 underRETURN0|Automatic JK sequence synthesis verified; M5 user acceptance pending |
| SEQ-4; p32 Counter3A.2 |0,3,1,2,5,4,7,6,0;Q2D,Q1T,Q0JK|Mixed-type complete design and verification|Mixed counter design|M5|Partial Coverage|Engineering passed M5-B1;all8 next vectors and per-pin constraints|Synchronous oldQ sampling and complete mixed design verified; no ripple clocks; M5 acceptance pending |
| SEQ-5; p33 Counter3B |T1=Q0+!Q1,J0=!Q0*Q1,K0=!Q0+Q1,Z=Q0|Next/state/output tables and trajectories|Reverse sequential analysis|M5|Partial Coverage|Engineering passed M5-D1;00->10,01->11,10->11,11->00 with presentZ=Q0|Visually transcribed complemented equations verified; present-output table only; M5 acceptance pending |
| SEQ-6; user three-bit control/mixed boundary |3Q+E,typedFFs,branch cycles or equations|All16 rows,full excitation/equations/maps,trace/recovery|M5 composed solver|M5|Partial Coverage|Engineering passed M5-C2,E1-E3,U1,R1|All16 rows verified;max4 Boolean vars; higher bits unsupported; M5 acceptance pending |
| TIME-1; p21 Timing Diagram1 |D/CLK event list,master high/slave low,initialQs|Qmaster,Qslave,Qbar waves and explanation|Latch/master-slave event simulator|M6|Partial Coverage|Engineering passed ordered full pictured trace including D changes during highCLK|Two actual latches and phases implemented;ordered event grid,not printed x-spacing;hardware deferred |
| TIME-2; p22 Timing Diagram2 |SharedD/CLK,initial0;high latch,risingD,fallingD|Qa,Qb,Qc and complements,table,waves|Mixed latch/FF simulator|M6|Partial Coverage|Engineering passed full ordered-event trace;D transition during high changes only latch until edges|Shared latch/rise/fall timeline and complement gates implemented;hardware deferred |
| TIME-3; p23 Timing Diagram3 |ThreeT=1 stages;Q0/Q1 derived clocks,initial000|Ripple event phases,table,waves and displayed bit order|Connected derived-clock simulation|M6|Partial Coverage|Engineering passed p23 trace with intermediate propagation phases at idealzero delay|Derived clocks/phases implemented;sourceQ0-first labels preserved explicitly;hardware deferred |
| TIME-4; p24-25 Timing Diagram4 |Four synchronousT stages,initialQ0Q1Q2Q3=1110|Q andT1/T2/T3 waves;16-step cycle|Synchronous event simulator|M6|Partial Coverage|Engineering passed ordered1110,0001,1001,...,0110,1110 in source bit order|Four-cell simulation implemented;M5 synthesis remains3bit;noBoolean core expansion;hardware deferred |
| TIME-5; p26-27 Timing Diagram5 |ThreeT network,active-low synchronous feedback reset,initial000|Q,T,reset waves and pre-edge reset explanation|Sequential/reset simulator|M6|Partial Coverage|Engineering passed ordered trace;reset takes effect on next trigger;contrast async case|Synchronous active-low feedback implemented;phase explanation shows pre-edge sampling;hardware deferred |
| TIME-6; user reset/general latch cases |SR/D/JK/T,latch enable/reset polarity,async/sync reset,initialstate|Event-level table/waves and invalid/race diagnostics|M6 simulator|M6|Partial Coverage|Engineering passed sync reset waits;async acts between edges;latch vsFF;SRinvalid;nonconvergent feedback rejection|Bounded4cells/4gates/64events,three-value ideal model;no analog delays;hardware deferred |

## Verilog and comprehensive problems

| ID / Source Problem or Topic | Required Inputs | Expected Outputs | Required Solver | Target Milestone | Current Coverage Status | Acceptance Test | Known Limitations |
| --- | --- | --- | --- | --- | --- | --- | --- |
| HDL-1; p14 P3.21 Verilog extension |Width5 or other supportedn;structural/behavioral choice|Usable parameterized adder code,ports/wiring|Verilog generator + FA model|M8|Planned|Planned hostcompile/sim;all1bit FA cases,5bit boundary andcarry chain|P3.21 also contains popcount circuit task; those are separate coverage rows |
| HDL-2; p18-19 P4.20 |4-to2 encoder,one-hot/invalid policy|Usable encoder code|Verilog generator + encoder model|M8|Planned|Planned all16 patterns including zero/multiple;hostsyntax/behaviour|Circuit design inM7 alone does not supply Verilog |
| HDL-3; p18,20 P4.21 |8-to3 encoder,one-hot/invalid policy|Usable encoder code|Verilog generator + encoder model|M8|Planned|Planned all256 patterns,hostsyntax/behaviour;explicit invalid/X policy|No HDL tools orcodegenerator verified atM4 |
| HDL-4; user D/T/JK code generation |Type,edge,reset choice,initialization contract|Synthesizable sequential modules|Verilog generator +FF model|M8|Planned|Planned host event/hold/reset truth tests for eachtype|M4 functional solver is not emitted HDL |
| HDL-5; p34-35 P5.13 |n,parallel load,hold,left/right serial inputs,mode priority|Parameterized universal shift register code|Verilog generator|M8|Planned|Planned hosttests all modes/directions;n1,2,5,8;simultaneous mode priority|Bit ordering andserial insertion must be stated; nofixed8bit-only template |
| HDL-6; p34,36 P5.20 |N(default12),width,synchronous reset polarity|Modulo-N counter code|Verilog generator|M8|Planned|Planned12:11->0;reset between edges waits;N1/nonpoweroftwo/invalid parameters|ThreebitM5 limit does not preclude4bit modulus12 HDL; generator semantics separate |
| HDL-7; p34,37 P5.23 |n,reset,seed/direction convention|Parameterized one-hot ring code|Verilog generator|M8|Planned|Planned n1 andlarger,one-hot reset/rotation,n-step period;hostcompile|Review fixedslices despiteparameter must not be copied as universally parameterized code |
| INT-1; p38-41 P5.26 |Clock/Start trace,syncreset3bit counter,targetf/g timing|Derived state/output tables,equations,circuit,complete waves|M5+M6+M7 composed design|M9|Partial Coverage|Planned reset/start trace;g=!Q2+!Q1;f=!Q2*Q1*!Q0*!CLK;all half-cycles andinitialunknown assumptions|M2B can minimize g;if f omits clock complement it misses the pulse;not covered by state-onlyM5 outputtable |
| INT-2; user general counter+output design |State/control table,typedFFs,output requirements|End-to-end table -> minimization -> connections -> simulation|Integrated problem workflow|M9|Partial Coverage|Planned one sourceproblem replayed through allsolver views;independent row/trace/netlist agreement|Present tools require manual independent re-entry; no shared end-to-end design yet |

## Source cautions and promotion policy

p31 uses complemented parity for T1 and ordinary parity for T0; plain extracted
text makes them look identical. p33 has complements on Q1 in T1 and Q0 in J0/K0.
p40 gates f with !CLK, not CLK. p23/25 print state bits Q0-first, whereas counter
design normally uses Q2-first: explicit labels must prevent reversing sequences.
p8's solution table contains a -1630 row inconsistent with the -63 prompt.
These source quirks must become independently checked fixtures, not blind answer
transcriptions or a reason to alter M3 now.

Promote a row only after the exact required workflow, all listed outputs and
bounded cases have actual host/target/UI evidence and the relevant milestone's
user acceptance. Preserve source references, acceptance IDs and known limits.
No percentage is calculated by counting related tools as solved exam questions.

## Authorized custom-width follow-up — November 12, 2026

The user separately authorized1..32bit numeric widths. Original accepted-domain
BASE-N evidence remains historical; the new extension is engineering-verified,
user acceptance pending. M5/M6 physical verification remains deferred. No M7 work.

| Topic | Updated status | Actual evidence | Remaining limits |
| --- | --- | --- | --- |
| BASE-N extension1..32 | Partial Coverage |97,433shared host/target numeric checks;22native UI pages/6relationships;5,941,888host arithmetic checks |Extension hardware acceptance pending; original8/16/32 accepted coverage retained |
| NUM-1 width4 two's complement | Partial Coverage |Direct -5=1011 host/target fixture and1..32conversion checks |No dedicated invert/add-one derivation steps; hardware pending |
| NUM-2 width5 overflow | Partial Coverage |Direct -8+-9=01111 overflow host/target fixture;5bit15+15 UI result-2 |Per-bit internal carries still absent; M7 unapproved |
| NUM-3 width10 P3.3 | Partial Coverage |All three literal raw/decimal conversion pairs pass host/target |Direct UI mode available; exact fixture physical checks pending |
| NUM-4 width12 P3.4 two's complement | Partial Coverage |73,1906,-95,-63 literal bit patterns and reverse conversion pass host/target |No sign-magnitude/ones-complement; do not inherit slide's contradictory -1630 answer;hardware pending |
| NUM-5 other encodings | Not Supported |No new implementation/tests |Still future backlog |

Earlier numeric rows describe the pre-follow-up assessment. This explicit update
supersedes their claims that widths4/5/10/12 cannot be selected; it does not add
missing explanatory steps or promote extension hardware coverage to Full.

## M7 implementation assessment — engineering verified; user acceptance pending

M7 explicitly approved by “批准进入M7”. Source `combinational.[ch]`, native UI,
shared host/SuperH fixtures and independent host graph interpreter now implement
the bounded workflows below. Keep Partial Coverage until user acceptance; related
math tools alone were not sufficient for these rows. M8/M9 remain unapproved.

| Source ID | Actual implemented workflow/evidence | Current coverage | Limits |
| --- | --- | --- | --- |
| COMB-1 full adder |8truth rows;Sum/Carry SOP/POS,K-maps,actual XOR/AND/OR structural path,gate/MUX pin replay |Partial Coverage |One selected output at a time;physical user acceptance pending |
| COMB-2 ripple |Width1..8,operands/Cin;all stage sums/carries and carry wires;all operand/Cin combinations checked on host |Partial Coverage |Unsigned bit-pattern structure,no analog delay;hardware pending |
| COMB-3 P3.21 |3input popcount Count1/Count0=FA Carry/Sum;all8inputs plus gate/MUX connections |Partial Coverage |Circuit task only;Verilog still M8;hardware pending |
| COMB-4 HA |4row table,both output equations/K-maps/actual gate/MUX paths |Partial Coverage |Hardware pending |
| COMB-5 P4.12 |Separate MUX_Sum/MUX_Carry instances;S1=x0,S0=y0,data[c0,!c0,!c0,c0] and[0,c0,c0,1];all8rows and pin functions verified |Partial Coverage |Select output to view each MUX;not a single graphical circuit;hardware pending |
| COMB-6 seven-segment |Standard decimalBCD a-g,all7equations/K-maps/gates/selectedsegmentMUX;both polarities,zero-pin/blank/DC policies;all16codes checked |Partial Coverage |Fixed decimal glyph mapping;hardware pending;DC completion need not match an arbitrary desired invalid glyph |
| COMB-7 encoder |Ordinary4to2/8to3,all16/256patterns,code OR-of-indices plus exact-one VALID circuitry,ZERO/MULTI diagnostics |Partial Coverage |No priority encoder/general8variable minimizer;invalid code values disclosed but unusable without VALID |
| COMB-8 generic |2..4var truth0/1/DC or79char expression -> exactSOP/POS,K-map groups,actual gates,2/4/8:1MUX and verification |Partial Coverage |All2/3var Boolean functions/select orders exercised;4var representative/DC integration plus existing exhaustive minimizer tests;no global minimal gate/MUX count |

All rows have actual engineering evidence, not planned-test claims. No Verilog
coverage row is upgraded by this circuit milestone, and no M9 joint sequential/
output/timing composition is claimed. M5/M6 hardware and custom-width follow-up
acceptance retain their separate pending/deferred states. Earlier table rows
preserve prior assessments; this section supersedes their M7 unimplemented claims.

## M8 HDL implementation — engineering verified; user acceptance pending

M8 explicitly authorized by “继续做M8”. All below rows have actual C generation,
Verilog-2001 compilation,independent behavior benches,native code/export evidence.
Keep Partial Coverage until user acceptance;Verilog tests are not physical FPGA
verification. M9 remains unapproved;older planned rows retain historical context.

| Source/topic | Actual implementation and acceptance evidence | Current coverage | Limits |
| --- | --- | --- | --- |
| HDL-1 P3.21 adder extension |Behavioral/structural WIDTH1..32,primitive/chain,explicit carry ports;all1bit inputs,5bit exhaustive,boundary/random larger widths,actual `.v` export |Partial Coverage |P3.21 popcount circuit remains separately M7;noFPGA synthesis/timing |
| HDL-2 P4.20 |Ordinary4to2 encoder with exact-one VALID;all16binary inputs including invalid,code OR-of-indices |Partial Coverage |No priority;ignore invalid code withoutVALID;physical acceptance pending |
| HDL-3 P4.21 |INPUTS8 override/default generation,all256binary patterns,native8input export |Partial Coverage |Binary contract,no arbitrary-X guarantees;no priority |
| HDL-4 D/T/JK |Edge,reset mode/polarity/value choices;actual characteristic references and independent event/hold/reset benches |Partial Coverage |No SR HDL family,no power-up init;no-resetT-X explicit |
| HDL-5 P5.13 |WIDTH1..32universal shift;allrequest combinations,reset>load>right>left>hold,serial insertion;1/2/5/8/32tested |Partial Coverage |Chosen explicit flag interface/priority,no analog delay;user acceptance pending |
| HDL-6 P5.20 |Modulo1..65535,safeWIDTH+EXTRA_BITS,enable,SYNC reset;1/2/3/12/31/65535wrap/reset/guard tests;native12and1exports |Partial Coverage |No arbitrary symbolic counter HDL or asynchronous counter reset;M9 composition unimplemented |
| HDL-7 P5.23 |RingWIDTH1..32,seedindex,direction/edge/resetchoices,enable;one-hot/period/hold tests,native1/5/8exports |Partial Coverage |Reset required,no repair of arbitrary invalid rings;physical acceptance pending |

17native production files match host bytes and are all consumed by the second
343case HDL matrix.70native UI pages/19relations and previous regressions pass.
Actual USB transfer and FPGA realization remain untested. No INT-1/INT-2 or M9
joint-solver workflow is promoted by the presence of a code generator.
