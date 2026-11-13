#!/usr/bin/env bash
# Custom-width clean builds; preserves M5 engineering and accepted M4/M3/M2B/M2A and Phase 1 evidence.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WORK="$ROOT/.phase1/ecen21-width"
IMAGE="manawyrm/fxsdk@sha256:292176e823e272b87b74f3e87b0c2ee26d4ada17abec017e98002c207a456227"
mkdir -p "$WORK/logs" "$WORK/artifacts"
rm -f "$WORK/artifacts/ECEN21.g3a" "$WORK/artifacts/EC21Test.g3a" "$WORK/artifacts/SHA256SUMS" "$WORK/artifacts/ECEN21.elf" "$WORK/verification.json" "$WORK/artifacts/EC21Bench.g3a"
python3 - "$ROOT" <<'PY'
from pathlib import Path
import hashlib, json, shutil, sys
root = Path(sys.argv[1]); work = root/'.phase1/ecen21-width'; dest = work/'build-source'
if dest.exists(): shutil.rmtree(dest)
for relative in ['apps/ecen21', 'common', 'experiments/boolean']:
    shutil.copytree(root/relative, dest/relative, ignore=shutil.ignore_patterns(
        'build', 'build-*', '*.g3a', '*.g1a', '__pycache__'))
files = sorted(p for p in dest.rglob('*') if p.is_file())
manifest = {str(p.relative_to(dest)): hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
(work/'source-manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')
assert not list(dest.rglob('*.g3a'))
print('Prepared fresh source copy:', dest)
PY
# Reuse the verified cached image. A first setup may need an explicit docker pull.
docker image inspect "$IMAGE" > "$WORK/image-inspect.json"
docker run --rm --platform linux/arm64 --network none --cap-drop ALL \
  --security-opt no-new-privileges --read-only --tmpfs /tmp \
  --mount "type=bind,source=$WORK,target=/work" \
  --workdir /work/build-source/apps/ecen21 "$IMAGE" sh -c '
    set -eu
    # Build on the Linux tmpfs to avoid macOS bind-mount clock skew in Make.
    cp -R /work/build-source /tmp/ecen21-width-source
    cd /tmp/ecen21-width-source/apps/ecen21
    uname -a
    fxsdk --version
    sh-elf-gcc --version
    fxsdk build-cg
    cp ECEN21.g3a /work/artifacts/ECEN21.g3a
    cp build-cg/ECEN21 /work/artifacts/ECEN21.elf
    fxgxa -d ECEN21.g3a > /work/logs/production-header.log
    sh-elf-readelf -h build-cg/ECEN21 > /work/logs/production-elf.log
    sh-elf-size build-cg/ECEN21 > /work/logs/production-size.log
    find build-cg -name "*.su" -exec cat {} \; > /work/logs/production-stack.log
    fxsdk build-cg -c -DECEN21_TEST_BUILD=ON
    fxsdk build-cg
    cp EC21Test.g3a /work/artifacts/EC21Test.g3a
    fxgxa -d EC21Test.g3a > /work/logs/target-test-header.log
    fxsdk build-cg -c -DECEN21_TEST_BUILD=OFF -DECEN21_BENCH_BUILD=ON
    fxsdk build-cg
    cp EC21Bench.g3a /work/artifacts/EC21Bench.g3a
    fxgxa -d EC21Bench.g3a > /work/logs/benchmark-header.log
  ' > "$WORK/logs/build.log" 2>&1
shasum -a 256 "$WORK/artifacts/ECEN21.g3a" "$WORK/artifacts/EC21Test.g3a" "$WORK/artifacts/EC21Bench.g3a" \
  | tee "$WORK/artifacts/SHA256SUMS"
