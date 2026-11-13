# M2B portable Boolean model

The approved M2B model is implemented in `apps/ecen21/src/solver.[ch]`, separate from gint input/display. It directly calls the existing, unchanged exact engine in `experiments/boolean/`. Upstream `apps/logic/` and shared application code are unchanged. The parser's recursive-descent structure is adapted from Logic with its original MIT copyright attribution retained; diagnostics and state handling are new.

## Domain and grammar

- Explicit active variable count: 2, 3 or 4, with A the most significant row bit. Initial state is four variables, `AB+C`, ON mask `FCCC`, no DC, valid but not yet solved.
- Expressions are at most 79 characters plus NUL. The cursor is a zero-based insertion offset. ASCII-printable insertion is bounded; malformed partial input is retained for editing and diagnosed.
- Variables A–D are case-insensitive. Variables outside the selected active count are errors rather than silently widening the table.
- Operators from low to high precedence: OR `+`/`|`, XOR `^`, AND `*`/`.`/`&` or juxtaposition, prefix NOT `!`/`~`/`/`, and atoms (variables, `0`, `1`, parentheses). Postfix `'` complements an atom or parenthesized expression and can repeat. Prefix NOT applies recursively. Spaces, tabs, CR and LF are skipped.
- Entire input must be consumed. Empty/whitespace-only input is distinct from syntax failure. The first diagnostic includes an explanatory message and **zero-based** source offset in `error_pos`; the UI may present position +1.
- Expression input has no symbolic X value; don't-cares are truth-table edits, keeping the expression parser and ternary table semantics separate.

## Source and result invariants

`BS_EXPRESSION` and `BS_TABLE` are explicit sources. Re-evaluating an expression clears DC and constructs its truth function for the current variable count. Empty/invalid expressions clear ON/DC, both prior solutions and validation counts, and set `valid=false`. No old truth table or solution remains valid behind a syntax error.

In table mode, row edits cycle `0 -> 1 -> X -> 0`. ON and DC remain disjoint. Saved expression text remains available but is **not** reapplied by solving, changing views or altering the table's variable count. It is reapplied only through `bs_use_expression()` or an actual expression edit. Backspace at cursor zero is not an edit and leaves table mode untouched. An explicit first table edit after invalid/empty text starts from that text's cleared truth data and establishes a new valid table.

Changing variables in expression mode reevaluates the saved expression, including an out-of-active-range error when necessary. In table mode, reducing variables retains only low-domain rows; increasing variables adds zero rows. This is truncation/reinterpretation, not Boolean elimination or projection; the UI must warn that lower rows are retained. The API rejects counts outside 2–4 and rows outside the active domain without modifying the model.

Every actual edit/source/variable change clears both formatted solutions, both result structs and `checked_rows`. `bs_solve()` computes exact SOP and POS with the same ON/DC specification and validates both against every cared row. It counts cared rows once, not twice. DC rows may legitimately receive different completions in SOP and POS. An all-DC function follows the engine convention: empty SOP `0`, empty POS `1`, zero cared rows. Internal verification/formatting failure clears all solutions and returns an error.

The exact engine returns ordered cubes for each form, which the UI can use directly for explanations and group highlighting. A SOP cube's fixed bits describe a product term; a POS cube covers zero rows and contributes its complemented clause. A single-cube `eb_result` may be formatted through `eb_format()` rather than reconstructing literal polarity in the UI.

## Karnaugh layout

`bs_kmap_row()` maps grid coordinates to the corresponding binary truth-table row. Layouts are 2x2 for two variables (A/B), 2x4 for three (A/BC), and 4x4 for four (AB/CD), with Gray order `00,01,11,10`. Invalid coordinates/counts return `UINT_MAX`. `bs_row_value()` returns 0, 1, 2 for X, or -1 for an invalid model/row.

## Verification performed

The reusable fixture `bs_run_checks(unsigned *checks)` is in `apps/ecen21/tests/solver_checks.c`. It returns zero on success or its failing source line. Root integrates it with the existing host/SuperH runners.

An isolated host build using Apple Clang C11, strict warnings as errors, AddressSanitizer and UBSan passed:

```text
PASS: M2B solver; 6510 checks; failure line 0
```

Evidence: `.phase1/m2b-model/results.log`. The check count excludes the earlier independent exact-engine regression, which remains a separate suite.

Coverage:

- Known literal masks, aliases, precedence, case/whitespace, constants, nested parentheses, complements and malformed tokens with exact first-error positions.
- All 256 choices of a,b,c,d in `!(a+b)c^d`, each evaluated over all 16 rows against independently written direct Boolean operations rather than another recursive parser.
- Initial `AB+C` output `C+AB`; its POS `(B+C)*(A+C)`; `A'(B^C+C^D)` as the four-variable `007E` counterexample with exact SOP cost 3 terms/9 literals and POS cost 3 clauses/7 literals; parity `6996`.
- Table row cycling, cared-row validation, explicit source restoration, saved expression preservation, invalid/empty-to-table transitions, both solution invalidation and variable-count truncation.
- Two-variable `A'B` with row zero changed to X gives SOP `A'` and POS `(A')`, each one term/clause and one literal; all-DC and all-one constants for every supported variable count.
- 79-character prefix/parenthesis boundaries, overlong rejection without mutation, cursor insertion/backspace, control-character rejection and invalid cursor/row/count guards.
- Every active K-map cell's expected row, bijection, horizontal/vertical wrap adjacency, coordinate rejection and cell count for all possible cubes in 2–4 variables.

This is bounded functional coverage, not exhaustive expression-string fuzzing or a complete four-variable ternary oracle. The unchanged engine's independently tested exactness is documented in `BOOLEAN-MINIMIZATION.md`. Host success is not a physical calculator result. Root owns the fresh target build, target fixture execution, UI verification, integration/resource review and milestone acceptance handoff.

Lead integration subsequently added six checks for 79 unclosed parentheses and 79 prefix operators, including first-error position and stale-result clearing. **Final shared count: 6,516 model checks**, plus 218 unchanged legacy controller checks; both suites pass under host sanitizers and as SuperH machine code in the emulator. The 6,510 figure above preserves the subagent's original isolated result. Final integration evidence is in `.phase1/ecen21-m2b/` and root `TESTING.md`.

## Resource limits

The production model uses fixed buffers and no heap. Results reserve 256 characters per form, safely above the bounded four-variable formatted cube domain, and reject any reported truncation. Parser recursion is bounded by the 79-character input limit; the deepest nested-parenthesis and long-prefix valid cases are exercised on the host. Parser stack and solve stack do not overlap in normal model operations, but target compiler frame estimates and physical worst-case/interrupt overhead still require integration review. No support above four variables or unbounded expressions is claimed.

Target compilation reports parse_or=20, parse_xor=20, parse_and=24 and parse_not=16 bytes/frame (atom handling is inlined), recompute=64 and main=224. Incomplete input can have 79 opening parentheses, deeper than a balanced valid expression; a conservative 80-level frame illustration is about 6,704 bytes including those main/edit/recompute frames before helpers/interrupts. The deepest malformed cases execute in the target fixture. This estimate is not a measured hardware peak; minimization uses a separate non-overlapping call chain. See root `TESTING.md` for solve-chain estimates.
