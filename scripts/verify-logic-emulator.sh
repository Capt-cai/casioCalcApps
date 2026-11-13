#!/usr/bin/env bash
# Run against the checked-in binary by default; pass a fresh .g3a to test it.
# Dependencies: git, cmake >=3.20, SDL3, Python 3, Apple command-line tools.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
EMU_COMMIT=63e325708e9eea53eb4769b9c057082614ebd919
EMU_SRC="$ROOT/.phase1/vendor/emuCasioCalc"
EMU_BUILD="$ROOT/.phase1/emulator-build"
mkdir -p .phase1/logs .phase1/logic-demo .phase1/vendor
if [[ ! -d "$EMU_SRC/.git" ]]; then
  git clone https://github.com/joserodpt/emuCasioCalc.git "$EMU_SRC"
  git -C "$EMU_SRC" checkout --detach "$EMU_COMMIT"
fi
actual=$(git -C "$EMU_SRC" rev-parse HEAD)
[[ "$actual" == "$EMU_COMMIT" ]] || { echo "Unexpected emulator revision: $actual" >&2; exit 1; }
cmake -S "$EMU_SRC" -B "$EMU_BUILD" > .phase1/logs/emulator-configure.log 2>&1
cmake --build "$EMU_BUILD" -j 6 > .phase1/logs/emulator-build.log 2>&1
ctest --test-dir "$EMU_BUILD" --output-on-failure | tee .phase1/logs/emulator-ctest.log
python3 - "$ROOT" <<'PY'
from pathlib import Path
import sys
root = Path(sys.argv[1])
lines = (root/'apps/logic/screenshots/demo.script').read_text().splitlines()
(root/'.phase1/logic-demo/demo.script').write_text('\n'.join(
    'shot '+str(root/'.phase1/logic-demo'/line[5:]) if line.startswith('shot ') else line
    for line in lines)+'\n')
PY
addin="${1:-$ROOT/dist/g3a/Logic.g3a}"
echo "Testing $addin (default is a prebuilt binary, not a source build)"
"$EMU_BUILD/emucasiocalc" --headless --clock '2026-01-01 12:00' \
  --script .phase1/logic-demo/demo.script "$addin" 2>&1 | tee .phase1/logs/logic-demo.log
for name in main expression truth-table kmap kmap-edited; do
  test -s ".phase1/logic-demo/$name.png"
done
