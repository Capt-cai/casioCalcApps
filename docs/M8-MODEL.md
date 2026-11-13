# M8 Verilog model, export contract and verification

M8 explicitly authorized by “继续做M8”. M7 acceptance remains pending; M5/M6 hardware
verification deferred; custom-width follow-up pending. M9 unapproved. No earlier
acceptance is inferred from this next-stage instruction.

## Composition and bounds

New portable `hdl.[ch]` emits deterministic ASCII Verilog-2001 source. Fixed module
identifiers and bounded structured inputs prevent identifier/path injection.
`hdl_export.[ch]` provides injectable file IO; `hdl_storage.c` is the thin gint/OS
adapter; `hdl_ui.c` owns retained parameters/code/pages. All prior computational
and UI modules are unchanged except launcher integration. No global IR refactor.
The existing M4 FF characteristic API and M7 ordinary encoder API supply generated
reference comments, preserving shared semantics without duplicating those solvers.
Behavioral simulation tests remain independent of generator strings/comments.

| Family/file prefix | Parameters/ports and contract |
| --- | --- |
| Behavioral adder ECAB |WIDTH1..32,a/b/Cin -> Sum/Cout; explicit WIDTH+1 arithmetic |
| Structural adder ECAS |WIDTH1..32; included ecen21_fa1 XOR/AND/OR primitive; LSB-first carry chain |
| Ordinary encoder ECEN |INPUTS4or8; code width2or3; OR of asserted indices,exact-one VALID; binary input contract,no priority |
| D/T/JK ECDF/ECTF/ECJK |Selected rising/falling edge;NONE/SYNC/ASYNC reset,active high/low,RESET_VALUE0/1;rst always a port but ignored inNONE |
| Universal shift ECSR |WIDTH1..32,parallel,load,right/left requests,serial_left/serial_right;reset>load>right>left>hold; right inserts atMSB,left atLSB |
| Modulo ECMN |MODULUS1..65535; WIDTH default safe ceil(log2(N))+EXTRA_BITS,minimum1; selected wider width preserved;Cin absent,enable holds;SYNC reset only |
| Ring ECRG |WIDTH1..32,SEED_INDEX0..WIDTH-1,enable;chosen rotation,edge,reset polarity/synchrony;reset required;one-hot seed |

Source parameters retain override support. Invalid WIDTH,encoder size,modulus,
insufficient counter width,seed or FF reset-value overrides have simulation-only
parameter guards between synthesis translate_off/on comments. Guards use no
SystemVerilog syntax; source is compiled with-g2001. Counter safe width uses
Verilog-2001 constant ternaries rather than `$clog2`. Extra bits can be overridden;
changing MODULUS alone recomputes its minimum. A combination exceeding32bits is
rejected rather than silently truncated. No synthesis/place-route or FPGA test
was performed; synthesis-directive support remains target-tool dependent.

Generated q has no assumed initial power-up value. Reset or D/JK capture/parallel
load establishes state. No-reset T starts and remains unknown in ordinary HDL
simulation until initialized externally; no artificial zero state is injected.
Ring requires reset;it does not repair arbitrary invalid one-hot states. Counter
recovers known binary q>=MODULUS-1 when enabled. Four-state unknown encoder inputs
and analog/metastability/setup-hold behavior are outside the binary contract.

## Source and file safety

Document capacity8192 includesNUL;successful text is<8192bytes and even-length.
An extra newline is appended when needed, never a binaryNUL, because gint BFile
requires even writes. Fixed config validation precedes emission; failure clears
validity,length and code. The saved config is compared before export,ignoring only
file slot,so stale code cannot be relabelled as a changed design.

Visual pages wrap46characters and preserve every source character/newline; the
export is the original text,not the visually wrapped lines. Numeric modal cancel,
help,type-menu cancel and tool reentry preserve state. Type replacement and actual
source parameter changes discard derived code. Export slot changes preserve code.
No project persistence after application exit is promised; exported files persist.

8.3 names consist of fixed family prefix,slot00..99 and `.v`. Default ECAB00.v etc
are written to `\\fls0\`,the USB storage root. F4 opens review and F6 explicitly
confirms. Existing files are never overwritten or deleted. Change slot or remove/
rename files externally to reuse names. A successful new file is created at exact
size,opened,written once with an even-sized source buffer,then closed. Partial/
negative writes and close failures are errors; only a newly created failed file
is removed where possible,and cleanup failure is surfaced. Lookup/create failure
never deletes an existing file. No arbitrary path or subdirectory is accepted.
All BFile operations execute within gint_world_switch. Return conventions and
even-write requirement were inspected in local pinned gint `include/gint/bfile.h`.

## Validation evidence and limits

Portable C fixtures exercise generation/paging for all widths,clock/reset choices,
config errors,bounded filenames,stale-document matching,odd/malformed export
rejection and mock lookup/create/open/short-write/write/close/cleanup failures.
They run sanitized on host and as actual SuperH code. Largest representative
settings are measured separately,not confused with physical performance.

`scripts/verify-hdl.py` consumes the real C generator CLI,compiles actual emitted
source using Icarus-g2001 and executes independently written Verilog benches.
Adder expected sums come from Python wide integers;encoder from every16/256binary
pattern;FF event/hold/reset cases cover types,edges,polarity,values and known-state
JK transitions;no-resetT-X is explicit. Shift covers all simultaneous request
combinations and serial data atWIDTH1/2/5/8/32. Modulo tests1/2/3/12/31/65535,
reset between edges,enable,illegal-state recovery and parameter overrides. Ring
covers both directions/edges/reset modes/polarities,seed endpoints plusbit2,
WIDTH1/2/5/8/32,one-hotness/period and enable hold. Invalid override guards are
actually simulated. These binary tests do not establish arbitrary four-state
input behavior or FPGA timing.

Native UI scripts export17actual production files. Every file is byte-compared
against independently invoked host emission for its exact settings. A second
343-case bench matrix consumes all17files,using compatible parameter overrides;
other cases still use newly generated variants. Export-source-use counts are
recorded per file,not misrepresented as all343cases testing every file. Native
read-only storage induces a real create failure with no `.v` file left. True disk
exhaustion/physical USB/long-run calculator behavior remain unmeasured.

Icarus13.0 was installed as the Apple Silicon Homebrew `icarus-verilog` package
for M8; the fxSDK container/toolchain was unchanged. Command flags and language
selection reference [official Icarus documentation](https://steveicarus.github.io/iverilog/usage/command_line_flags.html).
Actual version,compile/run logs,reports,source/artifact hashes and limits are
recorded in TESTING and `.phase1/ecen21-m8/`.
