#!/usr/bin/env bash
# Current source builds into M8; earlier milestone helpers are archived.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
echo "Building current M8 source; earlier milestone evidence is preserved."
exec bash "$ROOT/scripts/toolchain-build-m8.sh" "$@"
