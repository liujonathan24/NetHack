#ifndef NLE_SENTINEL_H
#define NLE_SENTINEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Max concurrent envs tracked. Headroom past the N=4096 stress config. */
#define NLE_SENTINEL_CAP 16384

/* Install signal handlers (once) and, iff NLE_WATCHDOG_SECS is set to a
 * positive integer, start the watchdog thread. Idempotent.
 * The watchdog detects a GLOBAL stall only (all envs stuck); per-env stalls are
 * caught by the snapshot API. */
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
