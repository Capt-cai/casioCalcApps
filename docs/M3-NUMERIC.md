# M3 portable numeric model

The bounded numeric model is in `apps/ecen21/src/numeric.[ch]`. It has no gint dependency, heap allocation or external library requirement. It adapts the upstream Base-N application's width masking, two's-complement magnitude and reverse-digit formatting conventions with its MIT copyright attribution retained. The original Base-N, Logic, common sources and exact Boolean engine are unchanged. Parsing, explicit validity/error handling, arithmetic flags and the controller API are new.

## Domain and API

Supported widths are **8,16,32 bits** and bases are **2,10,16**. `nm_init` starts with unsigned8-bit decimal addition, A=`255`, B=`1`, both inputs valid, no solved result. The public `nm_state` keeps bounded editable input text, canonical raw operand bits, current settings and result/flags. `nm_run_checks(unsigned *checks)` is the shared test entry point; root owns host/target runner integration.

`nm_format` returns the required length excluding NUL, supports `(NULL,0)` size queries and terminates valid output when capacity is positive. Binary uses exactly `width` digits, hex exactly `width/4` uppercase digits, and decimal has no padding. Signed interpretation applies only to decimal output. Formatting first masks the raw value to the requested width. Invalid format arguments return -1; invalid `nm_mask`/`nm_signed_value` widths return zero. Negative32-bit decimal formatting uses a wider magnitude rather than negating a signed32 minimum, and production formatting does not depend on printf64 support.

## Input and validity

- Signed decimal accepts a leading minus and the inclusive range `[-2^(w-1),2^(w-1)-1]`; unsigned decimal accepts `[0,2^w-1]` and no sign.
- Binary and hexadecimal specify unsigned raw bits, regardless of display mode; no signs or `0b`/`0x` prefixes. Lowercase hex digits are accepted. More binary/hex digits than the active width permits are rejected, even if leading zeros would fit numerically.
- Decimal leading zeros are accepted within the34-character text capacity when the value fits. Empty text, minus alone, whitespace, plus sign, decimal point, invalid digits and overflow are errors.
- Bounded malformed text is retained for editing. A too-long setter retains its first34 characters but marks that operand invalid; solve never reparses this prefix as valid. An explicit subsequent edit can repair it. Capacity/character errors are shown rather than silently ignoring a digit.
- Every input edit invalidates the prior result and clears arithmetic flags. Invalid inputs cannot reuse the last operand/result as if valid. A valid edit clears the truncation warning; failure does not acknowledge it.

Base, width and signed-mode changes are rejected while either operand is invalid, without reinterpreting the invalid text. The error requests repair of both operands first. Samples may provide a valid reset. Operation changes are allowed while invalid, but solve still refuses invalid inputs. Settings and successful calculations clear their old error message; unsupported setting requests retain the previous setting and invalidate the answer.

## Settings and truncation

Changing base or signedness preserves the raw bits, reformats both valid inputs and invalidates arithmetic output. Signedness toggles reinterpretation, not numeric-value preservation; unsigned8-bit255 becomes signed-1 without changing `FF`.

Increasing width zero-extends unsigned operands and sign-extends negative signed operands. Decreasing width retains low bits. The model compares each operand's interpreted old/new value and raises `truncated` only when that interpretation changes. Thus signed16-bit `FFFF` to8-bit `FF` does not warn, but signed16-bit `00FB` (+251) to8-bit `FB` (-5) does. The warning persists through solve and operation changes, and reapplying the same width; a successful explicit input edit or actual base/mode/width change may clear or replace it.

## Arithmetic flags

The retained result is modulo `2^width`. Addition uses a uint64 sum; subtraction uses defined unsigned wrap and width masking. Carry is meaningful for ADD and false for SUB. Borrow is meaningful for SUB and false for ADD. Signed overflow is computed independently of selected display mode from operand/result sign bits. `wrapped` reports signed overflow in signed mode, or carry/borrow in unsigned mode, so the native view can distinguish result truncation from ordinary exact integer arithmetic.

All full32-bit boundaries are handled without shift-by32 on a32-bit value or signed32 overflow. Signed conversion uses int64 arithmetic. Invalid direct state settings/operand bits cause solve to fail with no old solved result.

## Independent verification performed

An isolated Apple Clang build ran with C11, `-Wall -Wextra -Werror -pedantic`, AddressSanitizer and UBSan, with these actual results:

```text
PASS: M3 numeric; 20779 checks; failure line 0
PASS: M3 exhaustive numeric; 2359296 checks; failure line 0
```

Evidence: `.phase1/m3-numeric/results.log`; isolated driver and executable are in the same ignored directory. No target or physical test is claimed by this subtask; root performs fresh SuperH build/execution and native UI validation.

The production overflow implementation uses bit signs, while the independent arithmetic oracle converts operands to wider signed mathematical values, performs addition/subtraction and compares against numeric ranges. Its retained result uses signed modulo normalization. It independently checks carry, borrow, signed overflow, selected-mode wrapping, result range and solved/error status.

Host-only `nm_run_exhaustive`, compiled with `NM_FULL_HOST_TESTS`, checks **every65,536 8-bit operand pair for both operations and both display modes**:262,144 cases. It also checks4,096 reproducible pairs each for16/32 bits, both operations/modes:32,768 cases. Total294,912 arithmetic cases, each eight assertions, yield2,359,296 checks. This function is excluded from normal target fixtures to keep emulator runs bounded.

Shared `nm_run_checks` covers all8-bit raw values in every base and both interpretations, with independent decimal printf-long/bit/nibble string comparisons, parse roundtrips, required lengths and tiny-buffer termination. It includes16/32-bit boundary conversion/arithmetic matrices and deterministic sampled arithmetic; all proposed milestone examples; signed32 minimum; sign/zero extension; meaningful shrink warning persistence; invalid text/digits/ranges, repair, empty and too-long prefixes, stale-output clearing and blocked settings while invalid. The20,779 count is assertions, not a claim of exhaustive16/32-bit arithmetic or every input string.

## Integration limits

The model uses fixed buffers and small integer fields. Input parsing and formatting are iterative and bounded; no recursion or heap is used. Actual target stack/object measurements belong to the root integration records. Complete host success does not certify calculator keyboard, screen, OS return, physical timing, peak stack or USB behaviour. The model supports no64-bit mode, octal, multiplication/division, bit shifts/rotates or general arithmetic expressions.

An additional strict arm64 host `-Os -fstack-usage` compile passed. Its largest reported production function frame is96 bytes (`nm_format`); parser64 bytes, solve32 bytes, with caller/library overhead not included. The Mach-O object has4,644 bytes of `__TEXT` sections and no data/BSS section, plus unwind/alignment metadata. These host observations do not substitute for SuperH measurements. Evidence: `.phase1/m3-numeric/numeric.su` and `numeric.o`.

## Lead integration verification

Main Agent independently reran both host suites, then the same shared 20,779 numeric checks executed successfully as SuperH code alongside 6,516 Boolean and218 legacy checks; the emitted target report exactly matches the passing host report. Host-only exhaustive testing is excluded from the target harness. All59 archived Boolean screen pixels match after the selector extraction, and54 numeric/navigation screenshots with15 interaction invariants pass. The target fixture exits after39,252,272 emulated instructions including all three suites/report writing, not a pure numeric performance benchmark.

Target GCC14.1.0 `-Os -fstack-usage` reports parse64, nm_format84, nm_solve40 bytes/frame; numeric view128 and conversions116, launcher44. Library helpers, interrupts and dynamic gint resources are not included. The full production ELF has text58,512, data464, BSS2,048 bytes. No hardware peak/latency claim is made. Final evidence is under `.phase1/ecen21-m3/`; root `TESTING.md` records artifact identity and acceptance limits.
