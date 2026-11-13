#!/usr/bin/env bash
# Fresh-build unchanged Logic in a pinned ARM64 Linux fxSDK container.
# Requires running Docker Desktop. All generated files stay in .phase1/.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WORK="$ROOT/.phase1/toolchain"
IMAGE="manawyrm/fxsdk@sha256:292176e823e272b87b74f3e87b0c2ee26d4ada17abec017e98002c207a456227"
mkdir -p "$WORK/logs" "$WORK/artifacts"
rm -f "$WORK/artifacts/Logic.g3a" "$WORK/artifacts/SHA256SUMS"
python3 - "$ROOT" <<'PY'
from pathlib import Path
import hashlib, json, shutil, sys
root = Path(sys.argv[1])
work = root / '.phase1/toolchain'
dest = work / 'build-source'
if dest.exists():
    shutil.rmtree(dest)
for relative in ['apps/logic', 'common']:
    shutil.copytree(root/relative, dest/relative,
        ignore=shutil.ignore_patterns('build-*', '*.g3a', '*.g1a', '__pycache__'))
(dest/'scripts').mkdir()
shutil.copy2(root/'scripts/g3a-name.py', dest/'scripts/g3a-name.py')
files = sorted(p for p in dest.rglob('*') if p.is_file())
manifest = {str(p.relative_to(dest)): hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
(work/'source-manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')
assert not list(dest.rglob('*.g3a'))
print('Prepared fresh source copy without any .g3a files:', dest)
PY
docker pull --platform linux/arm64 "$IMAGE" > "$WORK/logs/docker-pull-arm64.log" 2>&1
docker image inspect "$IMAGE" > "$WORK/image-inspect-arm64.json"
docker run --rm --platform linux/arm64 --network none --cap-drop ALL \
  --security-opt no-new-privileges --read-only --tmpfs /tmp \
  --mount "type=bind,source=$WORK,target=/work" \
  --workdir /work/build-source/apps/logic "$IMAGE" sh -c '
    set -eu
    uname -a
    cmake --version
    fxsdk --version
    sh-elf-gcc --version
    sh-elf-as --version
    sh-elf-gcc -dumpmachine
    python3 --version
    fxsdk path sysroot
    for repo in /root/.local/share/giteapc/Lephenixnoir/fxsdk /root/.local/share/giteapc/Lephenixnoir/gint /root/.local/share/giteapc/Vhex-Kernel-Core/fxlibc /root/.local/share/giteapc/Lephenixnoir/OpenLibm; do
      if test -d "$repo/.git"; then
        printf "%s\n" "$repo"
        git -C "$repo" log -1 --format="%H %s"
      fi
    done
    fxsdk build-cg
    test -s Logic.g3a
    cp Logic.g3a /work/artifacts/Logic.g3a
    fxgxa -d Logic.g3a > /work/logs/logic-g3a-header.log
    sed -n "1,18p" /work/logs/logic-g3a-header.log
  ' > "$WORK/logs/logic-container-build.log" 2>&1
shasum -a 256 "$WORK/artifacts/Logic.g3a" | tee "$WORK/artifacts/SHA256SUMS"
echo "Fresh artifact: $WORK/artifacts/Logic.g3a"
echo "Build log: $WORK/logs/logic-container-build.log"
