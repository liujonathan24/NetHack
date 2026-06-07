# Crash & Hang Sentinel for NLE

**Date:** 2026-06-07
**Status:** Approved (design)
**Scope:** `vendor/nle` (this NetHack fork). No changes to any downstream consumer (e.g. PufferLib).

## Problem

When NLE is scaled across many environments and OS threads (vecenv-style
training), failures become *un-noticed*:

- A **hard crash** (SIGSEGV / SIGABRT / double-free / SIGFPE / SIGBUS) inside a
  worker thread takes down the whole process. There is no breadcrumb saying
  *which* env, seed, step, action, or thread was responsible — only a raw process
  exit code survives.
- A **hang** (a step that never returns, a deadlock, an infinite loop in level
  generation) makes the process spin forever with no diagnostic at all. Wall-clock
  job limits eventually kill it, hours later, with nothing to show.
- A graceful NetHack **panic** is already survivable (`really_done(PANICKED)` sets
  `obs->how_done = PANICKED` and yields), and downstream code can count it — but the
  *reason* for the panic is lost, and the rare recursive/wizard-mode `NH_abort()`
  path is a silent hard abort.

The goal of this work is **to notice every such failure loudly, fast, and with
enough attribution to act on it** — not to recover from it. (Recovery, e.g. per-env
isolation, is explicitly a later phase. Continuing a process past memory corruption
is unsafe and out of scope.)

A second, equally important goal: ship a **tests/ folder that proves NLE does not
hang over time** under scale, so regressions are caught in CI rather than in a
multi-hour training run.

## Non-goals

- No recovery / continuation after a hard fault. The sentinel re-raises so the
  original crash semantics (exit code, core dump) are preserved.
- No changes to NetHack game logic or RNG. Identity fields (seed) are read, never
  written.
- No performance regression on the step hot path.
- No changes outside `vendor/nle`.

## Architecture

One self-contained module inside NLE, plus four one-line hooks in the existing
step lifecycle, plus a tests folder.

```
nle_start()  ─► nle_sentinel_global_init()   // pthread_once: install handlers; start watchdog iff env var set
             ─► nle->sentinel = nle_sentinel_register(seed)

nle_step()   ─► nle_sentinel_beat(nle->sentinel, obs->action, dlvl)   // hot path: ~4 stores + 1 relaxed atomic inc

panic()      ─► nle_sentinel_set_panic(current slot, msg)             // capture reason before really_done/NH_abort

nle_end()    ─► nle_sentinel_unregister(nle->sentinel)

                 ┌─ signal handler (SIGSEGV/SIGABRT/SIGFPE/SIGBUS)  ← hard crash
                 ├─ watchdog pthread (opt-in: NLE_WATCHDOG_SECS)    ← hang
                 └─ snapshot API (poll, never abort)                ← used by tests
```

Because the hooks live in `nle_step` / `nle_start` / `nle_end`, **every** NLE
consumer benefits automatically — PufferLib calls `nle_step` directly
(`env->fn_step = &nle_step`), so no PufferLib code changes.

## Components

### 1. Per-env vital signs registry

A fixed-capacity, statically-allocated array of slots. One slot per live env.

```c
typedef struct {
    _Atomic uint64_t heartbeat;   /* monotonically increasing; bumped every step   */
    uint64_t         step;        /* total steps this env has taken                 */
    unsigned long    seed;        /* env->seed_a, for attribution                   */
    int              env_id;      /* registration index                             */
    int              last_action; /* obs->action of the most recent step            */
    int              dlvl;        /* current dungeon level                          */
    pthread_t        thread;      /* thread that most recently stepped this env     */
    char             panic_msg[160]; /* last panic() reason, if any                 */
    _Atomic int      in_use;      /* slot allocation flag                           */
} nle_sentinel_slot;
```

- `nle_sentinel_register(seed)` claims a free slot (atomic CAS on `in_use`),
  returns an opaque `void *` stored in `nle_ctx_t.sentinel`.
- A `__thread nle_sentinel_slot *` records the slot the **current thread** is
  servicing, so the signal handler can identify the faulting env without a lookup.
- Fixed capacity (default 8192, compile-time constant). Overflow logs once and
  returns NULL; `beat()` tolerates a NULL slot (no-op) so an over-cap env still runs.

### 2. Step hot path: `nle_sentinel_beat`

Called once at the top of `nle_step`. Strictly lock-free:

```c
void nle_sentinel_beat(void *slot_, int action, int dlvl) {
    nle_sentinel_slot *s = slot_;
    if (!s) return;
    s->last_action = action;
    s->dlvl = dlvl;
    s->step++;
    s->thread = pthread_self();
    atomic_fetch_add_explicit(&s->heartbeat, 1, memory_order_relaxed);
    _nle_sentinel_tls = s;   /* __thread pointer for the signal handler */
}
```

Cost is a handful of stores plus one relaxed atomic increment — negligible against
NLE's per-step cost. No syscalls, no locks, no allocation. This is the only cost
in steady state.

### 3. Signal handler

Installed once via `sigaction` for SIGSEGV, SIGABRT, SIGFPE, SIGBUS (with
`SA_SIGINFO`; on an alternate signal stack via `sigaltstack` so a stack-overflow
SIGSEGV is still reportable). The handler is **strictly async-signal-safe**: it
uses only `write(2)`, integer-to-string conversion into static buffers, and
`backtrace`/`backtrace_symbols_fd`. No `malloc`, no `printf`, no locks.

On a fault it emits, to **stderr** (captured by SLURM) and to a breadcrumb file
`${NLE_CRASH_DIR:-.}/nle_crash_<pid>.txt`:

- signal name + `si_code`, faulting thread id;
- the crashing thread's env from the `__thread` slot: **env_id, seed, step,
  last_action, dlvl**, and `panic_msg` if set;
- a compact vital-signs line for **every** registered slot (so you see what all
  threads were mid-flight on at the moment of death);
- a raw backtrace via `backtrace_symbols_fd`.

Then it restores the default disposition for the signal and re-raises, so the
process dies with the original signal — preserving the exit code and any core dump
(`ulimit -c`). The sentinel never swallows a crash; it annotates it.

Backtrace addresses are always usable; symbol names are recovered offline with
`addr2line -e libnethack.so <addr>` (documented in the tests README). This avoids a
hard `-rdynamic` build dependency.

### 4. Watchdog thread (opt-in)

If `NLE_WATCHDOG_SECS` is set to a positive integer, `nle_sentinel_global_init`
spawns one detached pthread. Every `secs/2` it computes the sum of all slots'
`heartbeat` counters. If that global sum has not advanced for `secs`, it declares a
hang, writes the same all-slots vital-signs report (stderr + breadcrumb), and
`abort()`s loudly — converting a silent infinite hang into a loud, attributable
crash that the signal handler then also documents.

Off by default → zero risk and zero cost for normal runs. Intended for long
unattended sweeps.

### 5. Panic enrichment

In `end.c`, `panic()` copies its formatted message into the current env's slot
(`nle_sentinel_set_panic`) before calling `really_done(PANICKED)` (and before the
recursive/wizard `NH_abort()` branch). This means:

- the breadcrumb shows *why* a panic happened, not just that one did;
- the rare `NH_abort()` hard-abort path is now attributable instead of silent.

The existing downstream panic counter (e.g. PufferLib's `errors` stat) is
unchanged; this only adds the reason string.

## Public API (`nle_sentinel.h`)

```c
void  nle_sentinel_global_init(void);                 /* idempotent (pthread_once) */
void *nle_sentinel_register(unsigned long seed);      /* -> opaque slot, or NULL   */
void  nle_sentinel_beat(void *slot, int action, int dlvl);
void  nle_sentinel_set_panic(void *slot, const char *msg);
void  nle_sentinel_unregister(void *slot);

/* For tests / external monitors: poll, never abort. */
typedef struct {
    int           env_id;
    unsigned long seed;
    uint64_t      step;
    uint64_t      heartbeat;
    int           last_action;
    int           dlvl;
    int           in_use;
} nle_sentinel_stat;

int      nle_sentinel_snapshot(nle_sentinel_stat *out, int max); /* returns count */
uint64_t nle_sentinel_total_heartbeat(void);                     /* sum of all slots */
```

## Integration points (exact)

| File | Change |
|------|--------|
| `src/include/nle.h` | add `void *sentinel;` to `struct nle_ctx` |
| `src/include/nle_sentinel.h` | new — public API above |
| `src/src/nle_sentinel.c` | new — registry, handler, watchdog, snapshot |
| `src/src/nle.c` (`nle_start`) | `nle_sentinel_global_init()`; `nle->sentinel = nle_sentinel_register(seed)` |
| `src/src/nle.c` (`nle_step`, ~864) | `nle_sentinel_beat(nle->sentinel, obs->action, dlvl)` at top |
| `src/src/nle.c` (`nle_end`, ~988) | `nle_sentinel_unregister(nle->sentinel)` |
| `src/src/end.c` (`panic`) | `nle_sentinel_set_panic(...)` before `really_done`/`NH_abort` |
| `src/build` (Makefile/CMake) | compile `nle_sentinel.c` into libnethack; link `-lpthread`, backtrace support |

## Tests (`vendor/nle/tests/`)

Consolidate the existing informal harnesses here (via `git mv`, preserving history)
and add the no-hang soak.

```
tests/
  test_no_hang.c            # NEW — tiered soak; the "no hangs over time" proof
  test_multi_env.c          # moved from repo root
  verify_determinism.c      # moved from repo root
  verify_determinism_all.sh # moved from repo root
  golden/                   # moved from repo root
  run_tests.sh              # NEW — build + run the whole suite, nonzero on any failure
  README.md                 # NEW — how to run; long soak; addr2line crash symbolization
```

### `test_no_hang.c` — tiered soak

Drives many envs across many OS threads and asserts forward progress using the
queryable snapshot API. It **fails gracefully** (returns non-zero) rather than
aborting, so a hang is a normal CI failure.

- **Default (CI):** 64 envs across 8 pthreads, run ~30 s. A monitor polls
  `nle_sentinel_snapshot` once per second; if any env's `heartbeat` fails to
  advance for a stall threshold (e.g. 5 s), or a signal handler fired, the test
  prints the offending env (id/seed/step/action/dlvl) and exits 1. If all envs
  advance for the whole duration, exit 0.
- **`--soak`:** large N (e.g. 1024+), more threads, minutes-to-hours; same
  assertions. This is the real over-time proof for unattended runs.
- Output: per-env step counts, min/median/max progress, total steps/sec, and a
  clear PASS/FAIL line.

### `run_tests.sh`

Builds libnethack and the test binaries, then runs: determinism check, multi-env
check, and `test_no_hang` (default tier). Prints a one-line summary per test and
exits non-zero if any fail. Documented to accept `--soak` to escalate the hang
test.

## Safety & performance

- Hot path is lock-free; ~4 stores + 1 relaxed atomic per step. No SPS regression
  (verified via a no-sentinel vs sentinel bench in `run_tests.sh`).
- Signal handler is strictly async-signal-safe and runs on an alternate stack.
- Watchdog is off unless `NLE_WATCHDOG_SECS` is set; when on it is a detached,
  process-lifetime thread (no join, no leak).
- Registry is a fixed-size static array — no allocation, no races on a lock.
- One pointer field added to `nle_ctx_t`; everything else is in the new module.

## Verification plan

1. **No-regression:** bench steps/sec with and without `nle_sentinel_beat`
   compiled in; assert within noise.
2. **Crash attribution:** fault-injection build that derefs NULL / calls `abort()`
   in a chosen env; assert the breadcrumb names the correct env_id/seed and
   contains a backtrace; assert the process still exits with the original signal.
3. **Hang detection:** fault-injection build that `sleep`s forever in one env;
   with `NLE_WATCHDOG_SECS` set, assert a loud abort + attribution; in the test
   harness (no watchdog), assert `test_no_hang` reports the stalled env and exits 1.
4. **No-hang soak:** `test_no_hang` default tier passes in CI; `--soak` passes in a
   longer manual run.
