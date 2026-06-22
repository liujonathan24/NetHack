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
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "hack.h"
#include "nle.h"

/* rl-port display mirror shims (win/rl/winrl.cc). The NetHackRL instance is
 * libc-malloc'd outside the arena, and its glyph/char/color map arrays are
 * updated incrementally, so they must be captured separately or stale cells
 * from an abandoned branch leak into the next restore's observation. */
extern size_t nle_rl_mirror_size(void);
extern void   nle_rl_mirror_save(nle_ctx_t *nle, void *dst);
extern void   nle_rl_mirror_load(nle_ctx_t *nle, const void *src);
extern void   nle_rl_winproc_reset(nle_ctx_t *nle);

/* A bundled copy of one off-current dungeon level file. NetHack writes the
 * levels you've left to disk (savelev/getlev) as "<s_lock>.<n>" in the hackdir;
 * only the level you're standing on lives in the arena. To make snapshots span
 * multiple dungeon levels we bundle those files into the handle and rewrite
 * them on restore. */
typedef struct nle_fr_levelfile {
    char    name[64]; /* basename, e.g. "1lock.2" */
    size_t  size;
    void   *data;
} nle_fr_levelfile_t;

typedef struct nle_fr_snapshot {
    nle_ctx_t           saved_ctx;
    void               *saved_stack;
    size_t              stack_size;
    void               *saved_arena;
    size_t              arena_used;
    void               *saved_mirror;
    size_t              mirror_size;
    nle_fr_levelfile_t *levelfiles;
    int                 n_levelfiles;
} nle_fr_snapshot_t;

/* Bundle every "<s_lock>.<n>" level file in the hackdir into the snapshot.
 * Skips the static <role>.lev / *.des templates and everything else. Best
 * effort: on any error a file is simply not bundled (single-level snapshots
 * have no such files, so this is a no-op for them). */
static void
nle_fr_bundle_levelfiles(nle_ctx_t *nle, nle_fr_snapshot_t *s)
{
    const char *dir = nle->settings.hackdir;
    const char *lockbase = nle->s_lock; /* note: `lock` is a NetHack macro */
    size_t locklen = strlen(lockbase);
    DIR *d;
    struct dirent *ent;
    int cap = 0;

    if (!dir[0])
        return;
    d = opendir(dir);
    if (!d)
        return;

    while ((ent = readdir(d)) != NULL) {
        char path[512];
        struct stat st;
        int fd;
        void *buf;
        ssize_t got;
        nle_fr_levelfile_t *lf;
        const char *p;

        /* A dynamic dungeon-level file is "<s_lock>.<ledger>". s_lock is
         * empty in this build, so the files are ".<digits>" (e.g. ".50");
         * the static "<role>.lev" / "*.des" templates never match because the
         * part after the dot is not all digits. This stays correct if s_lock
         * is ever set to a non-empty base. */
        if (strncmp(ent->d_name, lockbase, locklen) != 0)
            continue;
        p = ent->d_name + locklen;
        if (*p != '.')
            continue;
        p++;
        if (!*p)
            continue; /* need at least one digit */
        {
            const char *q = p;
            while (*q >= '0' && *q <= '9')
                q++;
            if (*q != '\0')
                continue; /* non-digit after the dot -> not a level file */
        }
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode) || st.st_size <= 0)
            continue;
        fd = open(path, O_RDONLY);
        if (fd < 0)
            continue;
        buf = malloc((size_t) st.st_size);
        if (!buf) {
            close(fd);
            continue;
        }
        got = read(fd, buf, (size_t) st.st_size);
        close(fd);
        if (got != (ssize_t) st.st_size) {
            free(buf);
            continue;
        }
        if (s->n_levelfiles >= cap) {
            nle_fr_levelfile_t *grown;
            cap = cap ? cap * 2 : 8;
            grown = (nle_fr_levelfile_t *) realloc(s->levelfiles,
                                                   cap * sizeof(*grown));
            if (!grown) {
                free(buf);
                break;
            }
            s->levelfiles = grown;
        }
        lf = &s->levelfiles[s->n_levelfiles++];
        strncpy(lf->name, ent->d_name, sizeof(lf->name) - 1);
        lf->name[sizeof(lf->name) - 1] = '\0';
        lf->size = (size_t) st.st_size;
        lf->data = buf;
    }
    closedir(d);
}

/* Rewrite the bundled level files back into the hackdir. Levels the snapshot
 * never visited are regenerated by NetHack (mklev), not read, so files created
 * after the snapshot need not be deleted. */
static void
nle_fr_restore_levelfiles(nle_ctx_t *nle, nle_fr_snapshot_t *s)
{
    const char *dir = nle->settings.hackdir;
    int i;

    if (!dir[0])
        return;
    for (i = 0; i < s->n_levelfiles; i++) {
        nle_fr_levelfile_t *lf = &s->levelfiles[i];
        char path[512];
        int fd;

        snprintf(path, sizeof(path), "%s/%s", dir, lf->name);
        fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd < 0)
            continue;
        if (write(fd, lf->data, lf->size) != (ssize_t) lf->size) {
            /* short write: nothing actionable here, leave best-effort */
        }
        close(fd);
    }
}

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

    /* Bundle off-current dungeon levels (on disk) so the snapshot is complete
     * across the whole dungeon, not just the level in the arena. */
    nle_fr_bundle_levelfiles(nle, s);
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

    /* Reset the win-proc diagnostic deque to empty: it lives outside the arena/
     * snapshot and would otherwise keep the pre-restore depth, which the
     * restored coroutine stack's pending ScopedStack dtors no longer match
     * (pop-on-empty corrupts the deque -> SIGSEGV on the next push). */
    nle_rl_winproc_reset(nle);

    /* Restore the rl-port display mirror (kept outside the arena). *nle above
     * already restored the s_netHackRL_instance pointer; refill its glyph/char
     * /color map arrays so the next observation has no abandoned-branch residue. */
    if (s->saved_mirror)
        nle_rl_mirror_load(nle, s->saved_mirror);

    /* Rewrite the off-current dungeon level files so travelling back to a level
     * left before the snapshot reads the snapshot-time level, not a later one. */
    nle_fr_restore_levelfiles(nle, s);
}

void
nle_fr_destroy(void *snap)
{
    if (!snap)
        return;
    nle_fr_snapshot_t *s = (nle_fr_snapshot_t *) snap;
    int i;
    free(s->saved_stack);
    free(s->saved_arena);
    free(s->saved_mirror);
    for (i = 0; i < s->n_levelfiles; i++)
        free(s->levelfiles[i].data);
    free(s->levelfiles);
    free(s);
}
