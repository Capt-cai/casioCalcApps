#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
OUT="$ROOT/.phase1/ecen21-width/host-tests"
mkdir -p "$OUT"
"${CC:-clang}" -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer -DNM_FULL_HOST_TESTS \
  -I"$ROOT/apps/ecen21/src" "$ROOT/apps/ecen21/src/numeric.c" \
  "$ROOT/apps/ecen21/tests/numeric_checks.c" "$ROOT/apps/ecen21/tests/numeric_exhaustive_main.c" \
  -o "$OUT/numeric-exhaustive"
"$OUT/numeric-exhaustive" | tee "$OUT/numeric-exhaustive.log"
