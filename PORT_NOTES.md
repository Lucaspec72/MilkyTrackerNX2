# MilkyTrackerNX2 — PORT NOTES

**What:** MilkyTrackerNX (Switch port of the MilkyTracker music tracker) ported
to **Switch 2** on top of the libounce runtime + OunceVK graphics stack.
**Upstream:** https://github.com/retronx-team/MilkyTrackerNX (Switch port by
p-sam), originally https://github.com/milkytracker/MilkyTracker (Peter "pailes"
Barth and contributors). This tree is a clone of MilkyTrackerNX edited in place.
**Port creator:** Lucaspec72. **AI assistance:** the porting work, build tooling
and documentation were created with AI assistance (OpenCode / GLM-5.3).
Unofficial — not affiliated with or endorsed by the original authors.

## Identity

- Homebrew tile / app title: **MilkyTrackerNX2**
- Author (NACP): **Lucaspec72** (the original authors stay credited in
  `AUTHORS`, the About screen and the platform README)
- Version: `1.0.0` (NACP), `1.0.0-nx2.0` in `platforms/nx/_env.sh`; the NX2
  line is independent from upstream's 1.02.00 / 1.2.0-nx6
- Install dir: `sdmc:/switch/MilkyTrackerNX2/`
- In-app: SDL window title "MilkyTrackerNX2", loading banner
  "Loading MilkyTrackerNX2...", About-screen credit line under `__SWITCH2__`.
  The upstream `MILKYTRACKER_VERSION_STRING` (and the underlying 1.02.00
  version header) are deliberately left alone — they are written into exported
  XM module files and have a 20-character limit.

## Build

```powershell
.\portnx2.ps1 build      # -> build_nx2/MilkyTrackerNX2.nro
.\portnx2.ps1 clean
.\portnx2.ps1 shell
.\portnx2.ps1 run "<bash command>"
```

Requirements: Docker Desktop running + the Switch 2 devkit drop
(`ounce-devkit-<date>` with `libounce/`, `portlibs/switch2/` and
`switch2vars.sh`) — found automatically in this folder or any parent, or via
`$env:SWITCH2_DEVKIT`. The drop is mounted read-only, never modified.

`portnx2.ps1` resolves this repo's directory symlink (the checkout lives in
`~/Documents/GitHub`) to its real path before mounting it at
`/work/MilkyTrackerNX2`: Docker for Windows cannot follow a Windows symlink
inside a bind mount. `platforms/nx/build_ns2.sh` runs inside the container.

## Port record

The port is deliberately mechanical: all MilkyTracker engine/UI code is
unchanged except identity strings and a touch-input fix. `__SWITCH__` is still
defined by libounce's toolchain, so every existing Switch code path in
`src/ppui`, `src/tracker` and `platforms/nx/nxhooks` compiles as-is;
`__SWITCH2__` gates only the new bits.

| Area | File(s) | Change |
|:--|:--|:--|
| Build system | `CMakeLists.txt` | `cmake_minimum_required` 2.8.5 → 3.13 (the Switch 2 toolchain requires 3.13; CMake 3.31 rejects older compatibility levels); added a `NINTENDO_SWITCH2` block with `nx_generate_nacp` (title/author/version) and `nx_create_nro` for `MilkyTrackerNX2.nro`, using `platforms/nx/icon.jpg` |
| Build wrapper | `portnx2.ps1`, `platforms/nx/build_ns2.sh` (new) | devkit-drop discovery + read-only mounts; CMake via `aarch64-none-elf-cmake` with libounce's `Switch2.cmake`; `ZZIPLIB_LIBRARIES` pointed at the Switch 1 `libzzip.a`; builds target `MilkyTrackerNX2-nro` |
| Touch input | `platforms/nx/nxhooks/src/init.c`, new `src/touch.c` + `include/nxhooks/touch.h`, `src/sdl/input.c` | under `__SWITCH2__`, call `hidSetTouchScreenOutputRanges(1920, 1080)` before SDL/HID touch init. Fix the 720p-only touch bug: SDL's Switch touch backend (confirmed in `SDL_switchtouch.o`) normalizes raw coords by a hardcoded 1280x720, so its touch→mouse events only cover the top-left ~2/3 of the 1080p window. `nxHooksTouchPoll()` reads `hidGetTouchScreenStates()` (already in the 1920x1080 output range) and emits absolute SDL mouse motion/press/release on its own mouse ID. SDL's own touch→mouse generation is switched off **after** `SDL_Init` with `SDL_SetHintWithPriority(..., "0", SDL_HINT_OVERRIDE)` — the backend sets the hint itself at init, and a plain pre-init `SDL_SetHint("0")` loses. This is essential because MilkyTracker coalesces motion with `SDL_PeepEvents()`, which bypasses the poll-time filter: any leftover bogus event makes the cursor jump back to the 720p position. The stick cursor path is untouched. |
| Native Switch 2 mouse | `platforms/nx/nxhooks/src/nmouse.c`, `include/nxhooks/nmouse.h` (new), `src/sdl/input.c`, `src/init.c`, `CMakeLists.txt` | new `__SWITCH2__`-only module using libounce's `hidInitializeNmouse` / merged handle (`hidSetMergedNmouseSourceTypes` = Joy-Con L/R + standard mouse) to read 1920x1080 pointer samples and synthesize absolute SDL mouse motion, left/right/middle/X1/X2 buttons and wheel events on a dedicated mouse ID. Polled once per frame from the existing event hook. `hidFinalizeNmouse()` on exit. Stick, touch and native pointer each use a distinct SDL mouse ID so they coexist without interfering. |
| Identity | `src/ppui/sdl/DisplayDevice_SDL.cpp`, `src/tracker/sdl/SDL_Main.cpp`, `src/tracker/AnimatedFXControl.cpp` | NX2 window/loading titles; About credit line for the port, keeping p-sam's original Switch-port credit |
| Metadata | `README.md`, `AUTHORS`, `NEWS`, `platforms/nx/README.md`, `CHANGELOG-NX2.md` (new) | NX2 branding, original-author credits retained, Lucaspec72 as port creator, AI-assistance note, usage/build docs |
| App identity env | `platforms/nx/_env.sh` | `APP_TITLE=MilkyTrackerNX2`, `APP_AUTHOR=Lucaspec72`, `APP_VERSION=1.0.0-nx2.0` (used by the platform scripts; CMake/NACP values live in `CMakeLists.txt`) |

### Dependencies and linking

All third-party libraries come from the Switch 1 portlibs already in the
devkitpro image: SDL2 (video/input/audio), zlib, zziplib, glad, mpg123,
modplug, vorbisidec and ogg. `-lEGL`, `-lglapi` and `-ldrm_nouveau` resolve to
OunceVK's real EGL library and its deliberate empty shadow archives, so the
old `nxhooks` link line works unchanged. The `dl` stub from `nxhooks` satisfies
`CMAKE_DL_LIBS`. `-lnx` is libounce's linker-script stub — `nm` confirms
mimalloc (`_mi_*`) and OunceVK (`eglGetDisplay`) are linked into the ELF.

The empty `src/submodules/*` directories mean **LHA decompression is
disabled** (no lhasa); GZIP and ZIP work. This matches the original NX build
recipe, which also did not build the submodules.

## Build status log

- **2026-11-26 (input update): BUILDS.** Rebuilt with the nmouse module, the
  HID-based full-resolution touch path and the restored stick cursor. `nm`
  shows `hidInitializeNmouse`, `hidGetMergedNmouseHandle`,
  `hidGetNmouseStatesWithDelta`, `hidGetTouchScreenStates`,
  `hidSetTouchScreenOutputRanges`, `nxHooksNmousePoll` and `nxHooksTouchPoll`
  linked, 0 undefined symbols. A first attempt that disabled SDL's touch hint
  and synthesized from `SDL_FINGER*` events broke both stick and touch; it was
  replaced by the current design: stop SDL's touch→mouse generation after
  `SDL_Init` with an OVERRIDE hint (the app peeks queued motion events directly,
  so filtering in the poll hook alone could not prevent cursor jumps), and
  supply full-resolution touch events from HID.
- **2026-11-26: BUILDS.** `build_nx2/MilkyTrackerNX2.nro` (~19.6 MB) from a
  clean configure. 219 build steps, no compile errors; only pre-existing
  upstream warnings (strncat bounds, missing return in ResamplerFactory).
  `aarch64-none-elf-nm -u` on the ELF shows **no undefined symbols**. NACP
  strings show "MilkyTrackerNX2" / "Lucaspec72".
- Hit and fixed: `nx_create_nro(MilkyTrackerNX2.nro ... OUTPUT
  MilkyTrackerNX2.nro)` makes ninja complain "multiple rules generate
  MilkyTrackerNX2.nro" — the NRO CMake target name must differ from both the
  executable target AND the output filename (now `MilkyTrackerNX2-nro`).

## Open risks / to verify on real hardware

- **On-console test not done**: the NRO builds and links, but has not been
  launched on a Switch 2 yet. Neither bundled emulator produced a visible
  window on this machine (see `TOOLBOX.txt` §8), so launching from hbmenu is
  the next milestone.
- **Audio**: MilkyTracker's SDL2 audio backend uses the Switch 1 SDL portlib
  (`audren`). libounce also offers the native Switch 2 `audu` service; whether
  `audren` works on current HOS is still an open workspace question. If audio
  is silent, the next step is an SDL2 audio path over `audu`.
- **Gameplay recording**: `nxHooksInit()` still calls
  `appletInitializeGamePlayRecording()` (present in libounce; errors are only
  logged to nxlink/stdout). Can be dropped for `__SWITCH2__` if it fails.
- **Graphics**: the GL ES 2.0 context is provided by Zink/NVK through OunceVK;
  GLES 3.1+ is supported, but visual parity on console is unverified.
- **Native pointer / touch**: the nmouse and full-screen touch paths compile
  and link but have not been exercised on hardware yet. If the Joy-Con
  pointer needs enabling in system settings, or `hidInitializeNmouse` fails
  because SDL already initialized HID, check the `[nxhooks]` messages over
  nxlink; the stick/touch fallback remains active when no merged pointer
  handle is available.
- **Config file**: the app still uses `~/.milkytracker_config` (shared with
  an installed MilkyTrackerNX) — deliberate, so settings/songs stay familiar.
- **Crash logs**: stdout/stderr go to nxlink; for on-console diagnosis add a
  `freopen("sdmc:/switch/MilkyTrackerNX2/log.txt", "w", stderr)` early in
  `main()` (see OunceVK README / TOOLBOX logging tip).
