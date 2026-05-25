#!/usr/bin/env bash
# verify_determinism_all.sh — rebuild the determinism harness and replay
# every golden file under ocean/nethack/golden against the current build of
# libnethack.so. Exits 0 iff all goldens match, nonzero on any mismatch.
#
# Run from the repo root, e.g.:
#
#   ./ocean/nethack/verify_determinism_all.sh
#
# Environment overrides:
#   GOLDEN_DIR   — directory of golden_seed*.bin files (default ocean/nethack/golden)
#   HARNESS      — path to the harness binary (default ./verify_determinism)
#   CC           — compiler (default clang)
#   CFLAGS_EXTRA — appended to the build command

set -euo pipefail

# Resolve the repo root: this script lives in ocean/nethack/.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$REPO_ROOT"

GOLDEN_DIR="${GOLDEN_DIR:-ocean/nethack/golden}"
HARNESS="${HARNESS:-./verify_determinism}"
CC="${CC:-clang}"
CFLAGS_EXTRA="${CFLAGS_EXTRA:-}"

if [[ ! -d "$GOLDEN_DIR" ]]; then
    echo "verify_determinism_all: GOLDEN_DIR '$GOLDEN_DIR' not found" >&2
    exit 2
fi

echo "==> Rebuilding harness"
"$CC" -O2 -Wall -Wextra -std=gnu11 \
    -I./vendor/nle/include -I./ocean/nethack \
    -DNETHACK_USE_BLSTATS=1 \
    $CFLAGS_EXTRA \
    ocean/nethack/verify_determinism.c \
    -o "$HARNESS" \
    -L./vendor/nle/src/build -lnethack \
    -Wl,-rpath="$PWD/vendor/nle/src/build" \
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
