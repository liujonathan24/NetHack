/* nle_fast_reset.c — multi-env-safe fast reset
 *
 * Snapshots nle_ctx_t + coroutine stack + per-env arena after the welcome
 * screen is drained, then restores via memcpy on subsequent resets.
 *
 * All mutable game state lives in nle_ctx_t (the global migration is
 * complete), so we no longer save/restore libnethack's LOAD segments.
 * This makes fast reset safe for concurrent multi-env use.
 *
 * Caveat: restores to the same initial game state (same seed/dungeon).
 * For diverse training, use slow reset (nle_end + nle_start).
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "hack.h"
#include "nle.h"

/* rl-port display mirror shims (win/rl/winrl.cc). The NetHackRL instance is
 * libc-malloc'd outside the arena, and its glyph/char/color map arrays are
 * updated incrementally, so they must be captured separately or stale cells
 * from an abandoned branch leak into the next restore's observation. */
extern size_t nle_rl_mirror_size(void);
extern void   nle_rl_mirror_save(nle_ctx_t *nle, void *dst);
extern void   nle_rl_mirror_load(nle_ctx_t *nle, const void *src);

typedef struct nle_fr_snapshot {
    nle_ctx_t       saved_ctx;
    void           *saved_stack;
    size_t          stack_size;
    void           *saved_arena;
    size_t          arena_used;
    void           *saved_mirror;
    size_t          mirror_size;
} nle_fr_snapshot_t;

void *
nle_fr_snapshot(nle_ctx_t *nle)
{
    nle_fr_snapshot_t *s = (nle_fr_snapshot_t *) calloc(1, sizeof(*s));
    if (!s)
        return NULL;

    s->saved_ctx = *nle;

    const long pagesz = sysconf(_SC_PAGESIZE);
    s->stack_size = nle->stack.ssize - (size_t) pagesz;
    s->saved_stack = malloc(s->stack_size);
    if (!s->saved_stack) {
        free(s);
        return NULL;
    }
    void *stack_lo = (char *) nle->stack.sptr - s->stack_size;
    memcpy(s->saved_stack, stack_lo, s->stack_size);

    s->arena_used = nle->s_arena_used;
    if (nle->s_arena_base && s->arena_used > 0) {
        s->saved_arena = malloc(s->arena_used);
        if (!s->saved_arena) {
            free(s->saved_stack);
            free(s);
            return NULL;
        }
        memcpy(s->saved_arena, nle->s_arena_base, s->arena_used);
    } else {
        s->saved_arena = NULL;
    }

    s->mirror_size = nle_rl_mirror_size();
    if (s->mirror_size > 0) {
        s->saved_mirror = malloc(s->mirror_size);
        if (!s->saved_mirror) {
            free(s->saved_arena);
            free(s->saved_stack);
            free(s);
            return NULL;
        }
        nle_rl_mirror_save(nle, s->saved_mirror);
    } else {
        s->saved_mirror = NULL;
    }
    return s;
}

void
nle_fr_restore(nle_ctx_t *nle, void *snap)
{
    nle_fr_snapshot_t *s = (nle_fr_snapshot_t *) snap;

    if (s->saved_arena && nle->s_arena_base) {
        memcpy(nle->s_arena_base, s->saved_arena, s->arena_used);
    }
    nle->s_arena_used = s->arena_used;

    void *stack_lo = (char *) nle->stack.sptr - s->stack_size;
    memcpy(stack_lo, s->saved_stack, s->stack_size);

    char *arena_base = nle->s_arena_base;
    size_t arena_cap = nle->s_arena_cap;
    *nle = s->saved_ctx;
    nle->s_arena_base = arena_base;
    nle->s_arena_cap = arena_cap;
    nle->s_arena_used = s->arena_used;

    current_nle_ctx = nle;

    /* Restore the rl-port display mirror (kept outside the arena). *nle above
     * already restored the s_netHackRL_instance pointer; refill its glyph/char
     * /color map arrays so the next observation has no abandoned-branch residue. */
    if (s->saved_mirror)
        nle_rl_mirror_load(nle, s->saved_mirror);
}

void
nle_fr_destroy(void *snap)
{
    if (!snap)
        return;
    nle_fr_snapshot_t *s = (nle_fr_snapshot_t *) snap;
    free(s->saved_stack);
    free(s->saved_arena);
    free(s->saved_mirror);
    free(s);
}
