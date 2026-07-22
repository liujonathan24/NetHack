#!/usr/bin/env bash
set -e
export PATH="/scratch/gpfs/ZHUANGL/jl0796/NetHackHarness/.venv/bin:$PATH"; source /scratch/gpfs/ZHUANGL/jl0796/emsdk/emsdk_env.sh 2>/dev/null
D='-DGCC_WARN -DNOCLIPPING -DNOMAIL -DNOTPARMDECL -DHACKDIR="/nethackdir" -DDEFAULT_WINDOW_SYS="rl" -DDLB -DNOCWD_ASSUMPTIONS -DNLE_USE_TILES -DNLE_ALLOW_SEEDING -DNLE_PER_ENV_FILES=1 -DNLE_PER_ENV_FLAGS=1 -DNLE_USE_ARENA_FREE=1'
I='-Iinclude -Ibuild/include -Ithird_party/libtmt -Ithird_party/deboost.context/include'
# recompile any src files passed as args
for f in "$@"; do o="build-wasm/obj/$(echo "$f" | tr '/' '_').o"; echo "cc $f"; emcc -O1 -g2 -w $D $I -sUSE_BZIP2=1 -c "$f" -o "$o"; done
echo "linking..."
emcc -O1 -g2 --profiling-funcs -sASSERTIONS=1 -sASYNCIFY -sASYNCIFY_STACK_SIZE=1048576 \
  -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=134217728 -sTOTAL_STACK=8388608 -sUSE_BZIP2=1 \
  -sFORCE_FILESYSTEM=1 -sEXIT_RUNTIME=1 --preload-file build/dat@/nethackdir \
  build-wasm/obj/*.o -o build-wasm/nethack_spike.js 2>/dev/null
