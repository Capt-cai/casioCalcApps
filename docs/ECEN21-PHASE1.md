# ECEN21 Toolkit: Phase 1 verification

**Historical snapshot of the initial inspection.** The source-build blocker described below was subsequently resolved using a pinned ARM64 Linux container. A fresh unchanged Logic build and independent fresh-binary emulator verification now pass. See [current progress](../PROGRESS.md), [toolchain evidence](TOOLCHAIN.md) and [testing results](../TESTING.md). Native macOS toolchain installation and physical calculator behaviour remain unverified. The original findings below are retained as the initial record.

Verified November 12, 2026, on macOS 26.6.2, Apple Silicon (arm64).

## Result

**A fresh Logic source build is blocked; the existing Logic binary runs successfully in a natively built emulator.** No ECEN21 features or Logic/shared application changes were made. The checked-in `dist/g3a/Logic.g3a` was preserved, not presented as a new build. A physical fx-CG50 has not been tested.

| Check | Actual result |
|---|---|
| `make app APP=logic` | Exit 2: `fxsdk not found`; no C compilation reached |
| Emulator configure/build | Successful, native Mach-O arm64 |
| Emulator CTest | 5/5 passed; fxSDK-dependent integration tests unavailable |
| Logic prebuilt scripted demo | Exit 0; all five screenshots produced |
| Demo screenshot comparison | All five decoded images exactly match checked-in reference pixels |
| Logic host core audit | 65,808 covers equivalent to their input truth tables; no ASan/UBSan diagnostics |
| Minimum-term audit | 2,288 of 65,536 four-variable functions have a nonminimal greedy cover |
| Physical calculator / fresh SuperH binary | Not verified |

## Repository provenance and architecture

Local commit: `12d6b4e1086746534636c3bb87703deeef5734a8`. The worktree was clean on entry. Local `origin` is `https://github.com/Capt-cai/casioCalcApps.git`; a read-only upstream HEAD query to `https://github.com/joserodpt/casioCalcApps.git` returned the same commit. No remote or branch was changed.

The repository is a collection of 16 independent C add-ins. Each `apps/<name>/` has application sources, CMake, icons and documentation; many also have screenshots and emulator key scripts. `template/` and `scripts/new-app.sh` provide scaffolding.

`Makefile` delegates to `scripts/build.sh`, which invokes `fxsdk build-cg` in each app and collects its `.g3a` into `dist/g3a/`. Apps explicitly marked `FX` also build a monochrome `.g1a`; Logic is colour only. `make clean` removes distribution binaries too, so it was not run.

`apps/logic/CMakeLists.txt` requires gint >=2.9, includes fxSDK asset/packaging modules, and calls `casio_app(Logic NAME_PT "Logica" SOURCES src/main.c)`. `common/common.cmake` builds `casio_common` as a static library, links `Gint::Gint`, converts the shared font/QR assets with fxconv, and generates the `.g3a` using fxgxa. A Python post-build step writes the Portuguese menu label; `fxgxa -r` repairs checksums afterward. Compile options are `-Wall -Wextra -Os`.

`apps/logic/src/main.c` contains all of Logic, with static functions and types:

- Recursive-descent parser: A–D, constants, parentheses, NOT, AND/juxtaposition, XOR and OR; evaluates 16 assignments.
- `model_t`: 40-byte expression buffer (39 characters plus NUL), cursor, 16-bit minterm set, active variable count 2–4, expression/manual mode.
- Prime implicant enumeration, essential implicants, then greedy cover selection; SOP formatting. No don't-care model, exact-cover search or POS solver exists.
- Three views: expression editor, editable truth table, Gray-code K-map with wraparound group drawing. It has a direct keyboard/event loop rather than a toolkit menu.

## Reuse assessment

| Component | Proposed use and limits |
|---|---|
| Logic parser/model/prime generation | Foundation for Boolean tools after a separately reviewed extraction; currently coupled to one translation unit |
| Logic table/K-map/editor | Existing interaction and layout reference for 2–4 variables; bounded expression/result display |
| `common/include/menu.h`, `common/src/menu.c` | Toolkit navigation; maximum seven entries per menu, so use categories/submenus |
| `utils`, `info`, `color` | F-key buttons, scrollbars, title/about screens, colour conventions |
| `keys`, `numfield`, `form`, `fmt` | Key mapping, numeric inputs, forms and formatting; `form_t` has eight fields maximum |
| `ui.h` / `ui.c` | Alternative symbol-aware text and richer `eform_t` fields; choose the suitable form API per screen |
| `i18n` | OS language detection, English/Portuguese strings |
| `usb_screen` | Optional screen streaming on hardware; no USB runtime validation here |
| Base-N application | Reuse candidates for 8/16/32/64-bit masking, signed/two's-complement interpretation, conversion formatting and bit-grid UI; these are static app functions, not a reusable arithmetic library yet |
| AC Circuit waveform rendering | Drawing reference for later timing-diagram presentation; analogue simulation is not a digital timing engine |
| Fonts/icons/CMake/template | Reuse build conventions and shared assets; create a distinct ECEN21 identity later |

Flip-flops, digital counters, sequential synthesis and Verilog tools are not implemented by Logic. Existing circuit simulation does not supply those digital engines.

The repository code uses the MIT license: preserve José Rodrigues's copyright and license notice when reusing it. Keep the existing DejaVu/Bitstream font attribution as well.

## Environment and dependency investigation

Initially present: Apple command-line tools, AppleClang 21.0.0.21000101, Make, Git, Python (system 3.9.6 and Homebrew 3.11.15), Homebrew. `/usr/bin/gcc` is Apple's compiler, not a SuperH cross-compiler. Docker CLI exists but its daemon was not running; no containers were started.

Installed successfully using Homebrew:

```sh
HOMEBREW_NO_AUTO_UPDATE=1 brew install cmake sdl3
```

Installed versions: CMake 4.3.4 and SDL3 3.4.10. Emulator CMake selected Homebrew Python 3.11.15. No shell profile changes were made.

Still missing: fxSDK (`fxsdk`, `fxconv`, `fxgxa`), SuperH `sh-elf` binutils/GCC, compatible fxlibc/OpenLibm and gint >=2.9 installed for fx-CG50. The repository does not pin these versions or provide an installer/container lockfile.

Official fxSDK/GiteaPC URLs on both `gitea.planet-casio.com` and `git.planet-casio.com` timed out with no response even after sandbox network access was granted. Initial sandbox-only access also failed DNS resolution. GitHub and Homebrew downloads worked. A GitHub SDK mirror's documentation was readable, but a mirror alone does not provide a complete verified compiler/library stack, so it was not installed as a substitute. A checked third-party compiler release URL returned 404; no compiler binary was downloaded or executed.

**Unresolved blocker:** acquire a complete, trusted SuperH toolchain and gint libraries. Native Apple Silicon support of that full stack remains unverified here. Once official hosting is reachable, use GiteaPC or the documented manual order: fxSDK host tools, binutils, bootstrap GCC, OpenLibm/fxlibc, final GCC runtime as required, then gint for fx-CG50. Pin the revisions that actually build. This is a proposed route, not a completed installation.

For macOS, fxSDK's Linux UDisks2 integration must be disabled when configuring that component; USB Flash copying is sufficient to install a resulting add-in. Asset conversion requires Python Pillow, packaging requires libpng; asset regeneration may additionally need qrcode/font dependencies, but existing tracked assets suffice for a normal Logic build. Do not install host GNU GCC expecting it to produce SuperH code.

## Build and test reproduction

From the repository root:

```sh
# Source-build attempt (currently fails before compilation):
make app APP=logic

# Emulator tests and the original Logic key script, with outputs isolated:
bash scripts/verify-logic-emulator.sh

# Host audit of source functions, independent of gint/fxSDK:
python3 scripts/verify-logic-core.py
```

The emulator helper checks out `joserodpt/emuCasioCalc` commit `63e325708e9eea53eb4769b9c057082614ebd919` under ignored `.phase1/vendor/`, builds in `.phase1/emulator-build/`, and saves logs/screenshots in `.phase1/`. CMake/SDL3 and native developer tools are prerequisites; the helper does not install system packages. When a source build becomes available, pass its absolute `.g3a` path as the script's first argument. If fxSDK is present, the emulator's upstream CMake may enable additional integration builds/tests.

Actual emulator build commands used:

```sh
cmake -S .phase1/vendor/emuCasioCalc -B .phase1/emulator-build
cmake --build .phase1/emulator-build -j 6
ctest --test-dir .phase1/emulator-build --output-on-failure
```

Available tests were `cpu`, `hello`, `hello_model`, `hello_cg20`, and `script`; all passed in 0.88 seconds on the initial run. These validate emulator infrastructure, not every Logic function. No application unit-test suite or CI workflow is tracked in casioCalcApps.

The original Logic demo was run headlessly with a fixed clock against **the checked-in prebuilt file**. Screenshot paths were redirected to `.phase1/logic-demo/` to preserve repository reference images. `main.png`, `expression.png`, `truth-table.png`, `kmap.png` and `kmap-edited.png` were generated, and decoded pixels compared with Pillow: all five matched exactly. Expression and K-map outputs were also visually inspected. The emulator emitted `script: 1 key left unused (the add-in ended first)` but returned 0 after normal add-in exit and all screenshots; this warning is retained in the log and is not a claim that every script event was consumed.

Prebuilt identity: 85,716 bytes; SHA-256:

```text
32cbf5839e212232b7038579d770d5ce784b4f17bad553ee7ff8f9bdf578aa7a
```

The host audit extracts the current parser/model/cover/SOP functions verbatim into generated C, and compiles with `clang -std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer`. It checks nine valid expressions, five invalid expressions, every possible 2-, 3- and 4-variable truth table, cover equivalence, and SOP round-trip equivalence. An independent dynamic program over all 81 four-variable cubes determines the minimum number of terms. No 80-byte SOP truncation occurred over this input domain. Findings are printed; exit 0 denotes completed checks, not mathematical optimality.

Source SHA-256 audited:

```text
2d1772c8e94c93de0d1931f07b711358fb00510f81c8d13774dcc5d8cd9bd093
```

Example nonminimal cover: `minterms=0x007e` (rows 1–6), four variables; existing greedy output has four terms while three suffice. This limitation is documented, not fixed in Phase 1. The audit checks term count only, not minimum literal count. Host extraction does not verify SH ABI, linking, display, input, USB or execution time on a calculator.

The emulator built with four nonfatal unused-function warnings in platform-specific helpers: `mount_device`, `parse_ohp`, `parse_fxlink` and `make_dirs`. No Logic compiler warnings can be reported because the source build never reached compilation. Emulation cannot certify physical OS compatibility or USB behaviour.

Logs: `.phase1/logs/logic-source-build.log`, `emulator-configure.log`, `emulator-build.log`, `emulator-ctest.log`, `logic-prebuilt-demo.log`, `logic-core-check.log`, `sdk-network.log` and `sdk-network-gitea.log`. The reproduction helper additionally writes `logic-demo.log`. Local artifacts are ignored to avoid committing the emulator checkout and generated files.

## Recommended ECEN21 architecture and next task

Create a separate `apps/ecen21/` using `casio_app(...)` and the shared UI library. Preserve `apps/logic/` as the verified upstream reference. Use a small app entry point and category menus around pure C domain modules: Boolean expressions/truth tables/minimization, fixed-width binary arithmetic, sequential models/flip-flops/counters, and Verilog text generation. Keep timing-diagram drawing and all gint input/display calls in view/controller modules, separate from those computational models. Use explicit bounded data structures appropriate to calculator memory and screen limits; validate expanded limits before lifting existing four-variable assumptions.

Initially keep the toolkit's domain modules app-local. Promote code into a shared domain library only when another app actually needs it. Reuse the existing algorithms and conventions through a reviewed extraction, with host tests plus emulator scripts guarding behaviour. Add an exact-cover stage later if course requirements promise mathematically minimal SOP; add don't-cares and POS only as explicit subsequent work.

**Next concrete task:** restore official SDK host access (or obtain an approved complete toolchain source), install and pin the fx-CG50 toolchain, build unchanged Logic, then rerun the demo against that newly generated binary. Only after that source-build gate should the first ECEN21 development task be extracting the existing Boolean core into the separate app with behaviour-preserving tests. A final physical fx-CG50 smoke test will need the user's calculator: launch, edit an expression, toggle a truth-table row, inspect its K-map, and exit normally.

## References

- [Upstream application repository](https://github.com/joserodpt/casioCalcApps)
- [Emulator and its build/test instructions](https://github.com/joserodpt/emuCasioCalc)
- [Official fxSDK](https://gitea.planet-casio.com/Lephenixnoir/fxsdk)
- [Official GiteaPC](https://gitea.planet-casio.com/Lephenixnoir/GiteaPC)
- [SDK mirror documentation inspected during the outage](https://github.com/peltlasvulpine/fxsdk-mirror)
