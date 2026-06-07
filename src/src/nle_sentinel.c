#define _GNU_SOURCE
#include "nle_sentinel.h"

#include <stdatomic.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>

#define NLE_WATCHDOG_MIN_POLL_US 100000UL  /* 100ms minimum watchdog poll interval */

typedef struct {
    _Atomic uint64_t heartbeat;     /* bumped every step                       */
    _Atomic uint64_t step;          /* total steps this env has taken          */
    unsigned long    seed;          /* set once before publish; read after acquire */
    int              env_id;        /* set once before publish                 */
    _Atomic int      last_action;   /* most recent action                      */
    _Atomic int      dlvl;          /* dungeon level as of last beat           */
    pthread_t        thread;        /* thread that last stepped this env       */
    char             panic_msg[160];/* last panic() reason, if any             */
    _Atomic int      in_use;        /* 0 = free, 1 = live, 2 = claiming        */
} sentinel_slot;

static sentinel_slot g_slots[NLE_SENTINEL_CAP];
static __thread sentinel_slot *g_tls; /* slot the current thread is servicing */

void *nle_sentinel_register(unsigned long seed) {
    for (int i = 0; i < NLE_SENTINEL_CAP; i++) {
        int expected = 0;
        if (atomic_compare_exchange_strong_explicit(
                &g_slots[i].in_use, &expected, 2,
                memory_order_acquire, memory_order_relaxed)) {
            sentinel_slot *s = &g_slots[i];
            atomic_store_explicit(&s->heartbeat, 0, memory_order_relaxed);
            atomic_store_explicit(&s->step, 0, memory_order_relaxed);
            atomic_store_explicit(&s->last_action, -1, memory_order_relaxed);
            atomic_store_explicit(&s->dlvl, 0, memory_order_relaxed);
            s->seed = seed;
            s->env_id = i;
            s->thread = 0;
            s->panic_msg[0] = '\0';
            atomic_store_explicit(&s->in_use, 1, memory_order_release); /* publish */
            return s;
        }
    }
    return NULL; /* registry full */
}

void nle_sentinel_beat(void *slot_, int action, int dlvl) {
    sentinel_slot *s = (sentinel_slot *)slot_;
    if (!s) return;
    atomic_store_explicit(&s->last_action, action, memory_order_relaxed);
    atomic_store_explicit(&s->dlvl, dlvl, memory_order_relaxed);
    atomic_store_explicit(&s->step,
        atomic_load_explicit(&s->step, memory_order_relaxed) + 1, memory_order_relaxed);
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
        if (atomic_load_explicit(&g_slots[i].in_use, memory_order_acquire) != 1) continue;
        sentinel_slot *s = &g_slots[i];
        out[n].env_id      = s->env_id;
        out[n].seed        = s->seed;
        out[n].step        = atomic_load_explicit(&s->step, memory_order_relaxed);
        out[n].heartbeat   = atomic_load_explicit(&s->heartbeat, memory_order_relaxed);
        out[n].last_action = atomic_load_explicit(&s->last_action, memory_order_relaxed);
        out[n].dlvl        = atomic_load_explicit(&s->dlvl, memory_order_relaxed);
        out[n].in_use      = 1;
        n++;
    }
    return n;
}

uint64_t nle_sentinel_total_heartbeat(void) {
    uint64_t total = 0;
    for (int i = 0; i < NLE_SENTINEL_CAP; i++) {
        if (atomic_load_explicit(&g_slots[i].in_use, memory_order_acquire) == 1)
            total += atomic_load_explicit(&g_slots[i].heartbeat, memory_order_relaxed);
    }
    return total;
}

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
/* long -> decimal (handles negative), async-signal-safe */
static void sas_write_d(int fd, long v) {
    if (v < 0) { sas_write(fd, "-"); sas_write_u(fd, (unsigned long)(-v)); }
    else sas_write_u(fd, (unsigned long)v);
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
        sas_write(fd, " step="); sas_write_u(fd, (unsigned long)atomic_load_explicit(&cur->step, memory_order_relaxed));
        sas_write(fd, " action="); sas_write_d(fd, (long)atomic_load_explicit(&cur->last_action, memory_order_relaxed));
        sas_write(fd, " dlvl="); sas_write_u(fd, (unsigned long)atomic_load_explicit(&cur->dlvl, memory_order_relaxed));
        sas_write(fd, "\n");
        if (cur->panic_msg[0]) { sas_write(fd, "panic_msg="); sas_write(fd, cur->panic_msg); sas_write(fd, "\n"); }
    } else {
        sas_write(fd, "FAULTING ENV: <unknown thread, no slot>\n");
    }

    sas_write(fd, "--- all envs (id seed step heartbeat dlvl) ---\n");
    for (int i = 0; i < NLE_SENTINEL_CAP; i++) {
        if (atomic_load_explicit(&g_slots[i].in_use, memory_order_acquire) != 1) continue;
        sentinel_slot *s = &g_slots[i];
        sas_write_u(fd, (unsigned long)s->env_id); sas_write(fd, " ");
        sas_write_x(fd, s->seed); sas_write(fd, " ");
        sas_write_u(fd, (unsigned long)atomic_load_explicit(&s->step, memory_order_relaxed)); sas_write(fd, " ");
        sas_write_u(fd, (unsigned long)atomic_load_explicit(&s->heartbeat, memory_order_relaxed)); sas_write(fd, " ");
        sas_write_u(fd, (unsigned long)atomic_load_explicit(&s->dlvl, memory_order_relaxed)); sas_write(fd, "\n");
    }

    sas_write(fd, "--- backtrace ---\n");
    void *bt[64];
    int nb = backtrace(bt, 64);
    backtrace_symbols_fd(bt, nb, fd);
    sas_write(fd, "=== END SENTINEL ===\n");
}

static void crash_handler(int sig, siginfo_t *info, void *uctx) {
    (void)info; (void)uctx;
    if (g_signal_fired) {
        /* A second fault while dumping — don't recurse; leave loudly. */
        signal(sig, SIG_DFL);
        _exit(128 + sig);
    }
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
    static char altstack[65536];
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

static void *watchdog_main(void *arg) {
    /* NOTE: detects a GLOBAL stall only. total_heartbeat() is a sum across all
     * envs, so this fires only when every env stops advancing (e.g. a process-
     * wide deadlock). A single stuck env while others progress will NOT trip it;
     * per-env stall detection is done by the snapshot poll in tests/test_no_hang.c. */
    unsigned long secs = (unsigned long)(long)arg;
    unsigned long half_us = (secs * 1000000UL) / 2;
    if (half_us < NLE_WATCHDOG_MIN_POLL_US) half_us = NLE_WATCHDOG_MIN_POLL_US;

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

static void global_init_once(void) {
    build_crash_path();
    install_handlers();
    const char *wd = getenv("NLE_WATCHDOG_SECS");
    if (wd && wd[0]) {
        long secs = atol(wd);
        if (secs > 0) {
            if (secs > 86400) secs = 86400; /* cap at 1 day; guards secs*1e6 overflow */
            pthread_t th;
            if (pthread_create(&th, NULL, watchdog_main, (void *)(long)secs) == 0)
                pthread_detach(th);
        }
    }
}

void nle_sentinel_global_init(void) {
    pthread_once(&g_once, global_init_once);
}
