# MilkyTrackerNX2

**Unofficial Nintendo Switch 2 port of MilkyTrackerNX, the Switch port of
MilkyTracker.** Ported by **Lucaspec72** on top of the original MilkyTracker by
**Peter "pailes" Barth** and the MilkyTracker contributors, and the original
Switch port by **p-sam** ([retronx-team/MilkyTrackerNX][milkytrackernx]). This
fork targets the Switch 2 (libounce / OunceVK toolchain) so the same module
tracker runs on the newer console.

> [!WARNING]
> **AI-assisted, unofficial port.** The Switch 2 porting work, build tooling
> and documentation were created with AI assistance (OpenCode). It is unofficial — not affiliated with or endorsed by the original
> authors. see
> [`PORT_NOTES.md`](PORT_NOTES.md) for the technical record and open risks. Use
> at your own risk. For the supported experience, use the original
> [MilkyTracker][milkytracker] or [MilkyTrackerNX][milkytrackernx].

## MilkyTrackerNX2 — quick start

Build (requires Docker Desktop and a `ounce-devkit-<date>` drop in this folder
or any parent, or `$env:SWITCH2_DEVKIT`):

```powershell
.\portnx2.ps1 build    # -> build_nx2/MilkyTrackerNX2.nro
.\portnx2.ps1 clean
.\portnx2.ps1 shell
```

Copy `MilkyTrackerNX2.nro` to `sdmc:/switch/MilkyTrackerNX2/` and launch it
from the homebrew menu. The Switch 2-specific controls and caveats are in
[`platforms/nx/README.md`](platforms/nx/README.md). Version history for this
fork is in [`CHANGELOG-NX2.md`](CHANGELOG-NX2.md).

---

# Original MilkyTracker / MilkyTrackerNX readme

MilkyTracker - Cross-Platform XM Tracker
========================================

[![Travis Build Status](https://travis-ci.org/milkytracker/MilkyTracker.svg?branch=master)](https://travis-ci.org/milkytracker/MilkyTracker)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/milkytracker/MilkyTracker?branch=master&svg=true)](https://ci.appveyor.com/project/Deltafire/milkytracker)

MilkyTracker is an multi-platform music application for creating .MOD
and .XM module files. It attempts to recreate the module replay and
user experience of the popular DOS program Fasttracker II, with
special playback modes available for improved Amiga ProTracker 2/3
compatibility.

Refer to http://milkytracker.titandemo.org/?about for further details.

Please read the file [INSTALL.md][] for installation instructions.

The [docs/readme_unix][] file contains notes specific to the SDL port
of MilkyTracker.

Refer to [platforms/nx/README.md][] for info specific to the Switch (and
Switch 2) port of MilkyTracker.

[INSTALL.md]:INSTALL.md
[docs/readme_unix]:docs/readme_unix
[platforms/nx/README.md]:platforms/nx/README.md
[milkytracker]:https://github.com/milkytracker/MilkyTracker
[milkytrackernx]:https://github.com/retronx-team/MilkyTrackerNX
