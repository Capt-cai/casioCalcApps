#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
OUT="${ECEN21_M8_WORK:-$ROOT/.phase1/ecen21-m8}/host-tests"
mkdir -p "$OUT"
"${CC:-clang}" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I"$ROOT/apps/ecen21/src" -I"$ROOT/experiments/boolean" "$ROOT/apps/ecen21/src/hdl.c" \
  "$ROOT/apps/ecen21/src/flipflop.c" "$ROOT/apps/ecen21/src/combinational.c" \
  "$ROOT/apps/ecen21/src/solver.c" "$ROOT/experiments/boolean/exact_boolean.c" \
  "$ROOT/apps/ecen21/tests/hdl_generate_main.c" -o "$OUT/generate-hdl"
/opt/homebrew/bin/python3.11 "$ROOT/scripts/verify-hdl.py" | tee "$OUT/hdl-simulation.log"
