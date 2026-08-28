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

/* The coroutine (fcontext) stack is copied wholesale by snapshot/restore. ASAN
 * doesn't know about the fiber stack and reports false stack-buffer-underflows
 * on those memcpys, so unpoison the region first under an ASAN build. No-op
 * otherwise. */
#if defined(__SANITIZE_ADDRESS__)
#include <sanitizer/asan_interface.h>
#define NLE_ASAN_UNPOISON(p, n) __asan_unpoison_memory_region((p), (n))
/* The snapshot scratch buffers (saved_stack/arena/mirror/levelfiles) are plain
 * malloc()'d, but NetHack's `free` macro routes to nle_arena_free->__libc_free,
 * which bypasses ASAN's malloc interceptor and aborts ("invalid pointer"). Use
 * the real (ASAN-intercepted) free here so the allocator pairs match. */
#undef free
#else
#define NLE_ASAN_UNPOISON(p, n) ((void) 0)
#endif

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

/* Defined below; nle_fr_snapshot needs it to unwind a failed bundle. */
void nle_fr_destroy(void *snap);

/* Return code of the level-file rewrite done by the most recent
 * nle_fr_restore (0 == the disk set now matches the snapshot exactly).
 * nle_fr_restore's void ABI is published, so this is reported out-of-band
 * rather than by changing its signature. Read via nle_fr_last_restore_rc(). */
static int nle_fr_last_rc = 0;

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

/* ------------------------------------------------------------------------
 * Level-file set: derivation, bundling, restoration.
 *
 * WHY THIS IS TOTAL AND NOT BEST-EFFORT (E16 incident, 2026-08):
 * `goto_level` (do.c:1451) branches on the IN-MEMORY flag
 * `level_info[ledger].linfo_flags & LFILE_EXISTS`. If that flag is set it
 * open_levelfile()s the on-disk "<base>.<ledger>" and, when the open fails,
 * calls tricked_fileremoved() (save.c:487) -> done(TRICKED): an INSTANT
 * game-over at full HP, indistinguishable downstream from a monster kill.
 * So any (heap says exists) / (file absent) divergence is not a cosmetic
 * inconsistency, it is a silent data-corrupting fake death. The contract
 * here is therefore an equality, enforced in both directions:
 *
 *   after nle_fr_restore, the set of "<base>.<digits>" files in the hackdir
 *   is EXACTLY the set that existed when nle_fr_snapshot ran.
 *
 * Bundling all of them fixes the (flag set, file missing) direction -- e.g.
 * done() -> clearlocks() (end.c:1371 -> files.c:614) unlinks EVERY level file
 * on death, and the forced-revive restore has to put them all back. Deleting
 * the extras fixes the (flag clear, stale file present) direction, so the
 * restored disk state is a pure function of the snapshot rather than a
 * superset that depends on the abandoned future.
 * ---------------------------------------------------------------------- */

/* Derive the lock BASE from nle->s_lock.
 *
 * `lock` (== current_nle_ctx->s_lock, decl.h:292) is NOT a stable base name:
 * set_levelfile_name() (files.c:499) rewrites it IN PLACE to "<base>.<lev>"
 * on every create/open/delete of a level file. Reading s_lock directly as the
 * prefix -- which this file did until 2026-08 -- therefore matched nothing at
 * all the moment any level transition had happened ("2" never prefixes ".1"),
 * so the bundle silently captured ZERO files and restore rewrote nothing.
 * Strip a trailing ".<digits>" to recover the base, exactly inverting what
 * set_levelfile_name() appends. s_lock is empty in this build, so the base is
 * "" and level files are ".<digits>"; this stays correct for a non-empty base.
 */
static void
nle_fr_lock_base(const nle_ctx_t *nle, char *out, size_t outsz)
{
    const char *s = nle->s_lock;
    size_t n = strlen(s);
    const char *dot = NULL;
    size_t i;

    for (i = n; i > 0; i--) {
        if (s[i - 1] == '.') {
            dot = s + (i - 1);
            break;
        }
    }
    if (dot && dot[1]) {
        const char *q = dot + 1;

        while (*q >= '0' && *q <= '9')
            q++;
        if (*q == '\0')
            n = (size_t) (dot - s); /* trailing ".<digits>" -> strip it */
    }
    if (n >= outsz)
        n = outsz - 1;
    memcpy(out, s, n);
    out[n] = '\0';
}

/* TRUE if `name` is a dynamic dungeon-level file "<base>.<digits>". The static
 * <role>.lev / *.des templates never match: the part after the dot is not all
 * digits. */
static int
nle_fr_is_levelfile(const char *name, const char *base, size_t baselen)
{
    const char *p;

    if (strncmp(name, base, baselen) != 0)
        return 0;
    p = name + baselen;
    if (*p != '.')
        return 0;
    p++;
    if (!*p)
        return 0; /* need at least one digit */
    while (*p >= '0' && *p <= '9')
        p++;
    return *p == '\0';
}

/* Bundle EVERY "<base>.<n>" level file in the hackdir into the snapshot.
 *
 * Returns 0 on success, -1 on failure. Failure is HARD: nle_fr_snapshot
 * discards the whole handle and returns NULL rather than hand back a
 * partial bundle, because a partial bundle is precisely the (flag set,
 * file missing) divergence that fakes a death on the next stair use. A
 * caller that gets NULL simply has no checkpoint -- loud and safe.
 *
 * Zero-length level files are bundled as genuine zero-length entries (not
 * skipped): NetHack's create_levelfile() sets LFILE_EXISTS the instant the
 * file is created, before savelev() writes a byte, so an empty file is a
 * state the heap can legitimately believe in and restore must reproduce. */
static int
nle_fr_bundle_levelfiles(nle_ctx_t *nle, nle_fr_snapshot_t *s)
{
    const char *dir = nle->settings.hackdir;
    char base[sizeof nle->s_lock];
    size_t baselen;
    DIR *d;
    struct dirent *ent;
    int cap = 0;

    if (!dir[0])
        return 0; /* no writable hackdir: there are no level files */
    nle_fr_lock_base(nle, base, sizeof base);
    baselen = strlen(base);
    d = opendir(dir);
    if (!d)
        return -1;

    while ((ent = readdir(d)) != NULL) {
        char path[512];
        struct stat st;
        int fd;
        void *buf;
        ssize_t got;
        nle_fr_levelfile_t *lf;

        if (!nle_fr_is_levelfile(ent->d_name, base, baselen))
            continue;
        if (strlen(ent->d_name) >= sizeof lf->name)
            goto fail; /* name we could not faithfully restore */
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode) || st.st_size < 0)
            goto fail;
        buf = NULL;
        if (st.st_size > 0) {
            fd = open(path, O_RDONLY);
            if (fd < 0)
                goto fail;
            buf = malloc((size_t) st.st_size);
            if (!buf) {
                close(fd);
                goto fail;
            }
            got = read(fd, buf, (size_t) st.st_size);
            close(fd);
            if (got != (ssize_t) st.st_size) {
                free(buf);
                goto fail;
            }
        }
        if (s->n_levelfiles >= cap) {
            nle_fr_levelfile_t *grown;
            cap = cap ? cap * 2 : 8;
            grown = (nle_fr_levelfile_t *) realloc(s->levelfiles,
                                                   cap * sizeof(*grown));
            if (!grown) {
                free(buf);
                goto fail;
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
    return 0;

fail:
    closedir(d);
    return -1;
}

/* Rewrite the bundled level files into the hackdir AND delete every other
 * "<base>.<digits>" file there, so the on-disk level-file set after restore is
 * exactly the snapshot-time set (see the block comment above). Returns 0 on
 * success, -1 if any write or unlink failed.
 *
 * `stamp_pid` restamps each file's leading hackpid header field with this
 * env's hackpid. savelev() writes `hackpid` as the very first bytes of a level
 * file (save.c:650) and getlev() rejects a mismatch with trickery() ->
 * done(TRICKED) (restore.c:1171) -- ANOTHER full-HP fake death, this one
 * specific to resuming a checkpoint written by a different process. It is off
 * for same-process fast-reset (where the pid already matches, so restamping
 * would be a no-op) and on for nle_fr_levelfiles_load. The dlvl header field
 * that follows is left alone, so a genuinely wrong level is still caught. */
static int
nle_fr_restore_levelfiles(nle_ctx_t *nle, nle_fr_snapshot_t *s, int stamp_pid)
{
    const char *dir = nle->settings.hackdir;
    char base[sizeof nle->s_lock];
    size_t baselen;
    DIR *d;
    struct dirent *ent;
    int i, rc = 0;

    if (!dir[0])
        return 0;
    for (i = 0; i < s->n_levelfiles; i++) {
        nle_fr_levelfile_t *lf = &s->levelfiles[i];
        char path[512];
        int fd;

        snprintf(path, sizeof(path), "%s/%s", dir, lf->name);
        fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd < 0) {
            rc = -1;
            continue;
        }
        if (lf->size
            && write(fd, lf->data, lf->size) != (ssize_t) lf->size)
            rc = -1;
        if (!rc && stamp_pid && lf->size >= sizeof nle->hackpid) {
            if (lseek(fd, 0, SEEK_SET) != 0
                || write(fd, (const void *) &nle->hackpid,
                         sizeof nle->hackpid)
                       != (ssize_t) sizeof nle->hackpid)
                rc = -1;
        }
        close(fd);
    }

    /* Drop stale post-snapshot level files: they belong to a future this
     * restore just abandoned, and leaving them makes the (heap, disk) pair
     * depend on that future. */
    nle_fr_lock_base(nle, base, sizeof base);
    baselen = strlen(base);
    d = opendir(dir);
    if (!d)
        return -1;
    while ((ent = readdir(d)) != NULL) {
        char path[512];
        int keep = 0;

        if (!nle_fr_is_levelfile(ent->d_name, base, baselen))
            continue;
        for (i = 0; i < s->n_levelfiles; i++)
            if (!strcmp(s->levelfiles[i].name, ent->d_name)) {
                keep = 1;
                break;
            }
        if (keep)
            continue;
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        if (unlink(path) != 0)
            rc = -1;
    }
    closedir(d);
    return rc;
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
    NLE_ASAN_UNPOISON(stack_lo, s->stack_size);
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
     * across the whole dungeon, not just the level in the arena. A partial
     * bundle is worse than no snapshot (it restores a heap that believes in
     * files that are not there -> done(TRICKED) on the next stair), so this
     * is fatal to the whole handle. */
    if (nle_fr_bundle_levelfiles(nle, s) != 0) {
        nle_fr_destroy(s);
        return NULL;
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
    NLE_ASAN_UNPOISON(stack_lo, s->stack_size);
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

    /* Rewrite the off-current dungeon level files (and drop the stale ones) so
     * the disk level-file set is exactly the snapshot-time set: travelling back
     * to a level left before the snapshot reads the snapshot-time level, and a
     * level the restored heap believes in is never missing (done(TRICKED)).
     * Recorded rather than returned so the void ABI is unchanged; read it with
     * nle_fr_last_restore_rc(). */
    nle_fr_last_rc = nle_fr_restore_levelfiles(nle, s, 0);
}

/* --- integrity introspection (used by the harness checkpoint guard) ------- */

/* Result of the level-file rewrite performed by the most recent
 * nle_fr_restore: 0 = every file written and every stale file dropped. */
int
nle_fr_last_restore_rc(void)
{
    return nle_fr_last_rc;
}

/* How many level files this snapshot handle bundled. Zero after any level
 * transition is the signature of the pre-2026-08 lock-base bug. */
int
nle_fr_levelfile_count(void *snap)
{
    return snap ? ((nle_fr_snapshot_t *) snap)->n_levelfiles : -1;
}

/* THE integrity invariant, checked against live state: how many ledgers does
 * the in-memory dungeon graph mark LFILE_EXISTS while the corresponding
 * "<base>.<ledger>" file is absent from the hackdir? Each one is a latent
 * done(TRICKED) waiting for the hero to walk onto that level's stairs
 * (do.c:1451 -> save.c:487). Must be 0. Returns -1 if it cannot be computed. */
int
nle_fr_missing_levelfiles(nle_ctx_t *nle)
{
    char base[sizeof nle->s_lock];
    const char *dir;
    int lev, maxlev, missing = 0;

    if (!nle)
        return -1;
    current_nle_ctx = nle;
    dir = nle->settings.hackdir;
    if (!dir[0])
        return -1;
    nle_fr_lock_base(nle, base, sizeof base);
    /* n_dgns is file-static-by-macro in dungeon.c (s_n_dgns on the ctx);
     * maxledgerno() reads the dungeon graph, so guard it the same way
     * clearlocks() does (files.c: "can't access maxledgerno() before
     * dungeons are created"). */
    maxlev = nle->s_n_dgns ? (int) maxledgerno() : 0;
    for (lev = 1; lev <= maxlev; lev++) {
        char path[512];
        struct stat st;

        if (!(level_info[lev].linfo_flags & LFILE_EXISTS))
            continue;
        snprintf(path, sizeof(path), "%s/%s.%d", dir, base, lev);
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode))
            missing++;
    }
    return missing;
}

/* --- portable level-file set (cross-PROCESS checkpoints) ------------------
 *
 * nle_fr_snapshot's handle is a live-process construct (a raw coroutine-stack
 * image, arena bytes addressed off nle->s_arena_base, and the rl display
 * mirror), so it cannot be written to disk. A disk checkpoint is instead
 * (player blob + level blob + THIS blob): the same "<base>.<digits>" file set
 * the fast-reset bundler captures, in a self-describing container.
 *
 * It is mandatory, not optional: nle_save_player -> savegamestate ->
 * save_dungeon (dungeon.c:175) writes the whole level_info[] array, so a
 * resumed game's heap carries the LFILE_EXISTS flags of every level the
 * ORIGINAL game had visited. Resuming into a fresh, empty hackdir without
 * these files reproduces the exact done(TRICKED) fake-death on the first
 * stair transition.
 *
 * Container format (little-endian, fixed widths):
 *   "NLELVLF1"                 8-byte magic
 *   uint32 count
 *   per entry: uint32 namelen, name bytes (no NUL), uint64 size, size bytes
 * ---------------------------------------------------------------------- */

#define NLE_LF_MAGIC "NLELVLF1"

static void
nle_fr_put32(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char) (v & 0xff);
    p[1] = (unsigned char) ((v >> 8) & 0xff);
    p[2] = (unsigned char) ((v >> 16) & 0xff);
    p[3] = (unsigned char) ((v >> 24) & 0xff);
}

static unsigned int
nle_fr_get32(const unsigned char *p)
{
    return (unsigned int) p[0] | ((unsigned int) p[1] << 8)
           | ((unsigned int) p[2] << 16) | ((unsigned int) p[3] << 24);
}

static void
nle_fr_put64(unsigned char *p, unsigned long long v)
{
    int i;

    for (i = 0; i < 8; i++)
        p[i] = (unsigned char) ((v >> (8 * i)) & 0xff);
}

static unsigned long long
nle_fr_get64(const unsigned char *p)
{
    unsigned long long v = 0;
    int i;

    for (i = 7; i >= 0; i--)
        v = (v << 8) | (unsigned long long) p[i];
    return v;
}

/* Serialize the hackdir's whole level-file set into one malloc'd blob. Free it
 * with nle_free_blob. Returns NULL (and *out_len == 0) on error. An empty set
 * still returns a valid header-only blob, so "no levels yet" is representable
 * and distinguishable from failure. */
void *
nle_fr_levelfiles_blob(nle_ctx_t *nle, long *out_len)
{
    nle_fr_snapshot_t tmp;
    unsigned char *blob, *p;
    size_t total = 8 + 4;
    int i;

    if (out_len)
        *out_len = 0;
    if (!nle)
        return NULL;
    current_nle_ctx = nle;
    memset(&tmp, 0, sizeof tmp);
    if (nle_fr_bundle_levelfiles(nle, &tmp) != 0) {
        for (i = 0; i < tmp.n_levelfiles; i++)
            free(tmp.levelfiles[i].data);
        free(tmp.levelfiles);
        return NULL;
    }
    for (i = 0; i < tmp.n_levelfiles; i++)
        total += 4 + strlen(tmp.levelfiles[i].name) + 8 + tmp.levelfiles[i].size;

    blob = (unsigned char *) malloc(total);
    if (blob) {
        p = blob;
        memcpy(p, NLE_LF_MAGIC, 8);
        p += 8;
        nle_fr_put32(p, (unsigned int) tmp.n_levelfiles);
        p += 4;
        for (i = 0; i < tmp.n_levelfiles; i++) {
            nle_fr_levelfile_t *lf = &tmp.levelfiles[i];
            size_t nl = strlen(lf->name);

            nle_fr_put32(p, (unsigned int) nl);
            p += 4;
            memcpy(p, lf->name, nl);
            p += nl;
            nle_fr_put64(p, (unsigned long long) lf->size);
            p += 8;
            if (lf->size) {
                memcpy(p, lf->data, lf->size);
                p += lf->size;
            }
        }
        if (out_len)
            *out_len = (long) total;
    }
    for (i = 0; i < tmp.n_levelfiles; i++)
        free(tmp.levelfiles[i].data);
    free(tmp.levelfiles);
    return blob;
}

/* Install a blob from nle_fr_levelfiles_blob into the hackdir, deleting any
 * "<base>.<digits>" file it does not contain (same totality contract as
 * nle_fr_restore). Returns 0 on success, nonzero on a malformed blob or an
 * I/O failure. */
int
nle_fr_levelfiles_load(nle_ctx_t *nle, const void *blob, long len)
{
    const unsigned char *p = (const unsigned char *) blob;
    const unsigned char *end;
    nle_fr_snapshot_t tmp;
    unsigned int count, i;
    int rc = 0;

    if (!nle || !blob || len < 12)
        return 1;
    current_nle_ctx = nle;
    end = p + len;
    if (memcmp(p, NLE_LF_MAGIC, 8) != 0)
        return 2;
    p += 8;
    count = nle_fr_get32(p);
    p += 4;

    memset(&tmp, 0, sizeof tmp);
    if (count) {
        tmp.levelfiles =
            (nle_fr_levelfile_t *) calloc(count, sizeof(*tmp.levelfiles));
        if (!tmp.levelfiles)
            return 3;
    }
    for (i = 0; i < count; i++) {
        unsigned int nl;
        unsigned long long sz;
        nle_fr_levelfile_t *lf = &tmp.levelfiles[i];

        if ((size_t) (end - p) < 4) {
            rc = 4;
            goto done;
        }
        nl = nle_fr_get32(p);
        p += 4;
        if (nl >= sizeof lf->name || (size_t) (end - p) < (size_t) nl + 8) {
            rc = 4;
            goto done;
        }
        memcpy(lf->name, p, nl);
        lf->name[nl] = '\0';
        p += nl;
        sz = nle_fr_get64(p);
        p += 8;
        if ((unsigned long long) (end - p) < sz) {
            rc = 4;
            goto done;
        }
        lf->size = (size_t) sz;
        lf->data = (void *) p; /* borrowed: not freed below */
        p += (size_t) sz;
        tmp.n_levelfiles++;
    }
    rc = nle_fr_restore_levelfiles(nle, &tmp, 1) == 0 ? 0 : 5;

done:
    free(tmp.levelfiles);
    return rc;
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
