#!/usr/bin/env bash
# Phase-0 spike: build libnethack to WebAssembly with the emscripten_fiber
# fcontext shim (replacing per-arch asm). Reuses generated headers + dat from a
# prior NATIVE build in ./build (run nethack_core/build_engine.sh first).
# Arena/snapshot DISABLED for the spike (Phase 2 re-adds them).
set -uo pipefail
cd "$(dirname "$0")"
export PATH="/scratch/gpfs/ZHUANGL/jl0796/NetHackHarness/.venv/bin:$PATH"
source /scratch/gpfs/ZHUANGL/jl0796/emsdk/emsdk_env.sh 2>/dev/null
command -v emcc >/dev/null || { echo "emcc not on PATH"; exit 2; }

OBJ=build-wasm/obj
rm -rf "$OBJ"; mkdir -p "$OBJ"

DEFS=(-DGCC_WARN -DNOCLIPPING -DNOMAIL -DNOTPARMDECL -DHACKDIR='"/nethackdir"'
      -DDEFAULT_WINDOW_SYS='"rl"' -DDLB -DNOCWD_ASSUMPTIONS -DNLE_USE_TILES
      -DNLE_ALLOW_SEEDING -DNLE_PER_ENV_FILES=1 -DNLE_PER_ENV_FLAGS=1
      -DNLE_USE_ARENA_FREE=1)
INC=(-Iinclude -Ibuild/include -Ithird_party/libtmt -Ithird_party/deboost.context/include)
OPT=(-O1 -w -sUSE_BZIP2=1)

SRC=()
for f in src/*.c; do case "$f" in */nle_fast_reset.c) continue;; esac; SRC+=("$f"); done
SRC+=(sys/share/posixregex.c sys/share/ioctl.c
      sys/unix/unixunix.c sys/unix/unixmain.c sys/unix/unixres.c
      win/rl/winrl.cc third_party/libtmt/tmt.c
      third_party/deboost.context/source/fcontext_emscripten.c)
for f in win/tty/*.c; do SRC+=("$f"); done

compile_one() {
  local f="$1" o="$OBJ/${1//\//_}.o"
  if emcc "${OPT[@]}" "${DEFS[@]}" "${INC[@]}" -c "$f" -o "$o" 2>"$o.err"; then
    :
  else
    echo "FAIL: $f"; grep -iE 'error:' "$o.err" | head -2; touch "$o.FAILED"
  fi
}

echo "=== compiling ${#SRC[@]} sources (pool of 8) ==="
for f in "${SRC[@]}"; do
  compile_one "$f" &
  while [ "$(jobs -r | wc -l)" -ge 8 ]; do wait -n; done
done
wait

NOBJ=$(ls "$OBJ"/*.o 2>/dev/null | wc -l)
NFAIL=$(ls "$OBJ"/*.FAILED 2>/dev/null | wc -l)
echo "=== compiled $NOBJ objects, $NFAIL failed ==="
[ "$NFAIL" -gt 0 ] && { echo "compile failures — fix before link"; exit 1; }

echo "=== linking to WASM (Asyncify) ==="
emcc -O1 -sASYNCIFY -sASYNCIFY_STACK_SIZE=131072 \
  -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=134217728 -sTOTAL_STACK=8388608 \
  -sUSE_BZIP2=1 -sFORCE_FILESYSTEM=1 \
  -sMODULARIZE=1 -sEXPORT_NAME=NetHackModule \
  -sEXPORTED_RUNTIME_METHODS='["ccall","cwrap","getValue","setValue","UTF8ToString","HEAPU8","HEAP8","FS"]' \
  -sEXPORTED_FUNCTIONS='["_nle_start","_nle_step","_nle_end","_nle_set_seed","_nle_get_tune","_nle_tune_count","_nle_tune_name","_nle_set_state","_nle_goto_depth","_nle_goto_abs","_nle_hero_on_stair","_nle_num_dungeons","_nle_dungeon_info","_nle_seat_on_stair","_nle_level_up","_malloc","_free"]' \
  --preload-file build-wasm/dat@/nethackdir \
  "$OBJ"/*.o -o build-wasm/nethack.js 2>build-wasm/link.err
LINKRC=$?
echo "link exit: $LINKRC"
if [ $LINKRC -ne 0 ]; then
  echo "=== link errors ==="; grep -iE 'error|undefined symbol|cannot|wasm-ld' build-wasm/link.err | head -40
else
  ls -la build-wasm/nethack.js build-wasm/nethack.wasm build-wasm/nethack.data 2>/dev/null
  echo "WASM BUILD OK"
fi
