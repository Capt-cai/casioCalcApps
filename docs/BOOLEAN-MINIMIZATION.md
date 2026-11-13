# Exact Boolean minimization experiment

Implemented and host-verified November 12, 2026. This is a portable C prototype in `experiments/boolean/`, not a change to the existing Logic application and not a calculator integration. It has no gint, fxSDK, heap or external library dependency.

## Why the existing cover is nonminimal

Logic enumerates prime implicants, selects essentials, then repeatedly takes the prime covering the largest number of remaining minterms. Greedy set cover preserves the truth table but cannot guarantee the smallest cover. Ties depend on enumeration order and there is no global search or literal-cost objective. The Phase 1 exhaustive audit found 2,288 four-variable functions with unnecessarily many terms.

For four variables and ON rows `{1,2,3,4,5,6}` (`0x007e`), the existing selection uses four product terms. An exact solution is:

```text
A'C'D + A'BD' + A'B'C
```

It has three terms and nine literals. The new independent test oracle and the lead engineer's separate brute-force review agree on that cost. No existing Logic file has been altered.

## API and semantics

`exact_boolean.h` exposes:

- `eb_minimize(n, on, dc, form, output)`: minimize a disjoint ON/DC specification for 2–4 variables; OFF is the complement within the active truth-table domain. Invalid input returns `-1` without modifying the output. Bits outside the active domain and overlapping ON/DC masks are errors.
- `eb_evaluate(result, row)`: evaluate the selected completion of the function, including the result chosen on don't-care rows.
- `eb_format(result, buffer, capacity)`: format SOP or POS text; return the full required length excluding NUL and terminate any nonempty destination. Querying length with `(NULL, 0)` is supported. Truncation is reported through the returned length rather than silently accepted.

A row index uses A as its most significant variable bit, matching Logic. `eb_cube.fixed` identifies constrained variables; `bits` identifies which are 1. SOP cubes represent product terms covering ON rows. POS cubes cover OFF rows, and the output clause is the complement of the cube. Thus a cube fixing `A=0,B=1` gives `A'B` in SOP or `(A+B')` in POS. The literal polarity flips for POS by De Morgan's law.

The objective is lexicographic: (1) cube/term count, (2) total literal count, (3) canonical sorted cube order, with cube order determined by ascending `(fixed,bits)`. The last rule makes equal-cost output deterministic; it does not claim that the result is the only optimum.

Don't-cares may be covered or left uncovered. They are never required rows and never forbidden rows. If no required rows exist, the empty cover is selected: SOP formats `0`, POS formats `1`. In particular an all-DC specification deterministically chooses SOP `0` and POS `1`. If nonempty required rows can be covered by the unconstrained cube, that single cube has zero literals and formats SOP `1` or POS `0`. Costs describe cover representation: the unconstrained cube counts as one cube, while the empty cover counts as zero. The two forms can choose different valid completions on DC rows.

## Exact algorithm and argument

1. Build `target=ON` for SOP; build `target=OFF` for POS. In either case `allowed=target | DC`.
2. Enumerate all `3^n <= 81` cubes, keeping cubes that cover at least one target row and no forbidden row.
3. Keep only prime cubes: a cube is nonprime if removing any one fixed literal still gives a valid cube. Restricting to primes is safe for the stated objective: expanding a cube never increases term count and strictly reduces its literals.
4. Search the prime-cover problem exactly. Choose an uncovered target row with the fewest incident primes, branch on every prime covering that row, and recurse after removing all rows that prime covers. Every valid completion must include one such prime, so this branching is complete.
5. Prune only when the current depth or the admissible bound `ceil(uncovered_rows / largest_remaining_coverage)` proves that a branch cannot improve the best term count. Complete covers are compared by terms, then literals, then sorted canonical cube indices. No greedy fallback or timeout can silently produce an approximate answer.

The result is an exact two-level cover, not a general multilevel Boolean circuit minimum. No hazards, XOR factoring, gate count, NAND/NOR technology mapping, symbolic parser, sequential synthesis or more-than-four-variable support is claimed.

## Independent verification

Run from the repository root:

```sh
bash experiments/boolean/run-tests.sh
```

The runner compiles C11 using Clang with `-Wall -Wextra -Werror -pedantic`, AddressSanitizer, UndefinedBehaviorSanitizer and frame pointers, then runs the tests. Generated files and logs are isolated under `experiments/boolean/build/`, which has a local ignore rule.

The oracle deliberately uses a different method from production. It enumerates all cubes from ternary variable patterns without prime filtering or branch-and-bound. A dynamic program finds the minimum `(terms,literals)` for each exact union truth mask. For a DC specification, it checks every completion `target | subset(DC)` and takes the cheapest result. This proves optimality relative to all cubes, rather than merely checking another implementation of the same prime-cover search.

Actual passing output:

```text
2 variables: all 16 fully specified functions, SOP/POS optimal
2 variables: all 81 ternary ON/OFF/DC specifications, SOP/POS optimal
3 variables: all 256 fully specified functions, SOP/POS optimal
3 variables: all 6561 ternary ON/OFF/DC specifications, SOP/POS optimal
4 variables: all 65536 fully specified functions, SOP/POS optimal
4 variables: 4096 deterministic randomized ON/OFF/DC specifications, SOP/POS optimal
greedy counterexample 0x007e: A'C'D+A'BD'+A'B'C (3 terms, 9 literals)
PASS: 153092 minimizations; 2237318 cared-row checks; constants, polarity, invalid inputs, formatting, deterministic ties
```

Each case checks exact cost, every cared row, formatted text via a separate evaluator, required-length formatting queries, short-buffer termination and canonical cube sorting. Selected cases are repeated to check deterministic cube choices. A separate fixture checks the tertiary tie choice for both forms: the three-variable target `0x3d` has two minimum-cost `(3 terms,6 literals)` covers, with `(fixed,bits)` lists `[(3,0),(6,2),(6,4)]` and `[(5,0),(6,2),(6,4)]`; the former must be selected. Those two optima were identified by independent all-cube subset enumeration. The broad DP oracle verifies the first two objectives; exhaustive tertiary-order optimality is not independently oracle-checked. Invalid input checks include out-of-range variables and truth masks, overlapping ON/DC, invalid form, NULL output, invalid result, and preservation of sentinel output on every rejected minimization. The all-DC convention is tested explicitly. The full four-variable ternary space is `3^16=43,046,721` specifications and was not exhaustively run; its fully specified subset and 4,096 reproducible ternary samples were tested. Sanitizers reported no memory or undefined-behaviour failures in the executed domain.

## Resource observations and integration limits

The production implementation uses fixed arrays of at most 81 candidates and 16 selected cubes, with no allocation and at most 16 selected terms/16 recursion edges. The result contains only small fixed-width values and an enum; it has no host pointers or ABI-dependent serialized format. Search remains exponential in the worst case and must stay behind the explicit four-variable limit.

Apple arm64 Clang `-Os -fstack-usage` reported `eb_minimize` 928 bytes and recursive `visit` 160 bytes per frame. A conservative 17 active visit frames plus the minimize frame is about 3,648 bytes, excluding callers and helper overhead. These are host compiler observations, **not SuperH stack measurements**. The optimized Mach-O object measured 2,000 bytes of `__TEXT`, zero `__DATA`, plus 160 bytes of other sections. The much larger test oracle tables are test-only and are not linked into a calculator application.

The lead subsequently compiled the unchanged prototype with the verified container's `sh-elf-gcc` 14.1.0, using `-std=c11 -Os -Wall -Wextra -Werror -pedantic -m4-nofpu -mb -fstack-usage`. Compilation passed without diagnostics. The SuperH object has 1,524 text bytes and zero data/BSS bytes. Static stack usage is 892 bytes for `eb_minimize`, 112 bytes per `visit` frame, 12 bytes for `eb_evaluate`, and 36 bytes for `eb_format`; 17 visit frames plus the minimize frame give a conservative 2,796 bytes before callers/helpers. Evidence: `.phase1/lead/target-core-compile.log` and `.phase1/lead/target-core/exact_boolean.su`. These are compiler estimates, not measured peak stack or runtime on the calculator, and no application was linked to this experiment.

Before integration, verify peak stack and worst-case latency on the fx-CG50, and define product-facing formatting/pagination. Keep the public API independent of gint and provide a separate screen/controller adapter. The existing Logic source-build gate remains separate from this prototype's verification.
