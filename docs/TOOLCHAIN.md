# Verified fx-CG50 build toolchain on Apple Silicon

Verified November 12, 2026. **A fresh build of unchanged Logic succeeded and passed the native macOS emulator demo.** The working compiler environment is a pinned **ARM64 Linux container in Docker Desktop**, not a native macOS installation of fxSDK/GCC. The calculator output remains native SuperH code.

## Working route

Docker Desktop was already installed and was started with `open -a Docker`. Its Linux VM is aarch64. The complete image documented by [Manawyrm/fxsdk-docker](https://github.com/Manawyrm/fxsdk-docker) has an ARM64 variant, despite its README's older amd64-only recommendation. The repository Dockerfile builds the compiler and libraries from upstream using GiteaPC. The inspected repository HEAD was `9ff264b8f8910bec3ff086e7524730ef65ebd433`.

The exact ARM64 image used is:

```text
manawyrm/fxsdk@sha256:292176e823e272b87b74f3e87b0c2ee26d4ada17abec017e98002c207a456227
```

Image inspection reports `Architecture=arm64`, `Os=linux`, creation `2025-02-19T13:38:25.324999136Z`, and size 2,301,723,869 bytes. Pinning the platform manifest avoids a mutable `latest` tag and x86 translation. The third-party source recipe was inspected and embedded source Git revisions recorded; no independent signature/attestation or bit-for-bit reproduction of the image itself was verified.

| Component | Verified version/revision inside image |
|---|---|
| fxSDK | 2.11.0; `09e2cf5fdab8d529217203a9a97a93facaa57311` |
| GNU GCC | 14.1.0; `sh-elf-gcc`, configured target `sh3eb-elf` |
| GNU binutils | 2.42 |
| gint | 2.11.0; `badbd0fd2bd8ac796fd55d49b93691741bd8a139` |
| fxlibc | 1.5.1; `e2f458fa3d88d11b0685ae66d267730980eb65ca` |
| OpenLibm | `9828d2e3f265a882af6a5bba8ffe6a00c3eeb8d7` |
| CMake | 3.25.1 |
| Python | 3.11.2 |

The container supplies fxconv/Pillow, fxgxa, compiler runtime, standard C library, math library and gint, so a build does not need the currently unreachable forge.

## Reproduce the clean source build

From the repository root, with Docker Desktop running:

```sh
bash scripts/toolchain-build-logic.sh
/opt/homebrew/bin/python3.11 scripts/toolchain-verify-logic.py
```

The verification command uses the existing Phase 1 native emulator at `.phase1/emulator-build/emucasiocalc` and host Pillow. If rebuilding that emulator is necessary, use the repository's `scripts/verify-logic-emulator.sh` first. The explicit Python path above is the working Homebrew interpreter on this Mac; another Python >=3.8 with Pillow can run the script too.

The clean-build helper:

1. Removes stale output `artifacts/Logic.g3a` and its checksum file.
2. Recreates `.phase1/toolchain/build-source/` from the current `apps/logic/`, `common/` and `scripts/g3a-name.py`, excluding all `.g3a`, `.g1a`, `build-*` and Python cache files.
3. Records SHA-256 hashes of all copied inputs in `source-manifest.json`. The final copy contained 49 files, independently compared by the lead engineer with the workspace inputs.
4. Runs `fxsdk build-cg` in the isolated copy with the pinned ARM64 image. The container has no network, a read-only root filesystem, no Linux capabilities, and only `.phase1/toolchain/` mounted writable. It has no host home, Docker socket, USB or privileged access.
5. Preserves the resulting `.g3a`, full compiler log and fxgxa header/checksum dump under `.phase1/toolchain/`.

Application/common source files and `dist/g3a/Logic.g3a` were not modified. The top-level `make app APP=logic` still needs fxSDK on the macOS host; the container helper is the verified build command for this setup.

## Actual fresh-build results

The final ARM64 container build compiled every shared C module, `apps/logic/src/main.c`, and both converted assets; it linked and packaged successfully with **no compiler warnings or errors**. CMake found gint 2.11.0, satisfying the application's >=2.9 requirement. Portuguese name patching and checksum repair also succeeded. fxgxa reported all signature/control/size/checksum/footer checks `OK`.

Target compile flags include:

```text
-m4-nofpu -mb -ffreestanding -nostdlib -Wa,--dsp
-Wall -Wextra -Os -fstrict-volatile-bitfields
```

`sh-elf-readelf -h build-cg/Logic` independently identified an ELF32 big-endian executable, `Machine: Renesas / SuperH SH`, entry point `0x300000`, and `Flags: 0x6, sh4al-dsp`. This distinguishes the calculator executable from the ARM64 Linux compiler host and ARM64 macOS emulator host.

Fresh deliverable:

```text
.phase1/toolchain/artifacts/Logic.g3a
85,716 bytes
SHA-256 100b83beaa5dd6051cebdecfb816df8946d60bd6afa66ea884691ca56fd98b25
```

Builds include date/time metadata, so another successful clean build can have a different file hash. The generated ELF, objects and CMake configuration remain in the isolated source copy for inspection; they were absent before compilation.

The final fresh binary ran in the existing native Mach-O arm64 emulator at the Phase 1 pinned emulator revision `63e325708e9eea53eb4769b9c057082614ebd919`. The original Logic demo completed with exit 0. All five images (`main`, `expression`, `truth-table`, `kmap`, `kmap-edited`) matched the tracked reference screenshots exactly after decoding to RGB pixels. The known diagnostic `script: 1 key left unused (the add-in ended first)` still occurs after all screenshots and normal application exit; the log preserves it. This does not establish that every trailing script event was consumed.

An initial clean build also succeeded using the amd64 platform manifest `sha256:95992227534aafd37904420aed135c24a12d3a47c589b6b44093dd1554dbc78d`, before ARM64 finished downloading. That artifact is retained as `artifacts/Logic-amd64.g3a`, SHA-256 `085d8593a729ecf20b6d993d5af5c4caeee18fd75c79b1a7bf57b6eac14109f3`. Its emulator demo also passed and the lead independently confirmed all five pixel matches. Prefer ARM64 for future builds. Both image variants remain in Docker's local cache; no existing containers/images were deleted.

## Native macOS investigation and remaining limits

The official forge is reachable at DNS/TCP/TLS level, with a valid TLS certificate, but it sends no HTTP response for the SDK/runtime/Git endpoints tested here. HTTPS repository, raw-file, archive and API requests timed out; `git clone` failed its 15-second low-speed timeout. The older `gitea.planet-casio.com` host redirects to `git.planet-casio.com`, which has the same problem. HTTP redirects to HTTPS. Logs preserve these results; no TLS verification bypass was attempted.

GitHub, Docker Hub and the official GNU archive server worked. Source mirrors inspected were fxSDK 2.11.0 (`a6e46eb6df060dc4e6e931521205e516c92b6c7f`) and gint 2.11.0 (`2bc6f7ebcb500b423ee9906e11664466ae913424`) under `peltlasvulpine`; an old Epitech fxlibc mirror dates to 2020 and is not a compatible substitute for current gint. None of those downloaded mirror sources was compiled or installed. GNU GCC 14.3.0 archive access was checked successfully, but a standalone compiler bootstrap was unnecessary after the complete container route worked.

For a future full native installation, the upstream-supported source route is GiteaPC/manual fxSDK, binutils, bootstrap GCC, OpenLibm/fxlibc, final compiler runtime as required, then gint. On macOS, disable Linux UDisks2 support (`fxsdk:noudisks2` or `FXLINK_DISABLE_UDISKS2`) and supply the normal CMake/libpng/Pillow/libusb/ncurses/GMP/MPFR/MPC/texinfo build dependencies. The [author's forum installation thread](https://www.planet-casio.com/Fr/forums/topic16614-15-giteapc-installer-et-mettre-a-jour-automatiquement-des-projets-gitea.html) documents macOS dependency notes and an Apple Silicon compiler race for which serial compilation is a workaround. That native route remains **unverified on this machine**, rather than being a prerequisite for the now-working container source build.

No new Homebrew packages or shell-profile changes were needed in this follow-up; CMake/SDL3/Python from Phase 1 remain installed. Docker Desktop was started and left running.

Physical fx-CG50 execution, its OS return behaviour and USB streaming still require hardware verification. Emulation and header checks do not certify those behaviours.

## Evidence files

All generated evidence is ignored under `.phase1/toolchain/`:

- `artifacts/Logic.g3a`, `artifacts/SHA256SUMS`
- `source-manifest.json`, `image-manifest.json`, `image-inspect-arm64.json`
- `logs/logic-container-build.log`, `logs/logic-g3a-header.log`, `logs/logic-elf-header.log`
- `logs/logic-fresh-emulator.log`, `logs/logic-fresh-verification.log`, `verification.json`
- `verification/*.png` and redirected `verification/demo.script`
- `logs/docker-pull-arm64.log`, initial `logs/logic-amd64-build.log`, `logs/logic-amd64-emulator.log`
- `logs/forge-verbose.log`, `logs/fxlibc-clone.log`

The source-build blocker is resolved through the pinned ARM64 container route. A fully native macOS toolchain and physical calculator smoke test remain separate outstanding work.

## M8 host Verilog verification dependency

M8 authorization required an actual HDL compiler/simulator. Installed native
Apple Silicon Homebrew `icarus-verilog`13.0 (`/opt/homebrew/Cellar/icarus-verilog/13.0`),
using the downloaded ARM64 bottle. No native macOS fxSDK/compiler installation,
Docker image change or shell-profile change was made. Existing fxSDK remains the
pinned ARM64 image above. `iverilog -g2001` and `vvp` compile/run real emitted code;
exact command forms and language options are documented by
[official Icarus flags](https://steveicarus.github.io/iverilog/usage/command_line_flags.html).
Actual version and test logs are `.phase1/ecen21-m8/logs/iverilog-version.log`,
`hdl-verification/` and `hdl-export-verification/`. Reproduce via
`bash apps/ecen21/tests/run-hdl.sh` after installing that host-only dependency.
Earlier statements about no new dependencies apply to their historical phases.
