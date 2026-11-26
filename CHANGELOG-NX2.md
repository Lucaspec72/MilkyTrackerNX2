# MilkyTrackerNX2 — changelog

Unofficial Switch 2 port of [MilkyTrackerNX](https://github.com/retronx-team/MilkyTrackerNX)
(the Switch port of [MilkyTracker](https://github.com/milkytracker/MilkyTracker)).
Ported by **Lucaspec72** with AI assistance. Version numbering is independent
from upstream and starts at 1.0.0.

## v1.0.0 (2026-11-26)

Initial release of the Switch 2 port:

- Builds `MilkyTrackerNX2.nro` with the libounce / OunceVK Switch 2 toolchain
  (CMake toolchain `Switch2.cmake`, `switch2vars.sh`, cortex-a78c), keeping the
  original MilkyTracker/MilkyTrackerNX application code and controls.
- Rebranded app identity: title **MilkyTrackerNX2**, author **Lucaspec72**,
  version `1.0.0-nx2.0`, separate `sdmc:/switch/MilkyTrackerNX2/` install dir.
- Keeps the original author credits (Peter "pailes" Barth, the MilkyTracker
  contributors and p-sam's original Switch port) in `AUTHORS` and the in-app
  About screen.
- Added `portnx2.ps1` build wrapper, `PORT_NOTES.md` and this changelog;
  README files now describe the Switch 2 fork and its AI-assisted, unofficial
  status.
