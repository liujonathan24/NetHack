# Crash & Hang Sentinel Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make every hard crash, hang, and panic in NLE loud and attributable when scaled across threads/envs, and ship a `tests/` suite that proves NLE does not hang over time.

**Architecture:** A self-contained `nle_sentinel` module inside `vendor/nle` maintains a fixed, lock-free per-env vital-signs registry bumped once per `nle_step`. An async-signal-safe handler dumps which env/seed/step/thread faulted plus a backtrace; an opt-in watchdog thread (`NLE_WATCHDOG_SECS`) converts hangs into loud aborts; `panic()` records its reason. A `tests/` folder consolidates existing harnesses and adds a tiered no-hang soak that polls the sentinel's snapshot API and fails gracefully.

**Tech Stack:** C11 (`_Atomic`), POSIX threads + signals (glibc 2.34, pthread/backtrace in libc), CMake (globs `src/*.c`), OpenMP test harnesses.

**Working directory for ALL tasks:** `/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle` (its own git repo, remote `git@github.com:liujonathan24/NetHack.git`, branch `feature/crash-hang-sentinel`).

**Commit rules (from repo CLAUDE.md):** author is the user only; NO `Co-Authored-By: Claude`, no "Generated with Claude", no emoji/attribution trailers. Keep messages focused on the change.

**Build note:** `src/CMakeLists.txt:105-106` uses `file(GLOB NETHACK_SRC "src/*.c" ...)` (NOT `CONFIGURE_DEPENDS`). A new `src/src/*.c` file is only picked up after CMake re-runs. To rebuild libnethack cleanly:
```bash
cmake -S src -B src/build >/dev/null && cmake --build src/build --target nethack -j8
```
(Re-running `cmake -S src -B src/build` re-evaluates the glob.)

---

## File Structure

| File | Responsibility |
|------|----------------|
| `src/include/nle_sentinel.h` | Public API: lifecycle, hot-path beat, panic capture, snapshot for tests |
| `src/src/nle_sentinel.c` | Registry, signal handler, watchdog, breadcrumb writer (the whole module) |
| `src/include/nle.h` | One added field `void *sentinel;` on `struct nle_ctx` |
| `src/src/nle.c` | 3 hook calls in `nle_start` / `nle_step` / `nle_end` |
| `src/src/end.c` | 1 hook call in `panic()` |
| `src/CMakeLists.txt` | Link `Threads::Threads` into `nethack` (portability) |
| `tests/test_sentinel_unit.c` | Unit-tests the registry/snapshot logic (compiles `nle_sentinel.c` directly) |
| `tests/test_sentinel_crash.c` | Fork-based: a faulting child writes an attributed breadcrumb |
| `tests/test_sentinel_watchdog.c` | Fork-based: a stalled child is aborted by the watchdog |
| `tests/test_no_hang.c` | Tiered soak — the "no hangs over time" proof (poll snapshot, graceful fail) |
| `tests/test_multi_env.c`, `tests/verify_determinism.c`, `tests/verify_determinism_all.sh`, `tests/golden/` | Moved (via `git mv`) from repo root |
| `tests/run_tests.sh` | Builds lib + tests, runs the suite, nonzero on any failure |
| `tests/README.md` | How to run; long soak; `addr2line` crash symbolization |

---

## Task 1: Sentinel module scaffold — registry, beat, snapshot (no handler/watchdog yet)

**Files:**
- Create: `src/include/nle_sentinel.h`
- Create: `src/src/nle_sentinel.c`
- Test: `tests/test_sentinel_unit.c`

- [ ] **Step 1: Write the public header**

Create `src/include/nle_sentinel.h`:

```c
#ifndef NLE_SENTINEL_H
#define NLE_SENTINEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Max concurrent envs tracked. Headroom past the N=4096 stress config. */
#define NLE_SENTINEL_CAP 16384

/* Install signal handlers (once) and, iff NLE_WATCHDOG_SECS is set to a
 * positive integer, start the watchdog thread. Idempotent. */
void nle_sentinel_global_init(void);

/* Claim a slot for a new env. Returns an opaque slot pointer to store in the
 * env, or NULL if the registry is full (caller must tolerate NULL). */
void *nle_sentinel_register(unsigned long seed);

/* Hot path: call once per step. Lock-free. Tolerates slot == NULL. */
void nle_sentinel_beat(void *slot, int action, int dlvl);

/* Record the reason for a panic so the breadcrumb can show it. */
void nle_sentinel_set_panic(void *slot, const char *msg);

/* Release a slot when an env ends. Tolerates slot == NULL. */
void nle_sentinel_unregister(void *slot);

/* --- Read-only API for tests / external monitors: poll, never abort. --- */
typedef struct {
    int           env_id;
    unsigned long seed;
    uint64_t      step;
    uint64_t      heartbeat;
    int           last_action;
    int           dlvl;
    int           in_use;
} nle_sentinel_stat;

/* Copy up to `max` live slots into `out`. Returns the number written. */
int nle_sentinel_snapshot(nle_sentinel_stat *out, int max);

/* Sum of all slots' heartbeat counters (forward-progress signal). */
uint64_t nle_sentinel_total_heartbeat(void);

#ifdef __cplusplus
}
#endif

#endif /* NLE_SENTINEL_H */
```

- [ ] **Step 2: Write the failing unit test**

Create `tests/test_sentinel_unit.c`:

```c
/* Unit test for the sentinel registry. Compiles nle_sentinel.c directly,
 * so it needs no libnethack and no NetHack data files.
 *
 * Build (from vendor/nle/):
 *   gcc -O2 -DNLE_SENTINEL_TEST_BUILD test_unit_tmp.c ... (see run_tests.sh)
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "nle_sentinel.h"

int main(void) {
    /* register two envs */
    void *a = nle_sentinel_register(1111);
    void *b = nle_sentinel_register(2222);
    assert(a && b && a != b);

    /* beats advance step + heartbeat */
    nle_sentinel_beat(a, 'k', 1);
    nle_sentinel_beat(a, 'j', 2);
    nle_sentinel_beat(b, 'h', 1);
    assert(nle_sentinel_total_heartbeat() == 3);

    /* snapshot reflects state */
    nle_sentinel_stat stats[8];
    int n = nle_sentinel_snapshot(stats, 8);
    assert(n == 2);
    int seen_a = 0, seen_b = 0;
    for (int i = 0; i < n; i++) {
        if (stats[i].seed == 1111) { seen_a = 1; assert(stats[i].step == 2); assert(stats[i].dlvl == 2); assert(stats[i].last_action == 'j'); }
        if (stats[i].seed == 2222) { seen_b = 1; assert(stats[i].step == 1); }
    }
    assert(seen_a && seen_b);

    /* NULL slot is a no-op (no crash) */
    nle_sentinel_beat(NULL, 0, 0);

    /* unregister frees the slot */
    nle_sentinel_unregister(a);
    n = nle_sentinel_snapshot(stats, 8);
    assert(n == 1 && stats[0].seed == 2222);

    printf("test_sentinel_unit: PASS\n");
    return 0;
}
```

- [ ] **Step 3: Run the test to verify it fails (no implementation yet)**

Run:
```bash
gcc -O2 -std=c11 tests/test_sentinel_unit.c src/src/nle_sentinel.c -Isrc/include -o /tmp/t_unit && /tmp/t_unit
```
Expected: FAIL — `src/src/nle_sentinel.c` does not exist yet (compile error: "No such file").

- [ ] **Step 4: Write the minimal implementation (registry only)**

Create `src/src/nle_sentinel.c`:

```c
#define _GNU_SOURCE
#include "nle_sentinel.h"

#include <stdatomic.h>
#include <string.h>
#include <pthread.h>

typedef struct {
    _Atomic uint64_t heartbeat;     /* bumped every step                       */
    uint64_t         step;          /* total steps this env has taken          */
    unsigned long    seed;          /* attribution                             */
    int              env_id;        /* registration index                      */
    int              last_action;   /* most recent action                      */
    int              dlvl;          /* dungeon level as of last beat           */
    pthread_t        thread;        /* thread that last stepped this env       */
    char             panic_msg[160];/* last panic() reason, if any             */
    _Atomic int      in_use;        /* 0 = free, 1 = claimed                   */
} sentinel_slot;

static sentinel_slot g_slots[NLE_SENTINEL_CAP];
static __thread sentinel_slot *g_tls; /* slot the current thread is servicing */

void *nle_sentinel_register(unsigned long seed) {
    for (int i = 0; i < NLE_SENTINEL_CAP; i++) {
        int expected = 0;
        if (atomic_compare_exchange_strong(&g_slots[i].in_use, &expected, 1)) {
            sentinel_slot *s = &g_slots[i];
            atomic_store(&s->heartbeat, 0);
            s->step = 0;
            s->seed = seed;
            s->env_id = i;
            s->last_action = -1;
            s->dlvl = 0;
            s->thread = 0;
            s->panic_msg[0] = '\0';
            return s;
        }
    }
    return NULL; /* registry full */
}

void nle_sentinel_beat(void *slot_, int action, int dlvl) {
    sentinel_slot *s = (sentinel_slot *)slot_;
    if (!s) return;
    s->last_action = action;
    s->dlvl = dlvl;
    s->step++;
    s->thread = pthread_self();
    g_tls = s;
    atomic_fetch_add_explicit(&s->heartbeat, 1, memory_order_relaxed);
}

void nle_sentinel_set_panic(void *slot_, const char *msg) {
    sentinel_slot *s = slot_ ? (sentinel_slot *)slot_ : g_tls;
    if (!s || !msg) return;
    strncpy(s->panic_msg, msg, sizeof(s->panic_msg) - 1);
    s->panic_msg[sizeof(s->panic_msg) - 1] = '\0';
}

void nle_sentinel_unregister(void *slot_) {
    sentinel_slot *s = (sentinel_slot *)slot_;
    if (!s) return;
    if (g_tls == s) g_tls = NULL;
    atomic_store(&s->in_use, 0);
}

int nle_sentinel_snapshot(nle_sentinel_stat *out, int max) {
    int n = 0;
    for (int i = 0; i < NLE_SENTINEL_CAP && n < max; i++) {
        if (!atomic_load(&g_slots[i].in_use)) continue;
        sentinel_slot *s = &g_slots[i];
        out[n].env_id      = s->env_id;
        out[n].seed        = s->seed;
        out[n].step        = s->step;
        out[n].heartbeat   = atomic_load(&s->heartbeat);
        out[n].last_action = s->last_action;
        out[n].dlvl        = s->dlvl;
        out[n].in_use      = 1;
        n++;
    }
    return n;
}

uint64_t nle_sentinel_total_heartbeat(void) {
    uint64_t total = 0;
    for (int i = 0; i < NLE_SENTINEL_CAP; i++) {
        if (atomic_load(&g_slots[i].in_use))
            total += atomic_load(&g_slots[i].heartbeat);
    }
    return total;
}

/* global_init is defined in Task 2 (handler + watchdog). Provide a temporary
 * no-op so Task 1's unit test links; Task 2 replaces this body. */
void nle_sentinel_global_init(void) { }
```

- [ ] **Step 5: Run the test to verify it passes**

Run:
```bash
gcc -O2 -std=c11 tests/test_sentinel_unit.c src/src/nle_sentinel.c -Isrc/include -lpthread -o /tmp/t_unit && /tmp/t_unit
```
Expected: `test_sentinel_unit: PASS`

- [ ] **Step 6: Add the `sentinel` field to `nle_ctx`**

In `src/include/nle.h`, inside `struct nle_ctx { ... }`, add near the other bookkeeping fields:

```c
    void *sentinel; /* opaque nle_sentinel slot for this env (may be NULL) */
```

- [ ] **Step 7: Commit**

```bash
git add src/include/nle_sentinel.h src/src/nle_sentinel.c src/include/nle.h tests/test_sentinel_unit.c
git commit -m "feat(sentinel): per-env vital-signs registry with snapshot API"
```

---

## Task 2: Signal handler + breadcrumb writer

**Files:**
- Modify: `src/src/nle_sentinel.c`
- Test: `tests/test_sentinel_crash.c`

- [ ] **Step 1: Write the failing fork-based crash test**

Create `tests/test_sentinel_crash.c`:

```c
/* A child process installs the sentinel, registers an env, beats once, then
 * dereferences NULL. The parent verifies the child died of SIGSEGV AND that an
 * attributed breadcrumb file was written naming the env's seed. */
#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "nle_sentinel.h"

int main(void) {
    char dir[] = "/tmp/nle-crash-XXXXXX";
    assert(mkdtemp(dir));
    setenv("NLE_CRASH_DIR", dir, 1);

    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        nle_sentinel_global_init();
        void *s = nle_sentinel_register(0xABCDEF);
        nle_sentinel_beat(s, 'k', 7);
        volatile int *p = (volatile int *)0;
        *p = 1;            /* SIGSEGV */
        _exit(0);          /* unreachable */
    }

    int status = 0;
    waitpid(pid, &status, 0);
    assert(WIFSIGNALED(status));
    assert(WTERMSIG(status) == SIGSEGV); /* re-raised, exit semantics preserved */

    /* find the breadcrumb and check it names the seed (hex ABCDEF) */
    char path[512], buf[8192];
    snprintf(path, sizeof(path), "%s/nle_crash_%d.txt", dir, pid);
    FILE *f = fopen(path, "r");
    assert(f && "breadcrumb file not written");
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    fclose(f);
    assert(strstr(buf, "abcdef") || strstr(buf, "ABCDEF")); /* seed attribution */
    assert(strstr(buf, "SIGSEGV"));

    printf("test_sentinel_crash: PASS\n");
    return 0;
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run:
```bash
gcc -O2 -std=c11 tests/test_sentinel_crash.c src/src/nle_sentinel.c -Isrc/include -lpthread -o /tmp/t_crash && /tmp/t_crash
```
Expected: FAIL — child dies of SIGSEGV but no breadcrumb file exists (assert on `fopen`), because the handler isn't installed yet.

- [ ] **Step 3: Implement the handler + breadcrumb (replace the no-op `nle_sentinel_global_init`)**

In `src/src/nle_sentinel.c`, add includes at the top (after the existing ones):

```c
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <execinfo.h>
```

Replace the temporary `void nle_sentinel_global_init(void) { }` with the following block:

```c
/* ---- async-signal-safe output helpers ---- */
static char g_crash_path[512];   /* breadcrumb path, built at init           */
static volatile sig_atomic_t g_signal_fired = 0;

static void sas_write(int fd, const char *s) {
    size_t n = 0; while (s[n]) n++;
    ssize_t w = write(fd, s, n); (void)w;
}
/* unsigned long -> decimal, async-signal-safe */
static void sas_write_u(int fd, unsigned long v) {
    char b[32]; int i = 31; b[i--] = '\0';
    if (v == 0) b[i--] = '0';
    while (v) { b[i--] = (char)('0' + (v % 10)); v /= 10; }
    sas_write(fd, &b[i + 1]);
}
/* unsigned long -> hex, async-signal-safe */
static void sas_write_x(int fd, unsigned long v) {
    static const char *hx = "0123456789abcdef";
    char b[32]; int i = 31; b[i--] = '\0';
    if (v == 0) b[i--] = '0';
    while (v) { b[i--] = hx[v & 0xf]; v >>= 4; }
    sas_write(fd, "0x"); sas_write(fd, &b[i + 1]);
}

static void dump_report(int fd, const char *signame) {
    sas_write(fd, "=== NLE SENTINEL: ");
    sas_write(fd, signame);
    sas_write(fd, " ===\n");
    sas_write(fd, "pid="); sas_write_u(fd, (unsigned long)getpid());
    sas_write(fd, " tid="); sas_write_u(fd, (unsigned long)pthread_self());
    sas_write(fd, "\n");

    sentinel_slot *cur = g_tls;
    if (cur) {
        sas_write(fd, "FAULTING ENV: id="); sas_write_u(fd, (unsigned long)cur->env_id);
        sas_write(fd, " seed="); sas_write_x(fd, cur->seed);
        sas_write(fd, " step="); sas_write_u(fd, cur->step);
        sas_write(fd, " action="); sas_write_u(fd, (unsigned long)cur->last_action);
        sas_write(fd, " dlvl="); sas_write_u(fd, (unsigned long)cur->dlvl);
        sas_write(fd, "\n");
        if (cur->panic_msg[0]) { sas_write(fd, "panic_msg="); sas_write(fd, cur->panic_msg); sas_write(fd, "\n"); }
    } else {
        sas_write(fd, "FAULTING ENV: <unknown thread, no slot>\n");
    }

    sas_write(fd, "--- all envs (id seed step heartbeat dlvl) ---\n");
    for (int i = 0; i < NLE_SENTINEL_CAP; i++) {
        if (!atomic_load(&g_slots[i].in_use)) continue;
        sentinel_slot *s = &g_slots[i];
        sas_write_u(fd, (unsigned long)s->env_id); sas_write(fd, " ");
        sas_write_x(fd, s->seed); sas_write(fd, " ");
        sas_write_u(fd, s->step); sas_write(fd, " ");
        sas_write_u(fd, (unsigned long)atomic_load(&s->heartbeat)); sas_write(fd, " ");
        sas_write_u(fd, (unsigned long)s->dlvl); sas_write(fd, "\n");
    }

    sas_write(fd, "--- backtrace ---\n");
    void *bt[64];
    int nb = backtrace(bt, 64);
    backtrace_symbols_fd(bt, nb, fd);
    sas_write(fd, "=== END SENTINEL ===\n");
}

static void crash_handler(int sig, siginfo_t *info, void *uctx) {
    (void)info; (void)uctx;
    g_signal_fired = sig;
    const char *name =
        sig == SIGSEGV ? "SIGSEGV" :
        sig == SIGABRT ? "SIGABRT" :
        sig == SIGFPE  ? "SIGFPE"  :
        sig == SIGBUS  ? "SIGBUS"  : "SIGNAL";

    dump_report(STDERR_FILENO, name);
    if (g_crash_path[0]) {
        int fd = open(g_crash_path, O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (fd >= 0) { dump_report(fd, name); close(fd); }
    }

    /* restore default disposition and re-raise: preserve exit code / core */
    signal(sig, SIG_DFL);
    raise(sig);
}

static void install_handlers(void) {
    static char altstack[SIGSTKSZ < 65536 ? 65536 : SIGSTKSZ];
    stack_t ss = { .ss_sp = altstack, .ss_size = sizeof(altstack), .ss_flags = 0 };
    sigaltstack(&ss, NULL);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = crash_handler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    int sigs[] = { SIGSEGV, SIGABRT, SIGFPE, SIGBUS };
    for (int i = 0; i < 4; i++) sigaction(sigs[i], &sa, NULL);

    /* pre-load libgcc unwinder so backtrace() in the handler is safe */
    void *bt[4]; (void)backtrace(bt, 4);
}

static pthread_once_t g_once = PTHREAD_ONCE_INIT;

static void build_crash_path(void) {
    const char *dir = getenv("NLE_CRASH_DIR");
    if (!dir || !dir[0]) dir = ".";
    /* g_crash_path = "<dir>/nle_crash_<pid>.txt" (built once, plain snprintf ok here) */
    snprintf(g_crash_path, sizeof(g_crash_path), "%s/nle_crash_%d.txt", dir, (int)getpid());
}

static void global_init_once(void) {
    build_crash_path();
    install_handlers();
    /* watchdog is started here in Task 3 */
}

void nle_sentinel_global_init(void) {
    pthread_once(&g_once, global_init_once);
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run:
```bash
gcc -O2 -std=c11 tests/test_sentinel_crash.c src/src/nle_sentinel.c -Isrc/include -lpthread -rdynamic -o /tmp/t_crash && /tmp/t_crash
```
Expected: `test_sentinel_crash: PASS`
(Note: `-rdynamic` gives symbol names in the test's own backtrace; the production libnethack build does not require it — addresses are symbolized offline with `addr2line`.)

- [ ] **Step 5: Commit**

```bash
git add src/src/nle_sentinel.c tests/test_sentinel_crash.c
git commit -m "feat(sentinel): async-signal-safe crash handler with per-env attribution"
```

---

## Task 3: Opt-in watchdog thread for hangs

**Files:**
- Modify: `src/src/nle_sentinel.c`
- Test: `tests/test_sentinel_watchdog.c`

- [ ] **Step 1: Write the failing watchdog test**

Create `tests/test_sentinel_watchdog.c`:

```c
/* Child sets NLE_WATCHDOG_SECS=1, registers an env, beats a few times, then
 * stops beating (sleeps). The watchdog must detect the stall and abort the
 * process. Parent asserts the child died of SIGABRT and a breadcrumb mentioning
 * a hang/stall was written. */
#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "nle_sentinel.h"

int main(void) {
    char dir[] = "/tmp/nle-wd-XXXXXX";
    assert(mkdtemp(dir));
    setenv("NLE_CRASH_DIR", dir, 1);
    setenv("NLE_WATCHDOG_SECS", "1", 1);

    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        nle_sentinel_global_init();
        void *s = nle_sentinel_register(0x1234);
        for (int i = 0; i < 3; i++) { nle_sentinel_beat(s, 'k', 1); usleep(100000); }
        for (;;) pause(); /* stop beating -> hang */
    }

    int status = 0;
    waitpid(pid, &status, 0);
    assert(WIFSIGNALED(status));
    assert(WTERMSIG(status) == SIGABRT); /* watchdog aborted */

    char path[512], buf[8192];
    snprintf(path, sizeof(path), "%s/nle_crash_%d.txt", dir, pid);
    FILE *f = fopen(path, "r");
    assert(f && "watchdog breadcrumb not written");
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    fclose(f);
    assert(strstr(buf, "HANG") || strstr(buf, "SIGABRT"));

    printf("test_sentinel_watchdog: PASS\n");
    return 0;
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run:
```bash
gcc -O2 -std=c11 tests/test_sentinel_watchdog.c src/src/nle_sentinel.c -Isrc/include -lpthread -o /tmp/t_wd && timeout 20 /tmp/t_wd
```
Expected: FAIL — the child hangs forever (no watchdog), so `timeout 20` kills the test (nonzero). The assertions are never reached.

- [ ] **Step 3: Implement the watchdog**

In `src/src/nle_sentinel.c`, add a watchdog thread. Add this function above `global_init_once`:

```c
static void *watchdog_main(void *arg) {
    unsigned long secs = (unsigned long)(long)arg;
    unsigned long half_us = (secs * 1000000UL) / 2;
    if (half_us < 100000UL) half_us = 100000UL;

    uint64_t last = nle_sentinel_total_heartbeat();
    unsigned long stalled_us = 0;
    for (;;) {
        usleep(half_us);
        uint64_t now = nle_sentinel_total_heartbeat();
        if (now != last) { last = now; stalled_us = 0; continue; }
        /* no live envs yet? don't count it as a stall */
        if (now == 0) { continue; }
        stalled_us += half_us;
        if (stalled_us >= secs * 1000000UL) {
            const char *name = "HANG (watchdog)";
            dump_report(STDERR_FILENO, name);
            if (g_crash_path[0]) {
                int fd = open(g_crash_path, O_CREAT | O_WRONLY | O_APPEND, 0644);
                if (fd >= 0) { dump_report(fd, name); close(fd); }
            }
            abort(); /* caught by the SIGABRT handler too; loud + attributed */
        }
    }
    return NULL;
}
```

Then, in `global_init_once`, replace the `/* watchdog is started here in Task 3 */` comment with:

```c
    const char *wd = getenv("NLE_WATCHDOG_SECS");
    if (wd && wd[0]) {
        long secs = atol(wd);
        if (secs > 0) {
            pthread_t th;
            if (pthread_create(&th, NULL, watchdog_main, (void *)(long)secs) == 0)
                pthread_detach(th);
        }
    }
```

- [ ] **Step 4: Run the test to verify it passes**

Run:
```bash
gcc -O2 -std=c11 tests/test_sentinel_watchdog.c src/src/nle_sentinel.c -Isrc/include -lpthread -o /tmp/t_wd && timeout 20 /tmp/t_wd
```
Expected: `test_sentinel_watchdog: PASS` (within a few seconds, well under the 20s timeout).

- [ ] **Step 5: Commit**

```bash
git add src/src/nle_sentinel.c tests/test_sentinel_watchdog.c
git commit -m "feat(sentinel): opt-in NLE_WATCHDOG_SECS hang watchdog"
```

---

## Task 4: Wire the sentinel into the NLE step lifecycle

**Files:**
- Modify: `src/src/nle.c` (`nle_start` ~709, `nle_step` ~864, `nle_end` ~988)
- Modify: `src/CMakeLists.txt:171`

- [ ] **Step 1: Include the header in `nle.c`**

At the top of `src/src/nle.c`, with the other includes, add:

```c
#include "nle_sentinel.h"
```

- [ ] **Step 2: Register in `nle_start`**

In `src/src/nle.c`, inside `nle_start(...)`, after the `nle_ctx_t *nle` for the env is allocated/anchored and the seeds are known (locate the line that returns/has the fully-built `nle`; place this just before the function returns `nle`), add:

```c
    nle_sentinel_global_init();
    /* seed_init->seeds[0] is the primary dungeon seed for this env */
    nle->sentinel = nle_sentinel_register(
        seed_init ? (unsigned long)seed_init->seeds[0] : 0UL);
```

(If `seed_init` is not in scope at the return site, capture `seed_init->seeds[0]` into a local `unsigned long` near the top of `nle_start` and use that local here.)

- [ ] **Step 3: Beat in `nle_step`**

In `src/src/nle.c`, at the very top of `nle_step(nle_ctx_t *nle, nle_obs *obs)` (line ~864), before any work, add:

```c
    nle_sentinel_beat(nle->sentinel, obs->action,
                      (int)obs->blstats[NLE_BL_DEPTH]);
```

`NLE_BL_DEPTH` is defined in `nleobs.h` (already included transitively; if the compiler complains, add `#include "nleobs.h"`).

- [ ] **Step 4: Unregister in `nle_end`**

In `src/src/nle.c`, inside `nle_end(nle_ctx_t *nle)` (line ~988), at the very start (before any teardown that frees `nle`), add:

```c
    nle_sentinel_unregister(nle->sentinel);
    nle->sentinel = NULL;
```

- [ ] **Step 5: Link Threads into the nethack library**

In `src/CMakeLists.txt`, find line 171:

```cmake
target_link_libraries(nethack PUBLIC m fcontext bz2 tmt)
```

Change it to:

```cmake
find_package(Threads REQUIRED)
target_link_libraries(nethack PUBLIC m fcontext bz2 tmt Threads::Threads)
```

- [ ] **Step 6: Rebuild libnethack and verify the existing multi-env harness still passes**

Run:
```bash
cmake -S src -B src/build >/dev/null && cmake --build src/build --target nethack -j8 2>&1 | tail -5
NETHACKDIR=$PWD/src/build/dat ./test_multi_env 64 500 4 2>&1 | tail -5
```
Expected: build succeeds; `test_multi_env` completes with no crash (it still lives at repo root until Task 6).

- [ ] **Step 7: Commit**

```bash
git add src/src/nle.c src/CMakeLists.txt
git commit -m "feat(sentinel): hook register/beat/unregister into nle_start/step/end"
```

---

## Task 5: Capture the panic reason in `end.c`

**Files:**
- Modify: `src/src/end.c` (`panic()` ~613-690)

- [ ] **Step 1: Include the header in `end.c`**

At the top of `src/src/end.c`, with the other includes, add:

```c
#include "nle_sentinel.h"
```

- [ ] **Step 2: Record the message before the game-end path**

In `src/src/end.c`, inside `panic(VA_DECL(const char *, str))`, the message is formatted with `Vsprintf(pbuf, ...)` into a local buffer (commonly `pbuf`) before output. Immediately after that buffer is filled and before the `really_done(PANICKED);` / `NH_abort();` calls, add:

```c
    /* Attribute the panic reason to this env's sentinel slot (NULL slot ok). */
    nle_sentinel_set_panic(current_nle_ctx ? current_nle_ctx->sentinel : NULL, pbuf);
```

(If the local buffer has a different name in this fork, use that name. `nle_sentinel_set_panic` falls back to the current thread's slot when passed NULL, so even `nle_sentinel_set_panic(NULL, pbuf)` is correct and safe.)

- [ ] **Step 3: Rebuild and smoke-test**

Run:
```bash
cmake --build src/build --target nethack -j8 2>&1 | tail -3
NETHACKDIR=$PWD/src/build/dat ./test_multi_env 32 300 4 2>&1 | tail -3
```
Expected: build succeeds; no crash. (Panics are rare; this only adds a string copy on the panic path.)

- [ ] **Step 4: Commit**

```bash
git add src/src/end.c
git commit -m "feat(sentinel): record panic reason for crash breadcrumbs"
```

---

## Task 6: Consolidate tests/ and add the tiered no-hang soak

**Files:**
- Move: `test_multi_env.c`, `verify_determinism.c`, `verify_determinism_all.sh`, `golden/` → `tests/`
- Create: `tests/test_no_hang.c`, `tests/run_tests.sh`, `tests/README.md`

- [ ] **Step 1: Move existing harnesses into tests/ (preserve history)**

Run:
```bash
mkdir -p tests
git mv test_multi_env.c tests/test_multi_env.c
git mv verify_determinism.c tests/verify_determinism.c
git mv verify_determinism_all.sh tests/verify_determinism_all.sh
git mv golden tests/golden
git status --short
```
Expected: the four entries show as renames (`R`).

- [ ] **Step 2: Fix relative paths in the moved determinism script**

`tests/verify_determinism_all.sh` and the moved `.c` files reference build/data paths relative to the repo root. Open `tests/verify_determinism_all.sh` and prefix any relative `src/build`, `src/build/dat`, or `./verify_determinism` paths with the repo root so they work from `tests/`. The robust pattern — add near the top of the script (after the shebang):

```bash
# Resolve repo root so the script works regardless of CWD.
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
```
Then leave the existing `src/build...` paths as-is (they are now relative to `$ROOT`). Do the same `cd "$ROOT"` idiom in any other moved shell script.

- [ ] **Step 3: Write the tiered no-hang soak test**

Create `tests/test_no_hang.c`:

```c
/* test_no_hang.c — proves NLE makes forward progress over time under scale.
 *
 * Drives N envs across T OS threads. A monitor polls the sentinel snapshot once
 * per second; if any live env's heartbeat fails to advance for STALL_SECS, or a
 * crash handler fired, the test prints the offending env and exits 1. If every
 * env keeps advancing for the whole DURATION, it exits 0.
 *
 * Tiers:
 *   (default)  N=64  T=8   DURATION=30s   STALL=5s
 *   --soak     N=1024 T=16 DURATION=180s  STALL=10s   (duration overridable: --soak <secs>)
 *
 * Build: see tests/run_tests.sh
 * Run:   NETHACKDIR=$PWD/src/build/dat ./tests/test_no_hang [--soak [secs]]
 */
#define _GNU_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <omp.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "nleobs.h"
#include "nle_sentinel.h"

typedef struct nle_ctx_t nle_ctx_t;
extern nle_ctx_t *nle_start(nle_obs *, FILE *, nle_seeds_init_t *, nle_settings *);
extern nle_ctx_t *nle_step(nle_ctx_t *, nle_obs *);
extern void nle_end(nle_ctx_t *);

/* ---- per-env scaffolding (mirrors tests/test_multi_env.c) ---- */
typedef struct {
    nle_ctx_t    *ctx;
    nle_obs       obs;
    nle_settings  settings;
    char          vardir[256];
    short         glyphs[21 * 79];
    unsigned char chars[21 * 79];
    unsigned char colors[21 * 79];
    long          blstats[NLE_BLSTATS_SIZE];
    unsigned char message[NLE_MESSAGE_SIZE];
    int           misc[NLE_MISC_SIZE];
    int           internal[NLE_INTERNAL_SIZE];
    unsigned long seed_a, seed_b;
} Env;

static void touch(const char *p) { int fd = open(p, O_CREAT | O_WRONLY, 0644); if (fd >= 0) close(fd); }

static void make_vardir(const char *hackdir, char *out, size_t cap) {
    char tmpl[] = "/tmp/nle-soak-XXXXXX";
    char *d = mkdtemp(tmpl); assert(d);
    snprintf(out, cap, "%s", d);
    char src[512], dst[512];
    snprintf(src, sizeof(src), "%s/nhdat", hackdir);
    snprintf(dst, sizeof(dst), "%s/nhdat", d);
    assert(symlink(src, dst) == 0);
    const char *files[] = {"perm", "record", "logfile", "xlogfile"};
    for (int i = 0; i < 4; i++) { snprintf(dst, sizeof(dst), "%s/%s", d, files[i]); touch(dst); }
    snprintf(dst, sizeof(dst), "%s/save", d); mkdir(dst, 0755);
}

static void env_bind(Env *e) {
    e->obs.glyphs = e->glyphs; e->obs.chars = e->chars; e->obs.colors = e->colors;
    e->obs.blstats = e->blstats; e->obs.message = e->message;
    e->obs.misc = e->misc; e->obs.internal = e->internal;
}

static void env_init(Env *e, int idx, const char *hackdir) {
    memset(e, 0, sizeof(*e));
    make_vardir(hackdir, e->vardir, sizeof(e->vardir));
    strncpy(e->settings.hackdir, e->vardir, sizeof(e->settings.hackdir) - 1);
    strncpy(e->settings.options,
            "name:Agent-mon-hum-neu-mal,autopickup,color,"
            "disclose:+i +a +v +g +c +o,nobones,!status_updates",
            sizeof(e->settings.options) - 1);
    e->settings.spawn_monsters = 1;
    env_bind(e);
    e->seed_a = 0xCAFEBEEF + (unsigned long)idx;
    e->seed_b = (0xCAFEBEEF ^ 0x9E3779B97F4A7C15UL) + (unsigned long)idx;
}

static void env_start(Env *e) {
    e->seed_a = e->seed_a * 6364136223846793005UL + 1442695040888963407UL;
    e->seed_b = e->seed_b * 6364136223846793005UL + 1442695040888963407UL;
    nle_seeds_init_t seeds; memset(&seeds, 0, sizeof(seeds));
    seeds.seeds[0] = e->seed_a; seeds.seeds[1] = e->seed_b; seeds.reseed = 0;
    e->obs.action = 0; e->obs.done = 0;
    e->ctx = nle_start(&e->obs, NULL, &seeds, &e->settings);
    assert(e->ctx);
}

/* ---- monitor: shared stop flag ---- */
static atomic_int g_stop = 0;     /* set when duration elapses */
static atomic_int g_failed = 0;   /* set by monitor on a detected stall */

int main(int argc, char **argv) {
    int   n_envs = 64, n_threads = 8, duration = 30, stall = 5;
    if (argc > 1 && strcmp(argv[1], "--soak") == 0) {
        n_envs = 1024; n_threads = 16; duration = 180; stall = 10;
        if (argc > 2) duration = atoi(argv[2]);
    }

    const char *hackdir = getenv("NETHACKDIR");
    if (!hackdir) hackdir = "src/build/dat";
    char abs_hackdir[512];
    if (hackdir[0] != '/') {
        char cwd[256]; assert(getcwd(cwd, sizeof(cwd)));
        snprintf(abs_hackdir, sizeof(abs_hackdir), "%s/%s", cwd, hackdir);
        hackdir = abs_hackdir;
    }

    printf("test_no_hang: N=%d threads=%d duration=%ds stall_threshold=%ds\n",
           n_envs, n_threads, duration, stall);

    nle_sentinel_global_init();
    omp_set_num_threads(n_threads);

    Env *envs = calloc(n_envs, sizeof(Env));
    assert(envs);
    for (int i = 0; i < n_envs; i++) env_init(&envs[i], i, hackdir);
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < n_envs; i++) env_start(&envs[i]);

    int actions[] = {'k','j','h','l','y','u','b','n','.','s'};
    int n_actions = (int)(sizeof(actions) / sizeof(actions[0]));

    double t_start = omp_get_wtime();

    /* Worker region: keep stepping every env until g_stop. One OMP team; each
     * env is stepped round-robin by its owning thread. */
    #pragma omp parallel
    {
        #pragma omp for schedule(static) nowait
        for (int i = 0; i < n_envs; i++) {
            unsigned int rng = (unsigned int)(i * 7 + 1);
            while (!atomic_load(&g_stop) && !atomic_load(&g_failed)) {
                rng = rng * 1103515245 + 12345;
                envs[i].obs.action = actions[rng % n_actions];
                envs[i].ctx = nle_step(envs[i].ctx, &envs[i].obs);
                if (envs[i].obs.done) {
                    nle_end(envs[i].ctx);
                    env_bind(&envs[i]);
                    env_start(&envs[i]);
                }
            }
        }

        /* Thread 0 is the monitor: poll snapshots once per second. */
        #pragma omp single nowait
        {
            int n_snap = n_envs + 16;
            nle_sentinel_stat *prev = calloc(n_snap, sizeof(nle_sentinel_stat));
            nle_sentinel_stat *cur  = calloc(n_snap, sizeof(nle_sentinel_stat));
            int *stalled_for = calloc(n_snap, sizeof(int)); /* by env_id */
            int *prev_hb_idx = NULL; (void)prev_hb_idx;
            uint64_t *prev_hb = calloc(NLE_SENTINEL_CAP, sizeof(uint64_t));

            while (omp_get_wtime() - t_start < duration && !atomic_load(&g_failed)) {
                sleep(1);
                int n = nle_sentinel_snapshot(cur, n_snap);
                for (int i = 0; i < n; i++) {
                    int id = cur[i].env_id;
                    if (cur[i].heartbeat == prev_hb[id]) {
                        stalled_for[id] += 1;
                        if (stalled_for[id] >= stall) {
                            fprintf(stderr,
                                "FAIL: env id=%d seed=0x%lx stalled %ds at step=%llu action=%d dlvl=%d\n",
                                id, cur[i].seed, stalled_for[id],
                                (unsigned long long)cur[i].step, cur[i].last_action, cur[i].dlvl);
                            atomic_store(&g_failed, 1);
                            break;
                        }
                    } else {
                        stalled_for[id] = 0;
                        prev_hb[id] = cur[i].heartbeat;
                    }
                }
            }
            atomic_store(&g_stop, 1);
            free(prev); free(cur); free(stalled_for); free(prev_hb);
        }
    }

    atomic_store(&g_stop, 1);

    /* Summary */
    nle_sentinel_stat *snap = calloc(n_envs + 16, sizeof(nle_sentinel_stat));
    int n = nle_sentinel_snapshot(snap, n_envs + 16);
    uint64_t total = 0, mn = (uint64_t)-1, mx = 0;
    for (int i = 0; i < n; i++) {
        total += snap[i].step;
        if (snap[i].step < mn) mn = snap[i].step;
        if (snap[i].step > mx) mx = snap[i].step;
    }
    double elapsed = omp_get_wtime() - t_start;
    printf("envs_live=%d total_steps=%llu min_steps=%llu max_steps=%llu steps/sec=%.0f elapsed=%.1fs\n",
           n, (unsigned long long)total, (unsigned long long)(n ? mn : 0),
           (unsigned long long)mx, total / (elapsed > 0 ? elapsed : 1), elapsed);
    free(snap);

    if (atomic_load(&g_failed)) { printf("test_no_hang: FAIL\n"); return 1; }
    printf("test_no_hang: PASS\n");
    return 0;
}
```

- [ ] **Step 4: Write the suite runner**

Create `tests/run_tests.sh`:

```bash
#!/bin/bash
# Build libnethack + all sentinel/integration tests, run the suite.
# Usage: tests/run_tests.sh [--soak [secs]]
set -uo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

SOAK_ARGS=()
if [ "${1:-}" = "--soak" ]; then SOAK_ARGS=(--soak "${2:-}"); fi

export NETHACKDIR="$ROOT/src/build/dat"
INC="-Isrc/include"
LIB="-Lsrc/build -lnethack -Wl,-rpath,src/build -lm -lpthread"
fail=0
run() { echo "=== $1 ==="; shift; "$@"; if [ $? -ne 0 ]; then echo "  -> FAILED"; fail=1; else echo "  -> ok"; fi; }

echo "### building libnethack"
cmake -S src -B src/build >/dev/null && cmake --build src/build --target nethack -j8 >/dev/null || { echo "build failed"; exit 1; }

echo "### standalone unit tests (no libnethack needed)"
gcc -O2 -std=c11 tests/test_sentinel_unit.c     src/src/nle_sentinel.c $INC -lpthread -o /tmp/t_unit  && run "sentinel_unit"     /tmp/t_unit
gcc -O2 -std=c11 tests/test_sentinel_crash.c    src/src/nle_sentinel.c $INC -lpthread -rdynamic -o /tmp/t_crash && run "sentinel_crash" /tmp/t_crash
gcc -O2 -std=c11 tests/test_sentinel_watchdog.c src/src/nle_sentinel.c $INC -lpthread -o /tmp/t_wd    && run "sentinel_watchdog" timeout 30 /tmp/t_wd

echo "### integration tests (link libnethack)"
gcc -O2 -fopenmp -DNLE_ALLOW_SEEDING=1 tests/test_multi_env.c $INC $LIB -o /tmp/t_multi && run "multi_env" /tmp/t_multi 64 500 4
gcc -O2 -fopenmp tests/test_no_hang.c src/src/nle_sentinel.c $INC $LIB -o /tmp/t_nohang && run "no_hang" /tmp/t_nohang "${SOAK_ARGS[@]}"

echo "### determinism"
run "determinism" bash tests/verify_determinism_all.sh

if [ $fail -ne 0 ]; then echo "SUITE: FAIL"; exit 1; fi
echo "SUITE: PASS"
```

Make it executable:
```bash
chmod +x tests/run_tests.sh
```

- [ ] **Step 5: Write the README**

Create `tests/README.md`:

```markdown
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
dumps all envs' vital signs and aborts loudly (instead of hanging forever).
```

- [ ] **Step 6: Run the full suite**

Run:
```bash
tests/run_tests.sh
```
Expected: each test prints `-> ok`, final line `SUITE: PASS`.

- [ ] **Step 7: Run the soak briefly to confirm the flag works**

Run:
```bash
NETHACKDIR=$PWD/src/build/dat /tmp/t_nohang --soak 20
```
Expected: `test_no_hang: PASS` with `N=1024 threads=16` and a nonzero `steps/sec`.

- [ ] **Step 8: Commit**

```bash
git add tests/
git commit -m "test: consolidate harnesses into tests/ and add tiered no-hang soak"
```

---

## Self-Review

**Spec coverage:**
- Per-env vital signs registry → Task 1 ✓
- `nle_sentinel_beat` hot path → Task 1 (impl) + Task 4 (wired into `nle_step`) ✓
- Async-signal-safe signal handler + breadcrumb → Task 2 ✓
- Opt-in `NLE_WATCHDOG_SECS` watchdog → Task 3 ✓
- `nle_ctx` field + lifecycle hooks → Task 1 (field) + Task 4 (start/step/end) ✓
- Panic reason capture in `end.c` → Task 5 ✓
- Snapshot/total-heartbeat read API → Task 1 ✓
- `tests/` consolidation + tiered no-hang soak + runner + README → Task 6 ✓
- 16384 cap → Task 1 header ✓; watchdog off by default → Task 3 (gated on env var) ✓; soak a few minutes → Task 6 default 180s ✓
- Build picks up new `src/src/*.c` via glob; Threads linked → Task 4 ✓

**Placeholder scan:** No TBD/TODO; every code step contains complete code. The two "if the local buffer has a different name" notes (Task 4 Step 2, Task 5 Step 2) are fork-specific fallbacks with a safe default given, not placeholders.

**Type consistency:** `nle_sentinel_stat` fields (`env_id, seed, step, heartbeat, last_action, dlvl, in_use`) used identically in Tasks 1 and 6. `nle_sentinel_beat(void*, int, int)`, `nle_sentinel_register(unsigned long)`, `nle_sentinel_snapshot(stat*, int)`, `nle_sentinel_total_heartbeat(void)` signatures consistent across header, impl, and all call sites. `NLE_BL_DEPTH` (12) and `NLE_SENTINEL_CAP` (16384) used consistently.
