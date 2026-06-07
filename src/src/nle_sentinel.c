#define _GNU_SOURCE
#include "nle_sentinel.h"

#include <stdatomic.h>
#include <string.h>
#include <pthread.h>

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

/* global_init is defined in Task 2 (handler + watchdog). Provide a temporary
 * no-op so Task 1's unit test links; Task 2 replaces this body. */
void nle_sentinel_global_init(void) { }
