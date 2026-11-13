#!/usr/bin/env bash
# Current source builds into M6; earlier milestone helpers are archived.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
echo "Building current M6 source; earlier milestone evidence is preserved."
exec bash "$ROOT/scripts/toolchain-build-m6.sh" "$@"
