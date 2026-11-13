#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
OUT="$ROOT/.phase1/ecen21-m6/host-tests"
mkdir -p "$OUT"
"${CC:-clang}" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer -DSQ_FULL_HOST_TESTS \
  -I"$ROOT/apps/ecen21/src" -I"$ROOT/experiments/boolean" \
  "$ROOT/apps/ecen21/src/sequential.c" "$ROOT/apps/ecen21/src/solver.c" \
  "$ROOT/apps/ecen21/src/flipflop.c" "$ROOT/experiments/boolean/exact_boolean.c" \
  "$ROOT/apps/ecen21/tests/sequential_checks.c" "$ROOT/apps/ecen21/tests/sequential_exhaustive_main.c" \
  -o "$OUT/sequential-exhaustive"
"$OUT/sequential-exhaustive" | tee "$OUT/sequential-exhaustive.log"
