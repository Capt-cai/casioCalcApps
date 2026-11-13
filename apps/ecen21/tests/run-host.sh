#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
OUT="$ROOT/.phase1/ecen21-m8/host-tests"
mkdir -p "$OUT"
"${CC:-clang}" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I"$ROOT/apps/ecen21/src" -I"$ROOT/experiments/boolean" \
  "$ROOT/apps/ecen21/src/controller.c" "$ROOT/experiments/boolean/exact_boolean.c" \
  "$ROOT/apps/ecen21/src/solver.c" "$ROOT/apps/ecen21/tests/solver_checks.c" \
  "$ROOT/apps/ecen21/src/numeric.c" "$ROOT/apps/ecen21/tests/numeric_checks.c" \
  "$ROOT/apps/ecen21/src/flipflop.c" "$ROOT/apps/ecen21/tests/flipflop_checks.c" \
  "$ROOT/apps/ecen21/src/sequential.c" "$ROOT/apps/ecen21/tests/sequential_checks.c" \
  "$ROOT/apps/ecen21/src/seq_connections.c" "$ROOT/apps/ecen21/tests/connection_checks.c" \
  "$ROOT/apps/ecen21/src/timing.c" "$ROOT/apps/ecen21/src/timing_samples.c" \
  "$ROOT/apps/ecen21/tests/timing_checks.c" "$ROOT/apps/ecen21/tests/timing_sample_checks.c" \
  "$ROOT/apps/ecen21/src/combinational.c" "$ROOT/apps/ecen21/tests/combinational_checks.c" \
  "$ROOT/apps/ecen21/src/hdl.c" "$ROOT/apps/ecen21/src/hdl_export.c" "$ROOT/apps/ecen21/tests/hdl_checks.c" \
  "$ROOT/apps/ecen21/tests/controller_checks.c" "$ROOT/apps/ecen21/tests/host_main.c" \
  -o "$OUT/controller-tests"
"$OUT/controller-tests" | tee "$OUT/results.log"
