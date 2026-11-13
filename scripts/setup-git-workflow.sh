#!/usr/bin/env bash
# Run once after clone; repository-local hooks/config only. No credentials stored.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
git config --local core.hooksPath .githooks
git config --local pull.ff only
git config --local push.default simple
if ! git remote get-url upstream >/dev/null 2>&1; then
  git remote add upstream https://github.com/joserodpt/casioCalcApps.git
fi
git config --local remote.upstream.pushurl DISABLED
printf '%s\n' 'Installed repository-local hooks; pull is fast-forward only; upstream push disabled.'
