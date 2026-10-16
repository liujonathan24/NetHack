/* NetHack 3.6	alloc.c	$NHDT-Date: 1432512769 2015/05/25 00:12:49 $  $NHDT-Branch: master $:$NHDT-Revision: 1.15 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * NLE fast-reset variant: NetHack's allocator is replaced with a bump-pointer
 * arena allocator. All allocations of one environment live in that
 * environment's contiguous arena, so a memcpy of [base, used) captures the
 * whole NetHack heap state of the environment.
 *
 * free() is redirected (via the global.h macro) to nle_arena_free(), which is
 * a no-op for arena pointers. Memory is reclaimed only at snapshot-restore
 * time, when the bump pointer rewinds to its saved position, and when the
 * environment ends (munmap). Non-arena pointers (e.g. from libc strdup()
 * called in a save-recovery path) are forwarded to libc free().
 *
 * The utility programs (makedefs, lev_comp, ...) build this file without
 * NLE_USE_ARENA_FREE and get the plain malloc-based alloc().
 */

#define ALLOC_C /* comment line for pre-compiled headers */
/* since this file is also used in auxiliary programs, don't include all the
   function declarations for all of nethack */
#define EXTERN_H /* comment line for pre-compiled headers */
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *FDECL(fmt_ptr, (const genericptr));

long *FDECL(alloc, (unsigned int));
extern void VDECL(panic, (const char *, ...)) PRINTF_F(1, 2);

#ifdef NLE_USE_ARENA_FREE
#include <sys/mman.h>
#include "nle.h"

/* Per-env bump arena. Each env owns its own mmap'd arena on nle_ctx_t
 * (arena_base / arena_used / arena_cap), lazily mapped on the first alloc()
 * that runs with current_nle_ctx anchored. Each env's coroutine is the sole
 * writer of its own arena, so bumps need no atomics.
 *
 * Allocations made before any env is anchored (very early process init)
 * fall back to a process-wide legacy arena.
 *
 * MAP_NORESERVE keeps the kernel from over-counting commit; madvise
 * DONTDUMP keeps un-touched pages out of cores. */
#ifndef NLE_PER_ENV_ARENA_SIZE
#define NLE_PER_ENV_ARENA_SIZE ((size_t) 64 * 1024 * 1024)
#endif
#ifndef NLE_ARENA_SIZE_GB
#define NLE_ARENA_SIZE_GB 16
#endif
#define NLE_LEGACY_ARENA_SIZE ((size_t) NLE_ARENA_SIZE_GB * 1024 * 1024 * 1024)
#define NLE_ARENA_ALIGN 16

/* Legacy fallback arena (process-level; only touched before the first env
 * exists). Exported for nle_arena_cpp.cc. */
char  *nle_arena_base = NULL;
size_t nle_arena_used = 0;
size_t nle_arena_cap  = 0;

/* Registry of live per-env arena ranges. nle_arena_free needs to recognise
 * pointers that came from ANY env's arena -- not just the current one --
 * because process-global state (e.g. sysopt strings) is populated by env
 * A's arena (via dupstr) and may be freed by env B during its teardown.
 * Without the registry such a free would fall through to __libc_free and
 * crash. Slots are written once on mmap (release), zeroed on munmap;
 * lookups walk linearly with acquire loads -- lock-free, and only the
 * (rare) nle_arena_free of a foreign pointer pays for it. */
#define NLE_ARENA_REGISTRY_CAP 32768
static char  *nle_arena_registry_base[NLE_ARENA_REGISTRY_CAP];
static size_t nle_arena_registry_cap_bytes[NLE_ARENA_REGISTRY_CAP];
/* high-water mark: max+1 slot index ever assigned, bounds the scan */
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
    panic("nle_arena: registry overflow (cap=%d)", NLE_ARENA_REGISTRY_CAP);
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
        if ((const char *) ptr >= base && (const char *) ptr < base + cap)
            return 1;
    }
    return 0;
}

/* nle_end unregisters an env's arena before munmap'ing it. */
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
    if (ctx->arena_base)
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
    ctx->arena_base = (char *) p;
    ctx->arena_used = 0;
    ctx->arena_cap  = NLE_PER_ENV_ARENA_SIZE;
    nle_arena_registry_add(ctx->arena_base, ctx->arena_cap);
}

long *
alloc(lth)
register unsigned int lth;
{
    size_t need = (lth + NLE_ARENA_ALIGN - 1) & ~(size_t) (NLE_ARENA_ALIGN - 1);
    if (need == 0)
        need = NLE_ARENA_ALIGN;

    /* Fast path: bump the anchored env's private arena. */
    nle_ctx_t *ctx = current_nle_ctx;
    if (ctx) {
        if (!ctx->arena_base)
            nle_arena_per_env_init(ctx);
        size_t offset = ctx->arena_used;
        if (offset + need > ctx->arena_cap) {
            panic("nle_arena: per-env out of memory "
                  "(used=%zu + req=%zu > cap=%zu)",
                  offset, need, ctx->arena_cap);
        }
        ctx->arena_used = offset + need;
        return (long *) (ctx->arena_base + offset);
    }

    /* Fallback: before any env is anchored (early process init). */
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

/* Called by NetHack code via the `free` macro in global.h. Pointers inside
 * any arena are no-ops; everything else (rare, e.g. libc strdup in save
 * recovery) is forwarded to libc free. */
void
nle_arena_free(void *ptr)
{
    if (!ptr)
        return;
    nle_ctx_t *ctx = current_nle_ctx;
    if (ctx && ctx->arena_base
        && (char *) ptr >= ctx->arena_base
        && (char *) ptr <  ctx->arena_base + ctx->arena_cap) {
        return;
    }
    if (nle_arena_base
        && (char *) ptr >= nle_arena_base
        && (char *) ptr <  nle_arena_base + nle_arena_cap) {
        return;
    }
    if (nle_arena_registry_contains(ptr))
        return;
    /* Non-arena pointer: forward to libc free, bypassing the `free` macro. */
    extern void __libc_free(void *);
    __libc_free(ptr);
}

#else /* !NLE_USE_ARENA_FREE */

/* Utility binaries (makedefs, dgn_comp, lev_comp, dlb) reuse this file but
 * link with libc free: the original malloc-based alloc(). */
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
 * the per-env arena (and is therefore captured by nle_fr_snapshot). alloc()
 * panics on failure, so the result is always non-NULL. */
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

/* A small pool of static formatting buffers (process-level; debug text
 * only -- see tools/collect_globals/keep.txt). */
#define PTRBUFCNT 4
#define PTRBUFSIZ 32
static char ptrbuf[PTRBUFCNT][PTRBUFSIZ];
static int ptrbufidx = 0;

/* format a pointer for display purposes; returns a static buffer */
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

/* strdup() which uses our alloc() rather than libc's malloc() */
char *
dupstr(string)
const char *string;
{
    return strcpy((char *) alloc(strlen(string) + 1), string);
}

/*alloc.c*/
