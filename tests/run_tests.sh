#!/bin/bash
# Build libnethack + all sentinel/integration tests, run the suite.
# Usage: tests/run_tests.sh [--soak [secs]]
set -uo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

SOAK_ARGS=()
if [ "${1:-}" = "--soak" ]; then
  SOAK_ARGS=(--soak)
  [ -n "${2:-}" ] && SOAK_ARGS+=("$2")
fi

export NETHACKDIR="$ROOT/src/build/dat"
INC="-Isrc/include"
LIB="-Lsrc/build -lnethack -Wl,-rpath,src/build -lm -lpthread"
fail=0
run() { echo "=== $1 ==="; shift; "$@"; if [ $? -ne 0 ]; then echo "  -> FAILED"; fail=1; else echo "  -> ok"; fi; }

# compile <name> <output> <gcc args...> ; sets fail=1 and returns 1 on error
compile() {
  local name="$1" out="$2"; shift 2
  if ! gcc "$@" -o "$out" 2>/tmp/nle_cc_err.$$; then
    echo "=== $name (compile) ==="
    sed 's/^/    /' /tmp/nle_cc_err.$$
    echo "  -> COMPILE FAILED"
    fail=1
    rm -f /tmp/nle_cc_err.$$
    return 1
  fi
  rm -f /tmp/nle_cc_err.$$
  return 0
}

echo "### building libnethack"
cmake -S src -B src/build >/dev/null && cmake --build src/build --target nethack -j8 >/dev/null || { echo "build failed"; exit 1; }

echo "### standalone unit tests (compile nle_sentinel.c directly; no libnethack)"
compile "sentinel_unit"     /tmp/t_unit  -O2 -std=c11 tests/test_sentinel_unit.c     src/src/nle_sentinel.c $INC -lpthread            && run "sentinel_unit"     /tmp/t_unit
compile "sentinel_crash"    /tmp/t_crash -O2 -std=c11 tests/test_sentinel_crash.c    src/src/nle_sentinel.c $INC -lpthread -rdynamic  && run "sentinel_crash"    /tmp/t_crash
compile "sentinel_watchdog" /tmp/t_wd    -O2 -std=c11 tests/test_sentinel_watchdog.c src/src/nle_sentinel.c $INC -lpthread            && run "sentinel_watchdog" timeout 30 /tmp/t_wd

echo "### integration tests (link libnethack; sentinel symbols come from the .so)"
compile "multi_env" /tmp/t_multi  -O2 -fopenmp -DNLE_ALLOW_SEEDING=1 tests/test_multi_env.c $INC $LIB && run "multi_env" /tmp/t_multi 64 500 4
compile "no_hang"   /tmp/t_nohang -O2 -fopenmp -DNLE_ALLOW_SEEDING=1 tests/test_no_hang.c   $INC $LIB && run "no_hang"   /tmp/t_nohang "${SOAK_ARGS[@]}"

echo "### determinism"
run "determinism" bash tests/verify_determinism_all.sh

if [ $fail -ne 0 ]; then echo "SUITE: FAIL"; exit 1; fi
echo "SUITE: PASS"
