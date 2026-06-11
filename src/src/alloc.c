/* NetHack 3.6	alloc.c	$NHDT-Date: 1454376505 2016/02/02 01:28:25 $  $NHDT-Branch: NetHack-3.6.0 $:$NHDT-Revision: 1.16 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * NLE fast-reset variant: NetHack's allocator is replaced with a bump-pointer
 * arena allocator. All allocations live in a single contiguous region, so a
 * memcpy of that region trivially captures the entire NetHack heap state.
 *
 * free() is redirected (via global.h macro) to nle_arena_free(), which is a
 * no-op for arena pointers. Memory is reclaimed only at snapshot-restore
 * time, when the bump pointer rewinds to its saved position. Within a single
 * "episode" the arena grows monotonically; on restore it shrinks back to the
 * snapshot watermark.
 *
 * Non-arena pointers (e.g. from libc strdup() called in a save-recovery
 * path) are forwarded to libc free().
 */

#define ALLOC_C /* comment line for pre-compiled headers */
#define EXTERN_H /* comment line for pre-compiled headers */
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

char *FDECL(fmt_ptr, (const genericptr));

long *FDECL(alloc, (unsigned int));
extern void VDECL(panic, (const char *, ...)) PRINTF_F(1, 2);

#ifdef NLE_USE_ARENA_FREE
#include <sys/mman.h>
#include "nle.h"

/* Per-env bump arena. Each env owns its own mmap'd arena on
 * nle_ctx_t (s_arena_base / s_arena_used / s_arena_cap), lazily allocated
 * on first alloc() call where current_nle_ctx is non-NULL. The previous
 * design used a single 16 GB process-wide arena guarded by
 * __sync_fetch_and_add — that atomic was the last serializing primitive
 * in the NetHack hot path under multi-env training. Per-env arenas remove
 * it entirely: each env's coroutine is the sole writer of its own arena.
 *
 * Allocations made before current_nle_ctx is anchored (very early process
 * init, before any env's mainloop runs) fall back to the legacy file-scope
 * arena below. That fallback is also what nle_fast_reset.c references for
 * its (dead-at-runtime; NETHACK_FAST_RESET=0) snapshot/restore code.
 *
 * MAP_NORESERVE keeps the kernel from over-counting commit; madvise
 * DONTDUMP keeps un-touched pages out of cores.
 */
#ifndef NLE_PER_ENV_ARENA_SIZE
#define NLE_PER_ENV_ARENA_SIZE ((size_t) 64 * 1024 * 1024)
#endif
#ifndef NLE_ARENA_SIZE_GB
#define NLE_ARENA_SIZE_GB 16
#endif
#define NLE_LEGACY_ARENA_SIZE ((size_t) NLE_ARENA_SIZE_GB * 1024 * 1024 * 1024)
#define NLE_ARENA_ALIGN 16

/* Legacy fallback arena. Used only for allocations made before
 * current_nle_ctx is set (early process init) and by nle_fast_reset.c
 * (dead at runtime when NETHACK_FAST_RESET=0). Lazily mapped on first
 * use. Exported (non-static) so nle_fast_reset.c can still reference it. */
char  *nle_arena_base = NULL;
size_t nle_arena_used = 0;
size_t nle_arena_cap  = 0;

/* Global registry of live per-env arena ranges. nle_arena_free needs to
 * recognise pointers that came from ANY env's arena — not just the
 * current one — because process-global state (e.g. sysopt) is populated
 * by env A's arena (via dupstr) and later freed by env B during its
 * teardown. Without a global registry the free would fall through to
 * __libc_free and crash.
 *
 * Slots are written exactly once on first per-env mmap (publish via
 * __atomic_store with release), and zeroed on munmap (env teardown).
 * Lookups walk linearly with acquire loads — no lock, no contention on
 * the hot alloc() path (only nle_arena_free pays the cost). Capacity
 * 4096 is large vs the realistic env count (~1024). */
#define NLE_ARENA_REGISTRY_CAP 32768
static char  *nle_arena_registry_base[NLE_ARENA_REGISTRY_CAP];
static size_t nle_arena_registry_cap_bytes[NLE_ARENA_REGISTRY_CAP];
/* High-water mark: max+1 index ever assigned. Bounds the linear scan in
 * nle_arena_registry_contains so we don't walk 4096 slots when only a
 * few are live. Monotonically grows; ok to slightly overshoot. */
static int nle_arena_registry_hwm = 0;

static void
nle_arena_registry_add(char *base, size_t cap)
{
    for (int i = 0; i < NLE_ARENA_REGISTRY_CAP; i++) {
        char *expected = NULL;
        if (__atomic_load_n(&nle_arena_registry_base[i], __ATOMIC_ACQUIRE)
            != NULL)
            continue;
        if (__atomic_compare_exchange_n(&nle_arena_registry_base[i],
                                        &expected, base, 0,
                                        __ATOMIC_ACQ_REL,
                                        __ATOMIC_ACQUIRE)) {
            __atomic_store_n(&nle_arena_registry_cap_bytes[i], cap,
                             __ATOMIC_RELEASE);
            int hwm;
            do {
                hwm = __atomic_load_n(&nle_arena_registry_hwm,
                                      __ATOMIC_ACQUIRE);
                if (i + 1 <= hwm)
                    break;
            } while (!__atomic_compare_exchange_n(&nle_arena_registry_hwm,
                                                  &hwm, i + 1, 0,
                                                  __ATOMIC_ACQ_REL,
                                                  __ATOMIC_ACQUIRE));
            return;
        }
    }
    /* Registry full — extremely unlikely under realistic env counts. */
    panic("nle_arena: registry overflow (cap=%d)",
          NLE_ARENA_REGISTRY_CAP);
}

static void
nle_arena_registry_remove(char *base)
{
    for (int i = 0; i < NLE_ARENA_REGISTRY_CAP; i++) {
        if (__atomic_load_n(&nle_arena_registry_base[i], __ATOMIC_ACQUIRE)
            == base) {
            __atomic_store_n(&nle_arena_registry_cap_bytes[i], 0,
                             __ATOMIC_RELEASE);
            __atomic_store_n(&nle_arena_registry_base[i], NULL,
                             __ATOMIC_RELEASE);
            return;
        }
    }
}

static int
nle_arena_registry_contains(const void *ptr)
{
    int hwm = __atomic_load_n(&nle_arena_registry_hwm, __ATOMIC_ACQUIRE);
    for (int i = 0; i < hwm; i++) {
        char *base = __atomic_load_n(&nle_arena_registry_base[i],
                                     __ATOMIC_ACQUIRE);
        if (!base)
            continue;
        size_t cap = __atomic_load_n(&nle_arena_registry_cap_bytes[i],
                                     __ATOMIC_ACQUIRE);
        if ((const char *) ptr >= base
            && (const char *) ptr <  base + cap)
            return 1;
    }
    return 0;
}

/* Exposed so nle.c's nle_end can unregister a per-env arena before
 * munmap'ing it. */
void
nle_arena_registry_release(char *base)
{
    nle_arena_registry_remove(base);
}

static void
nle_arena_legacy_init(void)
{
    if (nle_arena_base)
        return;
    void *p = mmap(NULL, NLE_LEGACY_ARENA_SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "nle_arena: legacy mmap(%zu) failed\n",
                NLE_LEGACY_ARENA_SIZE);
        abort();
    }
#ifdef MADV_DONTDUMP
    (void) madvise(p, NLE_LEGACY_ARENA_SIZE, MADV_DONTDUMP);
#endif
    nle_arena_base = (char *) p;
    nle_arena_used = 0;
    nle_arena_cap  = NLE_LEGACY_ARENA_SIZE;
}

static void
nle_arena_per_env_init(nle_ctx_t *ctx)
{
    if (ctx->s_arena_base)
        return;
    void *p = mmap(NULL, NLE_PER_ENV_ARENA_SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "nle_arena: per-env mmap(%zu) failed\n",
                NLE_PER_ENV_ARENA_SIZE);
        abort();
    }
#ifdef MADV_DONTDUMP
    (void) madvise(p, NLE_PER_ENV_ARENA_SIZE, MADV_DONTDUMP);
#endif
    ctx->s_arena_base = (char *) p;
    ctx->s_arena_used = 0;
    ctx->s_arena_cap  = NLE_PER_ENV_ARENA_SIZE;
    /* Register so nle_arena_free can recognise this arena's pointers
     * when called from a different env's coroutine (e.g. sysopt strings
     * dup'd by env A and freed during env B's nle_end). */
    nle_arena_registry_add(ctx->s_arena_base, ctx->s_arena_cap);
}

long *
alloc(lth)
register unsigned int lth;
{
    size_t need = (lth + NLE_ARENA_ALIGN - 1) & ~(size_t)(NLE_ARENA_ALIGN - 1);
    if (need == 0)
        need = NLE_ARENA_ALIGN;

    /* Fast path: current_nle_ctx is anchored to the active env. Bump its
     * private arena — no atomic, no contention. */
    nle_ctx_t *ctx = current_nle_ctx;
    if (ctx) {
        if (!ctx->s_arena_base)
            nle_arena_per_env_init(ctx);
        size_t offset = ctx->s_arena_used;
        if (offset + need > ctx->s_arena_cap) {
            panic("nle_arena: per-env out of memory "
                  "(used=%zu + req=%zu > cap=%zu)",
                  offset, need, ctx->s_arena_cap);
        }
        ctx->s_arena_used = offset + need;
        return (long *) (ctx->s_arena_base + offset);
    }

    /* Fallback: very early process init, before any env has anchored
     * current_nle_ctx. Use the legacy process-wide arena. Single-threaded
     * by construction at this point — no atomic needed. */
    if (!nle_arena_base)
        nle_arena_legacy_init();
    size_t offset = nle_arena_used;
    if (offset + need > nle_arena_cap) {
        panic("nle_arena: legacy out of memory "
              "(used=%zu + req=%zu > cap=%zu)",
              offset, need, nle_arena_cap);
    }
    nle_arena_used = offset + need;
    return (long *) (nle_arena_base + offset);
}

/* Called by NetHack code via the `free` macro in global.h (non-MONITOR_HEAP
 * branch). Pointers inside any arena (current env's or legacy fallback)
 * are no-ops; everything else (rare, e.g. libc strdup in save recovery)
 * is forwarded to libc free. */
void
nle_arena_free(void *ptr)
{
    if (!ptr)
        return;
    nle_ctx_t *ctx = current_nle_ctx;
    /* Fast path: current env's own arena (avoids walking the registry
     * for the overwhelmingly common case). */
    if (ctx && ctx->s_arena_base
        && (char *) ptr >= ctx->s_arena_base
        && (char *) ptr <  ctx->s_arena_base + ctx->s_arena_cap) {
        return;
    }
    /* Legacy fallback arena. */
    if (nle_arena_base
        && (char *) ptr >= nle_arena_base
        && (char *) ptr <  nle_arena_base + nle_arena_cap) {
        return;
    }
    /* Some other env's arena? sysopt strings et al. are dup'd into env A's
     * arena and may be freed during env B's teardown — must recognise
     * them as arena pointers (no-op), not libc free. */
    if (nle_arena_registry_contains(ptr))
        return;
    /* Non-arena pointer: forward to libc free. Use __libc_free to bypass
     * the `free` macro from global.h. */
    extern void __libc_free(void *);
    __libc_free(ptr);
}

#else /* !NLE_USE_ARENA_FREE */

/* Util binaries (makedefs, dgn_comp, lev_comp, dlb) reuse this file but link
 * with libc free. Provide the original libc-malloc-based alloc(). */
long *
alloc(lth)
register unsigned int lth;
{
#ifdef LINT
    long dummy = ftell(stderr);
    if (lth)
        dummy = 0;
    return &dummy;
#else
    register genericptr_t ptr;
    ptr = malloc(lth);
    if (!ptr)
        panic("Memory allocation failure; cannot get %u bytes", lth);
    return (long *) ptr;
#endif
}

#endif /* NLE_USE_ARENA_FREE */

/* calloc()-equivalent that routes through alloc() so the allocation lives in
 * the per-env arena and is therefore captured wholesale by nle_fr_snapshot
 * (which memcpy's the arena). init_nle uses this for every per-env heap buffer
 * hanging off nle_ctx_t that must survive snapshot/restore; raw libc calloc()
 * would place them outside the arena and silently drop them from snapshots.
 * alloc() panics on failure, so the result is always non-NULL. Memory is
 * zeroed to preserve calloc semantics regardless of arena page reuse. */
void *
nle_arena_calloc(size_t count, size_t size)
{
    size_t bytes = count * size;
    void *p = (void *) alloc((unsigned int) bytes);
    (void) memset(p, 0, bytes);
    return p;
}

#ifdef HAS_PTR_FMT
#define PTR_FMT "%p"
#define PTR_TYP genericptr_t
#else
#define PTR_FMT "%06lx"
#define PTR_TYP unsigned long
#endif

#define PTRBUFCNT 4
#define PTRBUFSIZ 32
static char ptrbuf[PTRBUFCNT][PTRBUFSIZ];
static __thread int ptrbufidx = 0;

char *
fmt_ptr(ptr)
const genericptr ptr;
{
    char *buf;

    buf = ptrbuf[ptrbufidx];
    if (++ptrbufidx >= PTRBUFCNT)
        ptrbufidx = 0;

    Sprintf(buf, PTR_FMT, (PTR_TYP) ptr);
    return buf;
}

/* strdup() which uses our alloc() rather than libc's malloc(); */
char *
dupstr(string)
const char *string;
{
    return strcpy((char *) alloc(strlen(string) + 1), string);
}

/*alloc.c*/
