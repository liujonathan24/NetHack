#!/usr/bin/env bash
# Regenerate NetHack data with the wasm32 ABI (4-byte long) so the wasm engine
# accepts it. Builds makedefs/dgn_comp/lev_comp/dlb to WASM (NODERAWFS via node),
# reusing the native build's bison/flex parsers + headers (ABI-portable).
# makedefs uses ../dat and ../include relative to cwd, so we run the tools from
# datgen/dat with a sibling datgen/include. Output: build-wasm/dat/nhdat.
set -uo pipefail
cd "$(dirname "$0")"
export PATH="/scratch/gpfs/ZHUANGL/jl0796/NetHackHarness/.venv/bin:$PATH"
source /scratch/gpfs/ZHUANGL/jl0796/emsdk/emsdk_env.sh 2>/dev/null
NB=build
W=build-wasm/datgen
rm -rf "$W"; mkdir -p "$W/dat" "$W/include"
cp -f dat/* "$W/dat"/ 2>/dev/null || true
cp -f "$NB"/include/*.h "$W/include"/ 2>/dev/null || true

DEFS="-DGCC_WARN -DNOCLIPPING -DNOMAIL -DNOTPARMDECL -DDLB -DNOCWD_ASSUMPTIONS -DNLE_USE_TILES -DNLE_ALLOW_SEEDING"
INC="-Iinclude -I$NB/include -I$NB/util -Ithird_party/deboost.context/include"
TOOLOPT="-O1 -w -sNODERAWFS=1 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=67108864 -sEXIT_RUNTIME=1"

echo "=== building wasm tools ==="
emcc $TOOLOPT $DEFS -DNLE_OBJECTS_GLOBAL $INC util/makedefs.c src/monst.c src/objects.c -o "$W/makedefs.js" || exit 1
emcc $TOOLOPT $DEFS $INC util/dgn_main.c util/panic.c src/alloc.c "$NB"/util/dgn_parser.c "$NB"/util/dgn_scanner.c -o "$W/dgn_comp.js" || exit 1
emcc $TOOLOPT $DEFS -DNLE_OBJECTS_GLOBAL $INC util/lev_main.c util/panic.c src/decl.c src/monst.c src/objects.c src/alloc.c src/drawing.c "$NB"/util/lev_parser.c "$NB"/util/lev_scanner.c -o "$W/lev_comp.js" || exit 1
emcc $TOOLOPT $DEFS $INC util/dlb_main.c src/dlb.c util/panic.c src/alloc.c -o "$W/dlb.js" || exit 1
echo "tools built"

cd "$W/dat"     # cwd so ../dat and ../include resolve to datgen/dat, datgen/include
MD="node ../makedefs.js"; DGN="node ../dgn_comp.js"; LEV="node ../lev_comp.js"; DLB="node ../dlb.js"
echo "=== makedefs data files ==="
for m in -s -d -r -q -h -v -o -p; do echo " makedefs $m"; $MD $m || { echo "makedefs $m FAIL"; exit 2; }; done
echo "=== dungeon ==="; $MD -e && $DGN dungeon.pdf || { echo "dungeon FAIL"; exit 3; }
echo "=== levels ==="
$LEV bigroom.des castle.des endgame.des gehennom.des knox.des medusa.des mines.des oracle.des sokoban.des tower.des yendor.des || { echo "special FAIL"; exit 4; }
$LEV Arch.des Barb.des Caveman.des Healer.des Knight.des Monk.des Priest.des Ranger.des Rogue.des Samurai.des Tourist.des Valkyrie.des Wizard.des || { echo "quest FAIL"; exit 5; }
echo "=== nhdat ==="
LEVS=$(ls *.lev 2>/dev/null)
LC_ALL=C $DLB cf nhdat help hh cmdhelp keyhelp history opthelp wizhelp \
  dungeon tribute bogusmon data engrave epitaph oracles options quest.dat rumors $LEVS || { echo "dlb FAIL"; exit 6; }
: > perm; : > record; : > logfile; : > xlogfile
cd ../..        # back to src/
rm -rf build-wasm/dat; mkdir -p build-wasm/dat
cp "$W/dat"/nhdat "$W/dat"/perm "$W/dat"/record "$W/dat"/logfile "$W/dat"/xlogfile build-wasm/dat/
ls -la build-wasm/dat/nhdat && echo "WASM DAT OK"
