# M8 approved plan — parameterized Verilog generator

Status: engineering verification passed;user acceptance pending;M9 unapproved.

Authorization: user “继续做M8”. Earlier user/hardware acceptances remain separate:
M7 pending, M5/M6 deferred, custom-width follow-up pending. M9 unapproved.

Deliver Verilog-2001 source, native code paging and USB-accessible `.v` text export.
New portable generator modules reuse existing design semantics without modifying
prior cores/UI except launcher. Preserve M7 engineering source/artifacts/evidence.
Default single Main Agent; no subagents requested.

## Bounded families and inputs

- Behavioral and structural full adders: WIDTH1..32, default5; explicit Cin/Cout,
  LSB-first structural chain and included one-bit primitive, parameter override.
- Ordinary encoder: INPUTS4or8, code width2or3; deterministic OR-of-active-indices
  actual code and separate exact-one VALID, no inferred priority. Match M7.
- D/T/JK one-bit FF: rising/falling clock; no/synchronous/asynchronous reset;
  active-high/low reset and reset value0/1. No assumed power-up initialization.
- Universal shift register: WIDTH1..32, parallel load, left/right serial data;
  reset > load > right shift > left shift > hold priority. Explicit bit insertion;
  no invalid part-select at WIDTH1. Chosen clock/reset configuration.
- Modulo-N counter: MODULUS1..65535, derived minimum WIDTH>=1, user-configurable
  width up to32 and insufficient-width rejection, synchronous reset only,
  clock edge/reset polarity chosen, recovery if current q>=MODULUS-1.
- Ring counter: WIDTH1..32, seed index0..WIDTH-1, left/right rotation, enable,
  synchronous/asynchronous reset required. Reset seed one-hot, including WIDTH1.

Generated parameters retain safe named defaults; reject invalid generator inputs
before code creation and include simulation-only parameter guards where needed.
Code itself is synthesizable subject to the selected target tool; no FPGA place/
route or timing/hardware verification claimed. Preserve portable independent tests.

## Workflow and export

Select family -> edit structured settings -> generate -> page full source ->
inspect contract/ports -> explicitly export with confirmation. Fixed family file
names prevent path injection; do not silently overwrite user files. Handle create,
open,write,close/capacity failures and show status/path. `.v` text must match source
bytes, not wrapped visual lines. Test export creation/repeat/conflict/cancel and
failure paths through emulator BFile support; physical USB remains unverified.

## Acceptance checks (planned, not results)

M8-A1: portable bounded generator checks, guards, deterministic source/line paging,
invalid settings clear old code and no buffer truncation presented as success.
M8-A2: actual Icarus syntax/elaboration `-g2001`, all family defaults and parameter
variants; independent executable Verilog testbenches check behavior, not string
matching. Include structural/behavioral equivalence and all1bit adder cases.
M8-A3: encoder all16/256 inputs; FF all event/hold/reset/type/polarity combinations;
universal shift modes and simultaneous requests, WIDTH1/2/5/8/32; modulo1/3/12/
nonpowers/large bounds, between-edge sync reset, width failures; ring WIDTH1 and
larger, each direction, enable hold, reset seed and one-hot/period behavior.
M8-A4: fresh `.g3a`, shared SuperH tests, actual production native UI/export scripts;
exported bytes independently compiled/simulated on host; verify previous tools.
M8-A5: provenance/resource observations, updated records/coverage and Chinese
seven-part user acceptance guide. Stop at M8; M9 needs separate authorization.

## Engineering completion

All approved bounded families,parameter guards,native source/contract pages,
slot-based no-overwrite text export and IO failure handling implemented. Reuses
M4 characteristic/M7 encoder reference APIs;old cores/UI preserved except launcher.
Actual92120shared host/SuperH checks;343HDL cases/280704checks;17native exports all
byte-compared and consumed by the exported-source simulation matrix;70UIpages/
19invariants and previous regressions. Fresh158376byte user artifact;full actual
hash/resource evidence in root TESTING. Chinese seven-part guide delivered.
Engineering passed;user acceptance pending;M9 unapproved. No hardware acceptance
or physical USB/FPGA result inferred. Source cap8192/even-sized writes and141input
provenance verified. Defaults and representative bounds were measured separately.
