# MilkyTrackerNX2 — PORT NOTES

**What:** MilkyTrackerNX (Switch port of the MilkyTracker music tracker) ported
to **Switch 2** on top of the libounce runtime + OunceVK graphics stack.
**Upstream:** https://github.com/retronx-team/MilkyTrackerNX (Switch port by
p-sam), originally https://github.com/milkytracker/MilkyTracker (Peter "pailes"
Barth and contributors). This tree is a clone of MilkyTrackerNX edited in place.
**Port creator:** Lucaspec72. **AI assistance:** the porting work, build tooling
and documentation were created with AI assistance (OpenCode / GLM-5.3).

## Identity

- Homebrew tile / app title: **MilkyTrackerNX2**
- Author (NACP): **Lucaspec72** (the original authors stay credited in
  `AUTHORS` and in the About screen)
- Version: `1.0.0-nx2.0` (the NX2 line is independent from upstream)
- Install dir: `sdmc:/switch/MilkyTrackerNX2/`

## Build

```powershell
.\portnx2.ps1 build      # -> build_nx2/MilkyTrackerNX2.nro
.\portnx2.ps1 clean
.\portnx2.ps1 shell
```

Requirements: Docker Desktop running + the Switch 2 devkit drop
(`ounce-devkit-<date>` with `libounce/`, `portlibs/switch2/` and
`switch2vars.sh`) — found automatically in this folder or any parent, or via
`$env:SWITCH2_DEVKIT`. The drop is mounted read-only, never modified.

## Port record

_(filled in during porting — see the build log below.)_

## Build status log

- Pending initial Switch 2 build.
