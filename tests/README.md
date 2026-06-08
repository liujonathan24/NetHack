# NLE test suite

Run the whole suite (build + tests):

    tests/run_tests.sh

Run the long over-time hang proof (a few minutes by default; pass seconds to override):

    tests/run_tests.sh --soak
    tests/run_tests.sh --soak 600

## What each test proves
- `test_sentinel_unit`     — registry/snapshot logic (no libnethack needed).
- `test_sentinel_crash`    — a hard fault writes an attributed breadcrumb and re-raises.
- `test_sentinel_watchdog` — with `NLE_WATCHDOG_SECS`, a stalled env is aborted loudly.
- `test_no_hang`           — N envs across T threads keep making forward progress for the
                             whole duration; a stalled env fails the test (exit 1).
- `test_multi_env`         — multi-env churn stress for the per-env refactor.
- `verify_determinism_all` — golden-trajectory determinism.

## Crash breadcrumbs
On a hard crash, the sentinel writes `${NLE_CRASH_DIR:-.}/nle_crash_<pid>.txt` and the
same report to stderr: the faulting env's id/seed/step/action/dlvl, a line per live env,
and a raw backtrace. Symbolize the backtrace offline:

    addr2line -e src/build/libnethack.so <address>

## Watchdog (hang detection in production)
Off by default. Enable for long unattended runs:

    NLE_WATCHDOG_SECS=30 <your program>

If total forward progress across all envs stalls for that many seconds, the watchdog
dumps all envs' vital signs and aborts loudly (instead of hanging forever). Note: the
watchdog detects a GLOBAL stall (all envs stuck); per-env stalls are what test_no_hang
catches via the snapshot API.
