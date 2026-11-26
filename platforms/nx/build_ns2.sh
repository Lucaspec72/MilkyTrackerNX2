#!/bin/bash
# Container-side build for MilkyTrackerNX2 (Switch 2 / libounce + OunceVK).
# Called by portnx2.ps1, with switch2vars.sh already sourced.
set -eo pipefail

PLATFORM_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ROOT_DIR="$PLATFORM_DIR/../.."
BUILD_DIR="${1:-build_nx2}"
[[ $# -gt 0 ]] && shift
EXTRA_ARGS=("$@")

source "$PLATFORM_DIR/_env.sh"

SWITCH1_PORTLIBS="${DEVKITPRO}/portlibs/switch"
mkdir -p "$BUILD_DIR"

aarch64-none-elf-cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DZZIPLIB_LIBRARIES="${SWITCH1_PORTLIBS}/lib/libzzip.a" \
    "${EXTRA_ARGS[@]}"

cmake --build "$BUILD_DIR" --target MilkyTrackerNX2-nro
echo "Built: $BUILD_DIR/MilkyTrackerNX2.nro"
