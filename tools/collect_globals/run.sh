#!/usr/bin/env bash
# Re-run the globals collector on the game sources under src/.
#
#   tools/collect_globals/run.sh [<build dir>]
#
# Expects an UNMODIFIED (upstream) game tree: the collector rewrites the
# sources in place, so run it once on a fresh tree (see README.md). The
# reference build it needs (compile_commands.json + the object files of the
# unmodified library, used to decide which objects are really writable) is
# produced here in <build dir> (default: src/build-ref).
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
SRC="$ROOT/src"
BUILD="${1:-$SRC/build-ref}"
PY="${PYTHON:-python3}"

$PY -c "import clang.cindex" 2>/dev/null || {
    echo "need the libclang python bindings: pip install libclang clang" >&2; exit 1; }

# 1. reference build of the unmodified tree (needs a python with setuptools
#    for the vendored pybind11; the library itself is plain C)
cmake -S "$SRC" -B "$BUILD" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo ${PYTHON_EXECUTABLE:+-DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE}
cmake --build "$BUILD" --target nethack -j"${JOBS:-8}"

# 2. rewrite the sources in place
cd "$HERE"
$PY -m nhglobals apply \
    --src "$SRC" --build "$BUILD" \
    --inventory "$BUILD/CMakeFiles/nethack.dir" \
    --exclude src/nle.c --exclude win/rl/winrl.cc \
    --ref-target lev_comp --ref-target dgn_comp --ref-target dlb \
    --none-target makedefs --none-target tilemap \
    --keep-file "$HERE/keep.txt" \
    --force def_inv_order \
    --model "$BUILD/nh_globals_model.pkl"

cat <<EOF

Generated: src/include/nh_globals.h, src/src/nh_globals.c, and in-place
edits of the game sources. Remaining by hand (see README.md):
  - src/util/CMakeLists.txt: makedefs/tilemap get -DNH_GLOBALS_NONE,
    lev_comp links src/nh_globals.c with -DNH_GLOBALS_STANDALONE
  - the NLE layer (src/nle.c, win/rl/winrl.cc) owns one struct nh_globals
    per environment and anchors nh_g at every entry point
Verify with:  tools/collect_globals/check_writable.sh <build>/CMakeFiles/nethack.dir
EOF
