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
