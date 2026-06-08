#!/usr/bin/env bash
# verify_determinism_all.sh — rebuild the determinism harness and replay
# every golden file under tests/golden against the current build of
# libnethack.so. Exits 0 iff all goldens match, nonzero on any mismatch.
#
# Run from anywhere, e.g.:
#
#   tests/verify_determinism_all.sh
#
# Environment overrides:
#   GOLDEN_DIR   — directory of golden_seed*.bin files (default tests/golden)
#   HARNESS      — path to the harness binary (default /tmp/verify_determinism)
#   OCEAN_DIR    — dir containing the PufferLib ocean wrapper nethack.h
#   CC           — compiler (default clang)
#   CFLAGS_EXTRA — appended to the build command

# Resolve repo root so the script works regardless of CWD.
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

set -uo pipefail

# The determinism harness (tests/verify_determinism.c) #includes the PufferLib
# ocean wrapper "nethack.h", which lives in the parent PufferLib repo, not in
# this nle repo. Locate it (default: ../../ocean/nethack relative to this nle
# repo root). Override with OCEAN_DIR if your layout differs.
OCEAN_DIR="${OCEAN_DIR:-$ROOT/../../ocean/nethack}"

GOLDEN_DIR="${GOLDEN_DIR:-tests/golden}"
HARNESS="${HARNESS:-/tmp/verify_determinism}"
CC="${CC:-clang}"
CFLAGS_EXTRA="${CFLAGS_EXTRA:-}"

if [[ ! -d "$GOLDEN_DIR" ]]; then
    echo "verify_determinism_all: GOLDEN_DIR '$GOLDEN_DIR' not found" >&2
    exit 2
fi

if [[ ! -f "$OCEAN_DIR/nethack.h" ]]; then
    echo "verify_determinism_all: SKIP — ocean wrapper '$OCEAN_DIR/nethack.h' not found." >&2
    echo "  The determinism harness depends on the PufferLib ocean wrapper header," >&2
    echo "  which lives outside this nle repo. Set OCEAN_DIR to the directory that" >&2
    echo "  contains nethack.h to run this test." >&2
    exit 0
fi

set -e

echo "==> Rebuilding harness"
"$CC" -O2 -Wall -Wextra -std=gnu11 \
    -I"$ROOT/include" -I"$OCEAN_DIR" \
    -DNETHACK_USE_BLSTATS=1 \
    $CFLAGS_EXTRA \
    tests/verify_determinism.c \
    -o "$HARNESS" \
    -L"$ROOT/src/build" -lnethack \
    -Wl,-rpath="$ROOT/src/build" \
    -ldl -lpthread -lm

n_files=$(find "$GOLDEN_DIR" -maxdepth 1 -name 'golden_seed*.bin' | wc -l)
echo "==> Replaying $n_files golden file(s) under $GOLDEN_DIR"
echo

if "$HARNESS" replay-all --in-dir "$GOLDEN_DIR"; then
    status=0
else
    status=$?
fi

echo
if [[ $status -eq 0 ]]; then
    echo "verify_determinism_all: PASS — $n_files seed(s) checked, all OK"
else
    echo "verify_determinism_all: FAIL — replay-all exited with status $status"
fi
exit $status
