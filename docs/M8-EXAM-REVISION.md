# M8 exam-oriented Verilog revision — 2026-11-13

## Scope and design

This explicitly authorized task revises M8 only; M9 remains unapproved.
The original M8 engineering artifact/tag, all previous user guides/PDFs and all
other six tools remain unchanged. No user manual was regenerated or written.

Portable hdl.c emits bounded source, hdl_ui.c handles settings/pages, and the
existing hdl_export/hdl_storage layers retain confirmation, exact-byte writes,
8.3 names, slot00..99 and no-overwrite/failure cleanup. Changes are limited to
M8 generator/UI/tests plus reproduction/verification helpers and records.

The original nine menu entries remain. Every adder now has a Model setting:
BEHAVIORAL -> DATAFLOW -> STRUCTURAL -> BEHAVIORAL. Existing menu entry1 starts
behavioral; entry2 starts structural. Dataflow has its own internal kind appended
without renumbering original kinds, module ecen21_add_d and ECADxx.v prefix.
Behavioral uses always@(*) and complete blocking assignment; dataflow uses assign;
structural uses actual xor/and/or primitives inside a one-bit cell and a generated
ripple chain of cell instances. Wiring-only carry assigns do not replace cells.

All widths/moduli/seeds/reset choices are validated on the calculator before
source generation. Output specializes those values, removing Verilog parameters,
runtime parameter guards, initial/$display/$finish, synthesis directives, truth
row comments, superfluous signals/extensions and explicit hold assignments where
omission already retains state. Width1 shift/ring templates avoid negative
part-selects. Raw code lines fit46characters: identifiers no longer split across
screen rows. Existing lossless paging remains as a fallback.

Functional contract is retained: ordinary encoder code ORs active indices and
valid marks exactly one; selected clock edge/reset polarity/value/mode; reset >
load > right > left > hold; modulo enable, wrap and invalid-state recovery;
ring seed/direction/enable. q has no assumed startup value. This revision does
not offer external HDL parameter overrides; change calculator settings and
regenerate. Width1..32 and every original setting remain available. This is
intentional fixed-selection exam output, not loss of selectable circuit sizes.

## Measured simplification

Defaults:5bits,4encoder inputs,modulus12/width4,seed0,rising edge,sync high reset.
Bytes include the at-most-one blank-line padding for even BFile writes. Lines
count nonblank physical lines, including the old explanatory/guard statements.

| Family | Old bytes -> new | Old lines -> new | Byte reduction |
|---|---|---|---|
| Behavioral adder (old source was dataflow) |458 ->138|15 ->5|69.9%|
| Structural adder |838 ->424|29 ->19|49.4%|
| Encoder |868 ->248|29 ->12|71.4%|
| D |684 ->122|26 ->6|82.2%|
| T |688 ->126|26 ->6|81.7%|
| JK |878 ->240|36 ->12|72.7%|
| Universal shift |964 ->338|29 ->13|64.9%|
| Modulo-N |1254 ->180|42 ->8|85.6%|
| Ring |708 ->156|22 ->6|78.0%|
| New separate Dataflow adder |new:124|new:5|not comparable as new entry|

Actual emitted sources:releases/engineering/M8-exam/verilog-examples/,
including ECAB00.v,ECAD00.v,ECAS00.v. Original length baseline was generated from
unchanged source at81c93e2 (same hdl.c as engineering/m8-recovered-p0), before edits.

## Actual verification

- Ten host suites with AddressSanitizer/UndefinedBehaviorSanitizer:PASS.
  M8 generator/export/paging/line-bound checks:555404; prior suites unchanged.
- Actual Icarus13.0 -g2001 +vvp:328cases/280690checks. Includes simultaneous
  behavioral/dataflow/structural adders, exhaustive small-width sums/encoder
  patterns,32bit carry, widths1/2/3/5/8/16/31/32, FF edge/reset options,
  shift request priority/directions,modulus1..65535,wide invalid-state recovery,
  ring widths1..32/seeds/rotation and initial-X checks.
- Separately compiled the preserved original generator; its output passed the
  same328cases/280690independent-oracle checks. This demonstrates agreement on
  tested functionality, not formal equivalence for arbitrary X/Z stimuli.
- Old343case suite included8runtime parameter-guard cases and6HDL-width-override
  cases. Those mechanisms intentionally no longer exist in emitted code. Removed
  those obsolete cases; replaced3modulus-override cases with2fixed wide-recovery
  cases. Adder cases now check all3models. Invalid calculator configurations
  remain tested in shared C checks. Do not compare case counts as coverage alone.
- Clean pinned ARM64 Docker build:production,EC21Test,EC21Bench all passed.
  Target fixtures byte-match all10host reports.40maximum configs pass;peak428bytes,
  20visual lines. Whole benchmark2,976,944emulator instructions (not hardware time).
- Full simulator regression:467previous-tool reference screens unchanged;
  M8:74images/21interaction invariants,including model invalidation,complete
  page roundtrip, slot conflict protection and real read-only-disk failure.
-18production BFile exports exactly match host bytes,compile and simulate under
  Icarus; every exported file is used in a behavioral bench.
- All10model variants visited every source page at wide/max selected values:
  16code pages,10wrap-to-first checks. Visually inspected all16final pages;
  no identifier splitting, lost rows or footer obstruction.
- All10old recovery archives and original M8 release hashes pass. All10existing
  user guides/PDFs compared byte-for-byte with81c93e2 and remain unchanged.

## Artifact and evidence

New engineering artifact:releases/engineering/M8-exam/ECEN21.g3a,156256bytes.
SHA256:8e2c940df3b16878c1f1a8c1b617e4b37f95b34fc7061161951bb2a3c80e4152.
Old M8 remains releases/engineering/M8/ECEN21.g3a,158376bytes,
SHA256:a893609cb6a6757a5b20dbd253495339f3d9c2e6e1a5a13fa7c2ce01480824df.
New compiler estimates:text127104,data464,bss24304bytes; these are not measured
physical peak memory or runtime. Target ELF remains SuperH,not a host executable.
Reports,source manifest,18exports and hash index are archived alongside new binary.
Ignored detailed logs/screenshots: .phase1/ecen21-m8-exam/.

Reproduce from repo root with the existing pinned toolchain/emulator/Icarus:

```sh
export ECEN21_M8_WORK="$PWD/.phase1/ecen21-m8-exam"
bash apps/ecen21/tests/run-host.sh
bash apps/ecen21/tests/run-hdl.sh
bash scripts/toolchain-build-m8.sh
/opt/homebrew/bin/python3.11 scripts/verify-m8.py
/opt/homebrew/bin/python3.11 scripts/verify-m8-exam-pages.py
python3 scripts/verify-version-archives.py
```

The baseline verifier additionally accepts --baseline-generator PATH. To recreate
that executable,extract the original hdl.c/headers and its unchanged supporting
sources from engineering/m8-recovered-p0 into a separate directory,then compile
hdl_generate_main.c with clang,hdl.c,flipflop.c,combinational.c,solver.c and
exact_boolean.c. Do not overwrite current worktree files.

Engineering verification passed. User physical acceptance of this exact revision
is pending. M5/M6 deferred,M7/custom-width/M8 acceptance statuses are not promoted.
No physical USB/OS-return/performance/FPGA synthesis result is claimed. Existing
emulator diagnostic of unused trailing exit keys remains logged. No M9 work.
