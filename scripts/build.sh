#!/usr/bin/env bash
# Fast build helper for CoinSorter / Superforce.
# Features:
#  - Uses Ninja generator if available.
#  - Enables ccache if present.
#  - Auto-detects ~75% of logical cores for parallel build.
#  - Release build with -O3, LTO (if supported), and native architecture (optional flag).
#  - Pass through extra CMake args.
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

GEN=""
if command -v ninja >/dev/null 2>&1; then
  GEN="-G Ninja"
fi

CCACHE=""
if command -v ccache >/dev/null 2>&1; then
  export CC="ccache cc"
  export CXX="ccache c++"
  CCACHE="(ccache enabled)"
fi

CORES=$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu)
: "${CORES:=1}"
PAR=$(( (CORES*3)/4 ))
if [ "$PAR" -lt 1 ]; then PAR=1; fi

CFLAGS_EXTRA="-O3 -DNDEBUG"
# Try LTO
CFLAGS_EXTRA+=" -flto"
# Optional native tuning
if [[ "${NATIVE:-1}" == "1" ]]; then
  CFLAGS_EXTRA+=" -march=native"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake $GEN -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS_RELEASE="${CFLAGS_EXTRA}" "$@" "$ROOT_DIR"
if [ -n "$GEN" ]; then
  cmake --build . -- -j "$PAR"
else
  cmake --build . --parallel "$PAR"
fi

echo "Build complete using $PAR jobs ${CCACHE}"
