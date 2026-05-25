/* nle_fast_reset.c
 *
 * Hacky fast-reset for NLE: snapshot libnethack's writable LOAD segments +
 * the fcontext stack + the nle_ctx_t struct right after the welcome screen
 * has been drained, then on each subsequent reset memcpy them back. Skips
 * the per-reset dlclose+dlopen+nle_start+welcome-drain cycle entirely.
 *
 * Caveats (intentional for this prototype):
 *   - No new dungeon, no new seed: every "reset" plays the SAME initial
 *     game state. That's what the user asked for — we want to measure raw
 *     reset speedup, not vary content.
 *   - We DO NOT call nle_end between snapshot and restores. Any heap
 *     allocations made by NetHack during play are leaked, not freed. This
 *     is required so pointers from the restored .data segment still point
 *     at live (if unused) memory. RSS grows by the per-game live-set per
 *     reset; bounded but real.
 *   - TMT terminal state + outbuf are not snapshotted. NetHack does not
 *     read them back for game logic, only writes display bytes, so this
 *     is observably harmless for the obs path.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <link.h>
#include <elf.h>
#include <unistd.h>

#include "hack.h"
/* nle.h now declares current_nle_ctx as proper `extern` (single
 * definition lives in nle.c, refactor stage 3+ cleanup). */
#include "nle.h"

#define NLE_FR_MAX_SEGS 8

typedef struct nle_fr_segment {
    void  *addr;
    size_t size;
    void  *saved;
} nle_fr_segment;

typedef struct nle_fr_snapshot {
    nle_ctx_t       saved_ctx;
    void           *saved_stack;
    size_t          stack_size;
    int             n_segs;
    nle_fr_segment  segs[NLE_FR_MAX_SEGS];
    /* Arena snapshot: bytes [0, arena_used) of the NetHack heap arena. */
    void           *saved_arena;
    size_t          arena_used;
} nle_fr_snapshot_t;

/* Defined in alloc.c when NLE_USE_ARENA_FREE is set; otherwise provide
 * local stubs so the snapshot/restore code compiles and runs without arena
 * support. */
#ifdef NLE_USE_ARENA_FREE
extern char  *nle_arena_base;
extern size_t nle_arena_used;
extern size_t nle_arena_cap;
#else
static char  *nle_arena_base = NULL;
static size_t nle_arena_used = 0;
static size_t nle_arena_cap  = 0;
#endif

struct fr_phdr_scan {
    void   *probe;
    int     n;
    void   *addr[NLE_FR_MAX_SEGS];
    size_t  size[NLE_FR_MAX_SEGS];
};

static int
fr_phdr_cb(struct dl_phdr_info *info, size_t info_size, void *data)
{
    (void) info_size;
    struct fr_phdr_scan *s = (struct fr_phdr_scan *) data;
    uintptr_t probe = (uintptr_t) s->probe;
    int matched = 0;
    for (int i = 0; i < info->dlpi_phnum; i++) {
        const ElfW(Phdr) *p = &info->dlpi_phdr[i];
        if (p->p_type != PT_LOAD)
            continue;
        uintptr_t a = info->dlpi_addr + p->p_vaddr;
        uintptr_t b = a + p->p_memsz;
        if (probe >= a && probe < b) {
            matched = 1;
            break;
        }
    }
    if (!matched)
        return 0;
    /* Find the GNU_RELRO range first: after dynamic linking, this region
     * within a writable LOAD segment is mprotect-ed read-only, so we MUST
     * skip it to avoid SIGSEGV on restore. */
    const long pagesz = sysconf(_SC_PAGESIZE);
    uintptr_t relro_start = 0, relro_end = 0;
    for (int i = 0; i < info->dlpi_phnum; i++) {
        const ElfW(Phdr) *p = &info->dlpi_phdr[i];
        if (p->p_type == PT_GNU_RELRO) {
            relro_start = info->dlpi_addr + p->p_vaddr;
            relro_end = relro_start + p->p_memsz;
            /* The kernel page-aligns the protected end UP. */
            relro_end = (relro_end + pagesz - 1) & ~(uintptr_t)(pagesz - 1);
            break;
        }
    }
    for (int i = 0; i < info->dlpi_phnum && s->n < NLE_FR_MAX_SEGS; i++) {
        const ElfW(Phdr) *p = &info->dlpi_phdr[i];
        if (p->p_type != PT_LOAD)
            continue;
        if (!(p->p_flags & PF_W))
            continue;
        uintptr_t a = info->dlpi_addr + p->p_vaddr;
        uintptr_t b = a + p->p_memsz;
        /* Trim the RELRO read-only prefix from [a, b). */
        if (relro_start && a >= relro_start && a < relro_end)
            a = relro_end;
        if (a >= b)
            continue;
        s->addr[s->n] = (void *) a;
        s->size[s->n] = (size_t) (b - a);
        s->n++;
    }
    return 1;
}

void *
nle_fr_snapshot(nle_ctx_t *nle)
{
    nle_fr_snapshot_t *s = (nle_fr_snapshot_t *) calloc(1, sizeof(*s));
    if (!s)
        return NULL;

    s->saved_ctx = *nle;
    /* deboost.context stack: sptr is the TOP of the stack (one byte past
     * last writable byte). Stack grows down. The first page from the bottom
     * (sptr - ssize) is mprotect(PROT_NONE) — a guard page. Live bytes are
     * [sptr - ssize + pagesz, sptr). */
    const long pagesz = sysconf(_SC_PAGESIZE);
    s->stack_size = nle->stack.ssize - (size_t) pagesz;
    s->saved_stack = malloc(s->stack_size);
    if (!s->saved_stack) {
        free(s);
        return NULL;
    }
    void *stack_lo = (char *) nle->stack.sptr - s->stack_size;
    memcpy(s->saved_stack, stack_lo, s->stack_size);

    struct fr_phdr_scan scan = { 0 };
    scan.probe = (void *) &nle_fr_snapshot;
    dl_iterate_phdr(fr_phdr_cb, &scan);
    s->n_segs = scan.n;
    for (int i = 0; i < scan.n; i++) {
        s->segs[i].addr = scan.addr[i];
        s->segs[i].size = scan.size[i];
        s->segs[i].saved = malloc(scan.size[i]);
        if (!s->segs[i].saved) {
            for (int j = 0; j < i; j++)
                free(s->segs[j].saved);
            free(s->saved_stack);
            free(s);
            return NULL;
        }
        memcpy(s->segs[i].saved, scan.addr[i], scan.size[i]);
    }

    /* Snapshot the NetHack heap arena. We copy only the used portion. */
    s->arena_used = nle_arena_used;
    if (nle_arena_base && s->arena_used > 0) {
        s->saved_arena = malloc(s->arena_used);
        if (!s->saved_arena) {
            for (int i = 0; i < s->n_segs; i++)
                free(s->segs[i].saved);
            free(s->saved_stack);
            free(s);
            return NULL;
        }
        memcpy(s->saved_arena, nle_arena_base, s->arena_used);
    } else {
        s->saved_arena = NULL;
    }
    return s;
}

void
nle_fr_restore(nle_ctx_t *nle, void *snap)
{
    nle_fr_snapshot_t *s = (nle_fr_snapshot_t *) snap;
    /* Restore arena BEFORE data segments, in case data segment pointers
     * reference into the arena (they do — but order doesn't really matter,
     * since both writes are pure memcpys of independent regions). The
     * bump pointer is rewound, freeing any allocations made after snapshot. */
    if (s->saved_arena && nle_arena_base) {
        memcpy(nle_arena_base, s->saved_arena, s->arena_used);
    }
    nle_arena_used = s->arena_used;

    for (int i = 0; i < s->n_segs; i++)
        memcpy(s->segs[i].addr, s->segs[i].saved, s->segs[i].size);
    {
        void *stack_lo = (char *) nle->stack.sptr - s->stack_size;
        memcpy(stack_lo, s->saved_stack, s->stack_size);
    }
    *nle = s->saved_ctx;
    /* segment memcpy clobbered the global ctx anchor; re-point it. */
    current_nle_ctx = nle;
}

void
nle_fr_destroy(void *snap)
{
    if (!snap)
        return;
    nle_fr_snapshot_t *s = (nle_fr_snapshot_t *) snap;
    for (int i = 0; i < s->n_segs; i++)
        free(s->segs[i].saved);
    free(s->saved_stack);
    free(s->saved_arena);
    free(s);
}
