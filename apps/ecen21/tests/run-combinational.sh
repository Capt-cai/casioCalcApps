#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
OUT="$ROOT/.phase1/ecen21-m7/host-tests"
mkdir -p "$OUT"
"${CC:-clang}" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer -DCB_FULL_HOST_TESTS \
  -I"$ROOT/apps/ecen21/src" -I"$ROOT/experiments/boolean" \
  "$ROOT/apps/ecen21/src/combinational.c" "$ROOT/apps/ecen21/src/solver.c" \
  "$ROOT/experiments/boolean/exact_boolean.c" "$ROOT/apps/ecen21/tests/combinational_checks.c" \
  "$ROOT/apps/ecen21/tests/combinational_host_main.c" -o "$OUT/combinational-full"
"$OUT/combinational-full" | tee "$OUT/combinational-full.log"
