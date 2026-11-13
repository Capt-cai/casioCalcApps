#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p build
cc=${CC:-clang}
"$cc" -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fstack-usage \
  exact_boolean.c test_exact_boolean.c -o build/test_exact_boolean
./build/test_exact_boolean | tee build/test-results.log
