# MilkyTrackerNX2 - build wrapper (run from anywhere; paths are relative to
# this file, which lives at the repo root).
#
# Builds MilkyTrackerNX2.nro (MilkyTrackerNX ported to Switch 2 / libounce)
# inside the devkitpro/devkita64 Docker image, with the local ounce-devkit
# drop overlaid read-only on libounce and portlibs/switch2. MilkyTracker's
# SDL2/audio/compression dependencies come from the Switch 1 portlibs that
# ship in the image (the NX port already used them).
#
# Usage:
#   .\portnx2.ps1 build       configure + build build_nx2/MilkyTrackerNX2.nro
#   .\portnx2.ps1 clean       delete the build dir
#   .\portnx2.ps1 shell       interactive bash in the build container
#   .\portnx2.ps1 run <bash>  arbitrary command (switch2vars.sh sourced)
#
# Requires:
#   - Docker Desktop running.
#   - The Switch 2 devkit drop (ounce-devkit-<date>, with libounce,
#     portlibs/switch2 and switch2vars.sh - NOT shipped with this repo).
#     Found in this folder or any parent, or set $env:SWITCH2_DEVKIT.
#     The drop is mounted READ-ONLY and never edited.
param(
    [Parameter(Position = 0)] [string] $Action = "build",
    [Parameter(Position = 1, ValueFromRemainingArguments = $true)] [string[]] $Rest
)
$ErrorActionPreference = "Stop"

$Repo = $PSScriptRoot
$BuildDir = "build_nx2"

# The repo folder is often a symlink into ~/Documents/GitHub on this machine;
# Docker for Windows does not follow Windows symlinks inside bind mounts, so
# resolve it to its real path before mounting.
$item = Get-Item -Force $Repo
if ($item.LinkType -eq "SymbolicLink") {
    $RepoSource = $item.Target
    if (-not [System.IO.Path]::IsPathRooted($RepoSource)) {
        $RepoSource = Join-Path (Split-Path $Repo -Parent) $RepoSource
    }
} else {
    $RepoSource = $Repo
}
if (-not (Test-Path (Join-Path $RepoSource "CMakeLists.txt"))) {
    Write-Error "Cannot find MilkyTracker sources at $RepoSource"
}

# --- locate the devkit drop -------------------------------------------------
$Devkit = $env:SWITCH2_DEVKIT
if (-not $Devkit) {
    $dir = $Repo
    while ($dir) {
        $hit = Get-ChildItem $dir -Directory -Filter "ounce-devkit-*" -ErrorAction SilentlyContinue |
            Where-Object { Test-Path (Join-Path $_.FullName "switch2vars.sh") } |
            Select-Object -First 1
        if ($hit) { $Devkit = $hit.FullName; break }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir) { break }
        $dir = $parent
    }
}
if (-not $Devkit -or -not (Test-Path (Join-Path $Devkit "switch2vars.sh"))) {
    Write-Error "Switch 2 devkit drop not found: put ounce-devkit-<date> in this folder or a parent, or set `$env:SWITCH2_DEVKIT."
}

$Vars = "/devkit/switch2vars.sh"
$WinDevkit = $Devkit -replace '\\', '/'
$WinRepo = $RepoSource -replace '\\', '/'
$Workdir = "/work/MilkyTrackerNX2"

$mounts = @(
    "-v", "${WinRepo}:${Workdir}",
    "-v", "${WinDevkit}:/devkit:ro",
    "-v", "${WinDevkit}/libounce:/opt/devkitpro/libounce:ro",
    "-v", "${WinDevkit}/portlibs/switch2:/opt/devkitpro/portlibs/switch2:ro"
)

function Invoke-Docker([string] $Dir, [string] $Command) {
    docker run --rm @mounts -w $Dir devkitpro/devkita64 bash -lc $Command
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

switch ($Action) {
    "build" {
        $extra = if ($Rest) { $Rest -join ' ' } else { "" }
        Invoke-Docker $Workdir "source $Vars && bash platforms/nx/build_ns2.sh $BuildDir $extra"
    }
    "clean" {
        Remove-Item -Recurse -Force (Join-Path $Repo $BuildDir) -ErrorAction SilentlyContinue
        "cleaned $BuildDir"
    }
    "shell" {
        docker run --rm -it @mounts -w $Workdir devkitpro/devkita64 `
            bash -lc "source $Vars && exec bash"
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
    "run" {
        $cmd = $Rest -join " "
        Invoke-Docker $Workdir "source $Vars && $cmd"
    }
    default {
        Write-Host "Unknown action '$Action'. See header comment in portnx2.ps1."
        exit 1
    }
}
