/* nle.c -- the NetHack Learning Environment layer: one context per
 * environment, a coroutine that runs the game, observation plumbing, and the
 * engine-control API (seeds, knobs, level/player blobs, state pokes,
 * curriculum traversal).
 *
 * All of the game's own mutable state lives in the generated
 * struct nh_globals (nh_globals.h) that every context owns; this file only
 * anchors the thread-local pointers to the environment being stepped. */

#include <assert.h>
#include <string.h>
#include <fcntl.h>     /* O_WRONLY/O_CREAT/O_TRUNC for nle_load_level */
#include <sys/time.h>
#include <sys/mman.h>  /* munmap per-env arena in nle_end */

#include <tmt.h>

#define NEED_VARARGS
#ifdef MONITOR_HEAP
#undef MONITOR_HEAP
#endif
#include "hack.h"
#include "lev.h"        /* WRITE_SAVE / FREE_SAVE for savelev() */

#include "dlb.h"

#include "nle.h"
#include "nle_sentinel.h"

/* The one thread-local pointer to the environment being stepped (declared
 * extern in nle.h). initial-exec TLS is a single fs-relative load; the
 * loader reserves static TLS for a dlopen'd library that uses it. */
__attribute__((tls_model("initial-exec")))
__thread nle_ctx_t *current_nle_ctx;

#ifdef NLE_BZ2_TTYRECS
#include <bzlib.h>
#endif

#define STACK_SIZE (1 << 16) /* 64KiB */

#ifndef __has_feature
#define __has_feature(x) 0 /* Compatibility with non-clang compilers. */
#endif

#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#include <sanitizer/asan_interface.h>
#endif

extern int unixmain(int, char **);

/* Make `nle` the current environment on this thread: both the NLE context
 * pointer and the game's globals pointer. Every public entry point does
 * this first, so any pool thread can step any env. */
void
nle_anchor(nle_ctx_t *nle)
{
    current_nle_ctx = nle;
    nh_g = nle ? nle->g : (struct nh_globals *) 0;
}

/* A context and its game globals are one contiguous, 64-byte aligned block:
 * [nle_ctx_t | struct nh_globals]. */
#define NLE_CTX_ALIGN 64
static size_t
nle_ctx_hdr_size(void)
{
    return (sizeof(nle_ctx_t) + NLE_CTX_ALIGN - 1) & ~(size_t) (NLE_CTX_ALIGN - 1);
}

size_t
nle_ctx_block_size(void)
{
    return nle_ctx_hdr_size() + nh_globals_size();
}

signed char
vt_char_color_extract(TMTCHAR *c)
{
    /* We pick out the colors in the enum tmt_color_t. These match the order
     * found standard in IBM color graphics, and are the same order as those
     * found in src/color.h.  */

    /* TODO: We no longer need *signed* chars. Let's change the dtype of
     * tty_chars when we change the API next. */

    signed char color;

    if (c->a.fg == TMT_COLOR_DEFAULT) {
        /* Need to make a choice for default color. To stay compatible with
           NetHack, choose black for the "null glyph", gray otherwise. */
        color = (c->c == ' ') ? CLR_BLACK : CLR_GRAY; /* 0 or 7 */
    } else if (c->a.fg < TMT_COLOR_MAX) {
        color = c->a.fg - TMT_COLOR_BLACK + CLR_BLACK; /* TMT color offset. */
        if (c->a.bold) {
            color |= BRIGHT;
        }
    } else {
        fprintf(stderr, "Illegal color %d\n", (int) c->a.fg);
        color = CLR_GRAY;
    }

    /* The above is 0..15. For "reverse" colors (bg/fg swap), let's
     * use 16..31. */
    if (c->a.reverse) {
        color += CLR_MAX;
    }
    return color;
}

void
nle_vt_callback(tmt_msg_t m, TMT *vt, const void *a, void *p)
{
    const TMTSCREEN *s = tmt_screen(vt);
    const TMTPOINT *cur = tmt_cursor(vt);

    nle_ctx_t *nle = (nle_ctx_t *) p;
    if (!nle || !nle->observation) {
        return;
    }

    switch (m) {
    case TMT_MSG_BELL:
        break;

    case TMT_MSG_UPDATE:
        for (size_t r = 0; r < s->nline; r++) {
            if (s->lines[r]->dirty) {
                for (size_t c = 0; c < s->ncol; c++) {
                    size_t offset = (r * NLE_TERM_CO) + c;
                    TMTCHAR *tmt_c = &(s->lines[r]->chars[c]);

                    if (nle->observation->tty_chars) {
                        nle->observation->tty_chars[offset] = tmt_c->c;
                    }

                    if (nle->observation->tty_colors) {
                        nle->observation->tty_colors[offset] =
                            vt_char_color_extract(tmt_c);
                    }
                }
            }
        }
        tmt_clean(vt);
        break;

    case TMT_MSG_ANSWER:
        break;

    case TMT_MSG_MOVED:
        if (nle->observation->tty_cursor) {
            /* cast from size_t is safe from overflow, since r,c < 256 */
            nle->observation->tty_cursor[0] = (unsigned char) cur->r;
            nle->observation->tty_cursor[1] = (unsigned char) cur->c;
        }
        break;

    case TMT_MSG_CURSOR:
        break;
    }
}

/* ---- virtual terminal snapshot -------------------------------------------
 * The TMT terminal emulator that produces the tty_* observations lives in
 * libc memory, outside the context block and the arena, and only repaints
 * cells the game redraws. A snapshot therefore carries the whole screen and
 * the cursor, so a restore cannot leave menu/message residue from the
 * abandoned continuation in tty_chars. */
size_t
nle_vt_snapshot_size(nle_ctx_t *nle)
{
    const TMTSCREEN *s = tmt_screen(nle->vterminal);
    return s->nline * s->ncol * sizeof(TMTCHAR) + 2 * sizeof(int);
}

void
nle_vt_snapshot_save(nle_ctx_t *nle, void *dst)
{
    const TMTSCREEN *s = tmt_screen(nle->vterminal);
    const TMTPOINT *cur = tmt_cursor(nle->vterminal);
    char *p = (char *) dst;
    int c[2];
    size_t r;

    for (r = 0; r < s->nline; r++) {
        memcpy(p, s->lines[r]->chars, s->ncol * sizeof(TMTCHAR));
        p += s->ncol * sizeof(TMTCHAR);
    }
    c[0] = (int) cur->r;
    c[1] = (int) cur->c;
    memcpy(p, c, sizeof c);
}

void
nle_vt_snapshot_load(nle_ctx_t *nle, const void *src)
{
    TMT *vt = nle->vterminal;
    const TMTSCREEN *s = tmt_screen(vt);
    const char *p = (const char *) src;
    char seq[32];
    int c[2];
    size_t r;

    for (r = 0; r < s->nline; r++) {
        /* the screen is ours to write: tmt only hands out a const view */
        memcpy((void *) s->lines[r]->chars, p, s->ncol * sizeof(TMTCHAR));
        s->lines[r]->dirty = true;
        p += s->ncol * sizeof(TMTCHAR);
    }
    memcpy(c, p, sizeof c);
    /* cursor + reset of the pending text attributes, through the emulator so
     * its own state agrees (TMT_MSG_MOVED refreshes tty_cursor) */
    snprintf(seq, sizeof seq, "\033[0m\033[%d;%dH", c[0] + 1, c[1] + 1);
    tmt_write(vt, seq, strlen(seq));
    /* every line is dirty: the next flush repaints the whole tty observation */
    for (r = 0; r < s->nline; r++)
        s->lines[r]->dirty = true;
}

/* ---- difficulty knob catalog (catalog defined in include/nle.h) ---------- */

static const char *const nle_tune_names_tbl[] = {
#define NLE_TUNE_NAME(name, dflt) #name,
    NLE_TUNE_FIELDS(NLE_TUNE_NAME)
#undef NLE_TUNE_NAME
};

int
nle_tune_count(void)
{
    return (int) (sizeof(nle_tune_names_tbl) / sizeof(nle_tune_names_tbl[0]));
}

const char *
nle_tune_name(int index)
{
    if (index < 0 || index >= nle_tune_count())
        return (const char *) 0;
    return nle_tune_names_tbl[index];
}

void
nle_tune_set_defaults(nle_tune_t *t)
{
#define NLE_TUNE_DFLT(name, dflt) t->name = (dflt);
    NLE_TUNE_FIELDS(NLE_TUNE_DFLT)
#undef NLE_TUNE_DFLT
}

nle_tune_t *
nle_get_tune(nle_ctx_t *nle)
{
    return &nle->knobs;
}

nle_ctx_t *
init_nle(FILE *ttyrec, nle_obs *obs)
{
    /* One block: the context, then the game's globals. */
    size_t hdr = nle_ctx_hdr_size();
    char *block = (char *) calloc(1, nle_ctx_block_size());
    nle_ctx_t *nle = (nle_ctx_t *) block;
    if (!nle) {
        fprintf(stderr, "init_nle: cannot allocate the environment\n");
        abort();
    }
    nle->g = (struct nh_globals *) (block + hdr);

    /* Anchor this env BEFORE anything expands an accessor macro. */
    nle_anchor(nle);
    /* Fresh game state: zero + the original static initializers. */
    nh_globals_init(nle->g);

    /* Difficulty knobs default to vanilla (all scales 1.0). */
    nle_tune_set_defaults(&nle->knobs);

    /* tmt_open below reads the terminal size through the game's LI/CO. */
    LI = NLE_TERM_LI;
    CO = NLE_TERM_CO;

    nle->ttyrec = ttyrec;

#ifdef NLE_BZ2_TTYRECS
    if (nle->ttyrec) {
        int bzerror;
        nle->ttyrec_bz2 = BZ2_bzWriteOpen(&bzerror, ttyrec, 9, 0, 0);
        assert(bzerror == BZ_OK);
    }
#endif

    nle->observation = obs;

    TMT *vterminal = tmt_open(LI, CO, nle_vt_callback, nle, NULL, true);
    assert(vterminal);
    nle->vterminal = vterminal;

    nle->outbuf_write_ptr = nle->outbuf;
    nle->outbuf_write_end = nle->outbuf + sizeof(nle->outbuf);

    return nle;
}

/* TODO: Consider copying the relevant parts of main() in unixmain.c. */
void
mainloop(fcontext_transfer_t ctx_transfer)
{
    current_nle_ctx->returncontext = ctx_transfer.ctx;
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
    /* ASan isn't happy with fcontext's assembly.
     * See: https://bugs.llvm.org/show_bug.cgi?id=27627 and
     * https://github.com/boostorg/coroutine/issues/30#issuecomment-325578344
     * TODO: I don't understand why __sanitizer_(start/finish)_switch_fiber
     * doesn't work here.
     */
    fcontext_stack_t *stack = &current_nle_ctx->stack;
    ASAN_UNPOISON_MEMORY_REGION((char *) stack->sptr - stack->ssize,
                                stack->ssize);
#endif

    nle_settings *s = &current_nle_ctx->settings;
    int len = strnlen(s->hackdir, sizeof(s->hackdir));

    if (len >= sizeof(s->hackdir) - 1) {
        error("HACKDIR too long");
        return;
    }
    if (s->hackdir[len - 1] != '/') {
        s->hackdir[len] = '/';
        s->hackdir[len + 1] = '\0';
    } else {
        s->hackdir[len] = '\0';
    }

    /* Read-only data directory. When supplied, the immutable game data lives
     * here (shared across envs, read-only) and only the writable game-state
     * files stay under hackdir -- so a fresh env needs only a tiny writable dir
     * rather than a full copy of the dat tree. Empty => fall back to hackdir,
     * which reproduces the original single-directory behavior exactly. */
    char *datadir = s->hackdir;
    if (s->datadir[0] != '\0') {
        int dlen = strnlen(s->datadir, sizeof(s->datadir));
        if (dlen >= (int) sizeof(s->datadir) - 1) {
            error("DATADIR too long");
            return;
        }
        if (s->datadir[dlen - 1] != '/') {
            s->datadir[dlen] = '/';
            s->datadir[dlen + 1] = '\0';
        }
        datadir = s->datadir;
    }

    char *scoreprefix = (s->scoreprefix[0] != '\0')
                            ? s->scoreprefix
                            : s->hackdir;
    /* Read-only prefixes -> shared datadir (never written by the engine). */
    fqn_prefix[DATAPREFIX] = datadir;
    fqn_prefix[HACKPREFIX] = datadir;
    fqn_prefix[SYSCONFPREFIX] = datadir;
    fqn_prefix[CONFIGPREFIX] = datadir;
    /* Writable prefixes -> per-env hackdir. */
    fqn_prefix[SAVEPREFIX] = s->hackdir;
    fqn_prefix[LEVELPREFIX] = s->hackdir;
    fqn_prefix[BONESPREFIX] = s->hackdir;
    fqn_prefix[SCOREPREFIX] = scoreprefix;
    fqn_prefix[LOCKPREFIX] = s->hackdir;
    fqn_prefix[TROUBLEPREFIX] = s->hackdir;

    char *argv[1] = { "nethack" };

    unixmain(1, argv);
}

boolean
write_ttyrec_data(void *buf, int length)
{
    nle_ctx_t *nle = current_nle_ctx;
#ifdef NLE_BZ2_TTYRECS
    int bzerror;
    BZ2_bzWrite(&bzerror, nle->ttyrec_bz2, buf, length);
    assert(bzerror == BZ_OK);
#else
    assert(fwrite(buf, 1, length, nle->ttyrec) == length);
#endif
    return TRUE;
}

boolean
write_ttyrec_header(int length, unsigned char channel)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    int buffer[3];
    buffer[0] = tv.tv_sec;
    buffer[1] = tv.tv_usec;
    buffer[2] = length;

    /* Assumes little endianness */
    write_ttyrec_data(buffer, 3 * sizeof(int));
    write_ttyrec_data(&channel, 1);

    return TRUE;
}

/* win/tty only calls fflush(stdout). */
int
nle_fflush(FILE *stream)
{
    /* Only act on fflush(stdout). For any other stream, pass straight
     * through to libc's fflush (wintty.h may have `#define fflush nle_fflush`
     * in scope in this TU, hence the #undef). */
    if (stream != stdout) {
#undef fflush
        return fflush(stream);
    }
    nle_ctx_t *nle = current_nle_ctx;

    ssize_t length = nle->outbuf_write_ptr - nle->outbuf;
    if (length == 0)
        return 0;

    if (nle->ttyrec) {
        write_ttyrec_header(length, 0);
        write_ttyrec_data(nle->outbuf, length);
    }

    nle_obs *obs = nle->observation;
    if (obs->tty_chars || obs->tty_colors || obs->tty_cursor) {
        tmt_write(nle->vterminal, nle->outbuf, length);
    }
    nle->outbuf_write_ptr = nle->outbuf;

#ifdef NLE_BZ2_TTYRECS
    return 0;
#else
    return nle->ttyrec ? fflush(nle->ttyrec) : 0;
#endif
}

/*
 * NetHack prints most of its output via putchar. We do our
 * own buffering.
 */
int
nle_putchar(int c)
{
    nle_ctx_t *nle = current_nle_ctx;
    /* When neither a tty observation nor a ttyrec consumes the terminal
     * stream, the bytes would be dropped by nle_fflush anyway: skip the
     * whole write path (a measurable share of the per-step cost). */
    nle_obs *obs = nle->observation;
    if (!nle->ttyrec
        && (!obs || (!obs->tty_chars && !obs->tty_colors && !obs->tty_cursor)))
        return c;
    if (nle->outbuf_write_ptr >= nle->outbuf_write_end) {
        nle_fflush(stdout);
    }
    *nle->outbuf_write_ptr++ = c;
    return c;
}

/*
 * Used in place of xputs from termcap.c. Not using
 * the tputs padding logic from tclib.c.
 */
int
nle_xputs(const char *str)
{
    int c;
    const char *p = str;

    if (!p || !*p)
        return 0;

    while ((c = *p++) != '\0') {
        nle_putchar(c);
    }
    return 0;
}

/*
 * puts seems to be called only by tty_raw_print and tty_raw_print_bold.
 * We could probably override this in winrl instead.
 */
int
nle_puts(const char *str)
{
    if (!*str) /* At exit, an empty string gets printed in tty_raw_print. */
        return 0;

    int val = fputs(str, stdout);
    putc('\n', stdout); /* puts includes a newline, fputs doesn't */
    return val;
}

/* Necessary for initial observation struct. */
nle_obs *
nle_get_obs()
{
    return current_nle_ctx->observation;
}

void *
nle_yield(void *notdone)
{
    nle_fflush(stdout);
    fcontext_transfer_t t =
        jump_fcontext(current_nle_ctx->returncontext, notdone);
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
    fcontext_stack_t *stack = &current_nle_ctx->stack;
    ASAN_UNPOISON_MEMORY_REGION((char *) stack->sptr - stack->ssize,
                                stack->ssize);
#endif

    if (notdone)
        current_nle_ctx->returncontext = t.ctx;

    return t.data;
}

void
nethack_exit(int status)
{
    if (status) {
        fprintf(stderr, "NetHack exit with status %i\n", status);
    }
    nle_yield(NULL);
}

/* Called in really_done() in end.c to get "how". */
void
nle_done(int how)
{
    nle_ctx_t *nle = current_nle_ctx;
    nle->observation->how_done = how;
}

char *
nle_ttyrecname()
{
    return current_nle_ctx->settings.ttyrecname;
}

int
nle_spawn_monsters()
{
    return current_nle_ctx->settings.spawn_monsters;
}

/* See rng.c. */
extern int FDECL(whichrng, (int FDECL((*fn), (int) )));

/* See hacklib.c. */
extern void FDECL(set_random, (unsigned long, int FDECL((*fn), (int) )));
/* An appropriate version of this must always be provided in
   port-specific code somewhere. It returns a number suitable
   as seed for the random number generator */
extern unsigned long NDECL(sys_random_seed);

char *
nle_getenv(const char *name)
{
    if (strcmp(name, "TERM") == 0) {
        return "ansi";
    }
    if (strcmp(name, "NETHACKOPTIONS") == 0) {
        return current_nle_ctx->settings.options;
    }
    /* Don't return anything for "SHOPTYPE" or "SPLEVTYPE". */
    return (char *) 0;
}

FILE *
nle_fopen_wizkit_file()
{
    nle_settings *s = &current_nle_ctx->settings;
    size_t len = strnlen(s->wizkit, sizeof(s->wizkit));
    if (!len) {
        return (FILE *) 0;
    }
    return fmemopen(s->wizkit, len, "r");
}

/*
 * Initializes the random number generator.
 * Originally in hacklib.c.
 */
void
init_random(int FDECL((*fn), (int) ))
{
#ifdef NLE_ALLOW_SEEDING
    nle_seeds_init_t *si = current_nle_ctx->seeds_init;
    if (si) {
        set_random(si->seeds[whichrng(fn)], fn);
        has_strong_rngseed = si->reseed;
        return;
    }
#endif
    set_random(sys_random_seed(), fn);
}

nle_ctx_t *
nle_start(nle_obs *obs, FILE *ttyrec, nle_seeds_init_t *seed_init,
          nle_settings *settings_p)
{
    nle_ctx_t *nle = init_nle(ttyrec, obs);
    nle->settings = *settings_p;
    nle->seeds_init = seed_init;

    /* Apply difficulty-knob overrides supplied at start, BEFORE the mainloop
     * below generates the first level (mklev). tune_n == 0 leaves the
     * vanilla defaults untouched. */
    {
        int k;
        double *tunep = (double *) &nle->knobs;
        int ncat = nle_tune_count();
        for (k = 0; k < settings_p->tune_n && k < NLE_TUNE_MAX; k++) {
            int idx = settings_p->tune_idx[k];
            if (idx >= 0 && idx < ncat)
                tunep[idx] = settings_p->tune_val[k];
        }
    }

    nle->stack = create_fcontext_stack(STACK_SIZE);
    nle->generatorcontext =
        make_fcontext(nle->stack.sptr, nle->stack.ssize, mainloop);

    nle_anchor(nle);
    /* Set CO and LI to control ttyrec output size. */
    CO = NLE_TERM_CO;
    LI = NLE_TERM_LI;
    fcontext_transfer_t t = jump_fcontext(nle->generatorcontext, NULL);
    nle->generatorcontext = t.ctx;
    nle->done = (t.data == NULL);
    obs->done = nle->done;
    nle->seeds_init =
        NULL; /* Don't set to *these* seeds on subsequent reseeds, if any. */

    if (nle->ttyrec) {
        if (obs->blstats) {
            /* See comment in `nle_step`. We record the score in line with
             * the state to ensure s,r -> a -> s', r'. These lines ensure
             * we don't skip the first reward. */
            write_ttyrec_header(4, 2);
            write_ttyrec_data(&obs->blstats[9], 4);
        }
    }

    nle_sentinel_global_init();
    /* seed_init->seeds[0] is the primary dungeon seed for this env */
    nle->sentinel = nle_sentinel_register(
        seed_init ? (unsigned long) seed_init->seeds[0] : 0UL);

    return nle;
}

static long
nle_arena_off(char *base, void *p)
{
    return p ? (long) ((char *) p - base) : -1L;
}

/* Memory map of one environment: the regions a snapshot must cover, the live
 * monster (fmon) and object (fobj) chains, and the monster grid with
 * fmon-membership (stale grid pointers are flagged). Writes to `path`, or
 * stderr if NULL. */
void
nle_dbg_memmap(nle_ctx_t *nle, const char *path)
{
    FILE *f = path ? fopen(path, "w") : stderr;
    struct monst *m;
    struct obj *o;
    char *base;
    int x, y, n;
    if (!f)
        return;
    if (!nle) {
        if (path)
            fclose(f);
        return;
    }
    nle_anchor(nle);
    base = nle->arena_base;

    fprintf(f, "# nle whole-game memory map\n");
    fprintf(f, "obs_dlvl=%d moves=%ld\n\n", (int) depth(&u.uz), (long) moves);

    {
        extern size_t nle_rl_mirror_size(void);
        char *stk_hi = (char *) nle->stack.sptr;
        char *stk_lo = stk_hi - nle->stack.ssize;
        fprintf(f, "## whole-game regions (addr  size  what)\n");
        fprintf(f, "%18p  %10zu  nle_ctx_t + struct nh_globals (one block)\n",
                (void *) nle, nle_ctx_block_size());
        fprintf(f, "%18p  %10zu    of which struct nh_globals\n",
                (void *) nle->g, nh_globals_size());
        fprintf(f, "%18p  %10zu  arena (used; cap=%zu) -- all dynamic state\n",
                (void *) base, nle->arena_used, nle->arena_cap);
        fprintf(f, "%18p  %10zu  coroutine stack (fcontext)\n",
                (void *) stk_lo, nle->stack.ssize);
        fprintf(f, "%18p  %10zu  rl display mirror (libc, outside arena)\n",
                nle->rl_instance, nle_rl_mirror_size());
        fprintf(f, "\n");
    }
    fflush(f);

#define INAR(p) ((char *) (p) >= base \
                 && (char *) (p) < base + nle->arena_used)
    fprintf(f, "## monsters fmon (arena offset id mnum hp species)\n");
    for (m = fmon, n = 0; m && INAR(m) && n < 100000; m = m->nmon, n++) {
        int valid = (m->data >= &mons[0] && m->data < &mons[NUMMONS]);
        fprintf(f, "%10ld  id=%u mnum=%d hp=%d %s\n", nle_arena_off(base, m),
                m->m_id, m->mnum, m->mhp,
                valid ? mons[m->mnum].mname : "<BAD data>");
    }
    if (m && !INAR(m))
        fprintf(f, "  (fmon walk hit OOB ptr %p at #%d)\n", (void *) m, n);
    fflush(f);

    fprintf(f, "\n## objects fobj (offset id otyp)\n");
    for (o = fobj, n = 0; o && INAR(o) && n < 100000; o = o->nobj, n++)
        fprintf(f, "%10ld  id=%u otyp=%d\n", nle_arena_off(base, o), o->o_id,
                o->otyp);
    fflush(f);

    fprintf(f, "\n## grid monster ptrs (x y offset in_fmon valid_data)\n");
    for (x = 0; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++) {
            struct monst *gm = NH_G(level).monsters[x][y], *fm;
            int in = 0, fn = 0;
            if (!gm)
                continue;
            for (fm = fmon; fm && INAR(fm) && fn < 100000;
                 fm = fm->nmon, fn++)
                if (fm == gm) { in = 1; break; }
            fprintf(f, "%3d %3d  %10ld  in_fmon=%d valid_data=%d%s\n", x, y,
                    nle_arena_off(base, gm), in,
                    INAR(gm) && gm->data >= &mons[0]
                        && gm->data < &mons[NUMMONS],
                    in ? "" : "  <<< STALE GRID PTR");
        }
#undef INAR
    fflush(f);
    if (path)
        fclose(f);
}

nle_ctx_t *
nle_step(nle_ctx_t *nle, nle_obs *obs)
{
    nle_sentinel_beat(nle->sentinel, obs->action,
                      obs->blstats ? (int) obs->blstats[NLE_BL_DEPTH] : 0);
    /* Under round-robin stepping every call touches a different env's
     * block cold: hint the prefetcher at the head of the context. */
    __builtin_prefetch((const char *) nle +   0, 0, 3);
    __builtin_prefetch((const char *) nle +  64, 0, 3);
    __builtin_prefetch((const char *) nle + 128, 0, 3);
    __builtin_prefetch((const char *) nle + 192, 0, 3);
    nle_anchor(nle);
    nle->observation = obs;
    if (nle->ttyrec) {
        write_ttyrec_header(1, 1);
        write_ttyrec_data(&obs->action, 1);
    }
    fcontext_transfer_t t = jump_fcontext(nle->generatorcontext, obs);
    nle->generatorcontext = t.ctx;
    nle->done = (t.data == NULL);
    obs->done = nle->done;

    if (nle->ttyrec) {
        /* NLE ttyrec version 3 stores the action and in-game score in
         * different channels of the ttyrec. These channels are:
         *  - 0: the terminal instructions (classic ttyrec)
         *  - 1: the keypress/action (1 byte)
         *  - 2: the in-game score (4 bytes)
         *
         * We note the in-game score just before resuming the game, assuming
         * no chicanery has happened to the score after it is written to the
         * array `blstats`.
         *
         * Note: blstats[9] == botl_score which is used for score/reward fns.
         * see winrl.cc
         */
        if (obs->blstats) {
            write_ttyrec_header(4, 2);
            write_ttyrec_data(&obs->blstats[9], 4);
        }
    }

    return nle;
}

void
nle_end(nle_ctx_t *nle)
{
    nle_sentinel_unregister(nle->sentinel);
    nle->sentinel = NULL;
    nle_anchor(nle);
    if (!nle->done) {
        /* Reset without closing nethack. Need free memory, etc.
         * this is what nh_terminate in end.c does. I hope it's enough. */
        if (!NH_G(program_state).panicking) {
            freedynamicdata();
            dlb_cleanup();
        }
    }
    nle_fflush(stdout);

#ifdef NLE_BZ2_TTYRECS
    if (nle->ttyrec) {
        int bzerror;
        BZ2_bzWriteClose(&bzerror, nle->ttyrec_bz2, 0, NULL, NULL);
        assert(bzerror == BZ_OK);
    }
#endif

    tmt_close(nle->vterminal);

#ifdef __EMSCRIPTEN__
    /* The fiber backend heap-allocates a context (fib_t + asyncify stack) per
     * make_fcontext, so it has to be released alongside the stack. The asm
     * backends own no such memory and expose no destroy_fcontext. */
    destroy_fcontext(nle->generatorcontext);
    nle->generatorcontext = NULL;
#endif
    destroy_fcontext_stack(&nle->stack);
    if (nle->arena_base) {
        extern void nle_arena_registry_release(char *);
        nle_arena_registry_release(nle->arena_base);
        munmap(nle->arena_base, nle->arena_cap);
        nle->arena_base = NULL;
        nle->arena_used = 0;
        nle->arena_cap  = 0;
    }
    extern void nle_winrl_destroy_for_ctx(nle_ctx_t *);
    nle_winrl_destroy_for_ctx(nle);
    nle_anchor((nle_ctx_t *) 0);
    free(nle); /* the whole [ctx | globals] block */
}

#ifdef NLE_ALLOW_SEEDING
void
nle_set_seed(nle_ctx_t *nle, unsigned long core, unsigned long disp,
             boolean reseed)
{
    nle_anchor(nle);
    /* Keep up to date with rnglist[] in rnd.c. */
    set_random(core, rn2);
    set_random(disp, rn2_on_display_rng);

    /* Determines logic in reseed_random() in hacklib.c. */
    has_strong_rngseed = reseed;
};

void
nle_get_seed(nle_ctx_t *nle, unsigned long *core, unsigned long *disp,
             boolean *reseed)
{
    nle_anchor(nle);
    /* nle_seeds[] (hacklib.c) records the last seed set for each RNG. */
    *core = nle_seeds[0];
    *disp = nle_seeds[1];
    *reseed = has_strong_rngseed;
}
#endif

/* ===================================================================
 * Single-level blob save/load.
 * =================================================================== */

/* Serialize the CURRENT dungeon level to a malloc'd byte blob.
 * Reuses NetHack's own savelev(WRITE_SAVE) into the per-env levelfile
 * on disk, then slurps the bytes back. Caller frees via nle_free_blob.
 * Returns the blob (and writes its length to *out_len), or NULL on error. */
void *
nle_save_level(nle_ctx_t *nle, long *out_len)
{
    int fd, ledger;
    char errbuf[BUFSZ];
    const char *fq;
    long sz;
    void *blob;
    FILE *fp;

    nle_anchor(nle);
    if (out_len)
        *out_len = 0;

    ledger = ledger_no(&u.uz);
    /* Write the in-memory current level to its <lock>.<ledger> file.
     * WRITE_SAVE without FREE_SAVE so the live level stays intact. */
    fd = create_levelfile(ledger, errbuf);
    if (fd < 0)
        return (void *) 0;
    /* Exactly the do.c goto_level levelfile shape: no version header,
     * just savelev() bytes. bufon/bufoff bracket the zerocomp stream. */
    bufon(fd);
    savelev(fd, ledger, WRITE_SAVE);
    bflush(fd);
    bufoff(fd);
    nhclose(fd);

    /* Slurp the file back into a blob. */
    set_levelfile_name(lock, ledger);
    fq = fqname(lock, LEVELPREFIX, 0);
    fp = fopen(fq, "rb");
    if (!fp)
        return (void *) 0;
    fseek(fp, 0, SEEK_END);
    sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz <= 0) { /* ftell error or empty file: nothing valid to return */
        fclose(fp);
        return (void *) 0;
    }
    blob = malloc((size_t) sz);
    if (!blob) {
        fclose(fp);
        return (void *) 0;
    }
    if (fread(blob, 1, (size_t) sz, fp) != (size_t) sz) {
        free(blob);
        fclose(fp);
        return (void *) 0;
    }
    fclose(fp);
    if (out_len)
        *out_len = sz;
    return blob;
}

/* Release a blob returned by nle_save_level & co (libc memory, so bypass
 * the arena-aware `free` macro). */
void
nle_free_blob(void *blob)
{
    extern void __libc_free(void *);
    __libc_free(blob);
}

/* Load a level blob (from nle_save_level) as the CURRENT level of this
 * (already-started) game. The game must have been started (nle_start) so
 * the dungeon graph / u.uz / player struct exist; we overwrite the level
 * CONTENTS in place.
 *
 * Two-phase: this mutates state and resets vision but does NOT re-render.
 * Returns 0 on success, nonzero on error. */
int
nle_load_level(nle_ctx_t *nle, const void *blob, long len)
{
    int fd, ledger;
    char errbuf[BUFSZ];
    const char *fq;

    nle_anchor(nle);
    if (!blob || len <= 0) /* reject NULL/empty blobs before touching disk */
        return 4;
    ledger = ledger_no(&u.uz);

    /* Stamp the blob over the target ledger's levelfile, then getlev it. */
    set_levelfile_name(lock, ledger);
    fq = fqname(lock, LEVELPREFIX, 0);
    {
        int wfd = open(fq, O_WRONLY | O_CREAT | O_TRUNC, FCMASK);
        if (wfd < 0)
            return 1;
        if (write(wfd, blob, (size_t) len) != (ssize_t) len) {
            close(wfd);
            return 2;
        }
        close(wfd);
        level_info[ledger].flags |= LFILE_EXISTS;
    }

    fd = open_levelfile(ledger, errbuf);
    if (fd < 0)
        return 3;

    minit();                 /* ZEROCOMP reader init */
    /* Discard the live current level so getlev's allocations don't leak/
     * collide; FREE_SAVE tears down monsters/objs/timers of current level. */
    savelev(-1, ledger, FREE_SAVE);

    /* Pass pid=0, lev=0 to skip getlev()'s "is this the level/pid I expect?"
     * sanity check. A standalone load DELIBERATELY installs an arbitrary level
     * blob into the current ledger slot, so a mismatch is normal: e.g. resuming
     * a checkpoint taken on dungeon level 5 stamps that blob over the level-1
     * slot. With the check on, getlev() would call trickery() -> pline(...) ->
     * done(TRICKED), and the pline() routes through the rl window port, which
     * yields the game coroutine (jump_fcontext) from THIS (main) context -> jump
     * to a dead fcontext -> SIGSEGV. (pid=0 likewise makes cross-process resume
     * safe, since a checkpoint saved in another server run has a different
     * hackpid.) pid/lev are used nowhere else in getlev(). */
    getlev(fd, 0, (xchar) 0, FALSE);
    nhclose(fd);

    /* Standalone-load context fixups: place the hero on a sane, walkable
     * tile of the LOADED level. The destination game's u.ux/u.uy is stale
     * relative to the swapped-in contents and may land on rock/void, so we
     * re-seat in priority order: the level's upstairs, else its downstairs,
     * else the first ACCESSIBLE tile found by scanning the map. */
    if (xupstair) {
        u_on_newpos(xupstair, yupstair);
    } else if (xdnstair) {
        u_on_newpos(xdnstair, ydnstair);
    } else {
        int x, y;
        boolean placed = FALSE;

        for (x = 1; x < COLNO && !placed; x++)
            for (y = 0; y < ROWNO && !placed; y++)
                if (ACCESSIBLE(levl[x][y].typ)) {
                    u_on_newpos(x, y);
                    placed = TRUE;
                }
    }

    /* docrt()/flush_screen()/pline() route through the rl window port, which
     * YIELDS the game coroutine (jump_fcontext). They MUST NOT be called from
     * this entry point (main context) or we jump to a dead fcontext and
     * SIGSEGV. vision_reset() is pure computation (no window calls), so it is
     * safe here; the actual re-render happens on the next nle_step(), which
     * runs docrt() inside the coroutine. */
    vision_reset();
    return 0;
}

/* ===================================================================
 * Secure state-modification API.
 *
 * A curated whitelist of player-field pokes plus a deferred dungeon-level
 * jump. The C side only exposes the setters; the binding validates bounds.
 * =================================================================== */

/* Poke a single whitelisted integer player field. Returns 0 on success,
 * nonzero for an unknown field name. */
int
nle_set_state(nle_ctx_t *nle, const char *field, long value)
{
    nle_anchor(nle);
    if (!field)
        return 1;

    if (!strcmp(field, "hp")) {
        u.uhp = (int) value;
    } else if (!strcmp(field, "max_hp")) {
        u.uhpmax = (int) value;
    } else if (!strcmp(field, "hunger")) {
        /* set the food counter, then recompute the derived hunger STATE
         * (Hungry/Weak/Fainting/...) so blstats/encumbrance stay coherent. */
        u.uhunger = (int) value;
        newuhs(FALSE);
    } else if (!strcmp(field, "xp_level")) {
        int lev = (int) value;

        if (lev < 1)
            lev = 1;
        if (lev > MAXULEV)
            lev = MAXULEV;
        u.ulevel = lev;
        if (u.ulevelmax < u.ulevel)
            u.ulevelmax = u.ulevel;
        /* Keep experience points consistent with the new level: bump uexp
         * up to the threshold for this level if it is currently too low, so
         * the level does not immediately get clobbered by newexplevel(). */
        if (u.uexp < newuexp(u.ulevel - 1))
            u.uexp = newuexp(u.ulevel - 1);
    } else if (!strcmp(field, "gold")) {
        /* Gold is not a scalar field: it is a COIN_CLASS object in invent,
         * and blstats[GOLD] == money_cnt(invent) == that object's quan.
         * Adjust the existing gold object's quan, or create one if absent. */
        struct obj *gold = findgold(invent);

        if (value < 0L)
            value = 0L;
        if (!gold && value > 0L) {
            gold = mkgold(value, u.ux, u.uy);
            if (gold) {
                obj_extract_self(gold); /* remove from floor pile */
                gold->quan = value;
                gold->owt = weight(gold);
                addinv(gold);
            }
        } else if (gold) {
            gold->quan = value;
            gold->owt = weight(gold);
            if (value == 0L) {
                /* an empty coin stack should not linger in inventory */
                extract_nobj(gold, &invent);
                dealloc_obj(gold);
            }
        }
    } else if (!strcmp(field, "luck")) {
        /* Directly set the hero's intrinsic luck (u.uluck). Effective luck is
         * Luck == u.uluck + u.moreluck, where moreluck is the luckstone bonus
         * (+LUCKADD when carrying a blessed/uncursed luck stone). Normal
         * intrinsic luck is bounded LUCKMIN..LUCKMAX (-10..10); the maximum
         * *effective* luck achievable in vanilla play is LUCKMAX + LUCKADD
         * (10 + 3 == 13), so the injected value is clamped to [-13, 13]. */
        int lk = (int) value;

        if (lk < -13)
            lk = -13;
        if (lk > 13)
            lk = 13;
        u.uluck = (schar) lk;
    } else if (!strcmp(field, "str") || !strcmp(field, "dex")
               || !strcmp(field, "con") || !strcmp(field, "int")
               || !strcmp(field, "wis") || !strcmp(field, "cha")) {
        /* Set a single attribute (base + max). The caller passes NetHack's
         * encoded value: 3..18 normal, 19..118 == 18/01..18/00 strength
         * percentile, 119..125 == 19..25 (exceptional, magic only). */
        int idx = (!strcmp(field, "str")) ? A_STR
                : (!strcmp(field, "int")) ? A_INT
                : (!strcmp(field, "wis")) ? A_WIS
                : (!strcmp(field, "dex")) ? A_DEX
                : (!strcmp(field, "con")) ? A_CON
                : A_CHA;
        int v = (int) value;

        if (v < 3)
            v = 3;
        if (v > 125)
            v = 125;
        u.acurr.a[idx] = (schar) v;
        if (u.amax.a[idx] < (schar) v)
            u.amax.a[idx] = (schar) v;
    } else {
        return 1; /* unknown field */
    }

    context.botl = TRUE; /* bottom-line status is now stale */
    return 0;
}

/* Schedule a DEFERRED move of the hero to dungeon level n within the
 * current dungeon branch. The game loop (allmain.c) processes u.utotype
 * via deferred_goto() after rhack(), so this is safe to call from the
 * ctypes entry point: the actual goto_level() runs in-context on the next
 * nle_step(). Returns 0 on success, nonzero on an out-of-range target. */
int
nle_goto_depth(nle_ctx_t *nle, int n)
{
    d_level dest;

    nle_anchor(nle);

    if (n < 1 || n > (int) dunlevs_in_dungeon(&u.uz))
        return 1;

    dest.dnum = u.uz.dnum; /* stay in the current branch */
    dest.dlevel = (xchar) n;

    if (on_level(&u.uz, &dest))
        return 0; /* already there; nothing to schedule */

    /* schedule_goto sets u.utolev + u.utotype; deferred_goto() consumes
     * them on the next step. at_stairs/falling/portal all FALSE so the hero
     * lands on the destination's normal entry tile. */
    schedule_goto(&dest, FALSE, FALSE, 0, (char *) 0, (char *) 0);
    return 0;
}

/* Seat the hero on the down (or up) staircase of the current level, if present.
 * Two-phase like goto_depth: caller steps once to re-render. Returns 0 on
 * success, nonzero if the requested stair does not exist on this level. */
int
nle_seat_on_stair(nle_ctx_t *nle, int down)
{
    nle_anchor(nle);

    if (down && xdnstair > 0) {
        u_on_newpos(xdnstair, ydnstair);
    } else if (!down && xupstair > 0) {
        u_on_newpos(xupstair, yupstair);
    } else {
        return 1; /* no such stair on this level */
    }

    context.botl = TRUE; /* hero position / status is now stale */
    return 0;
}

/* Real level-up: raise the hero n experience levels with the normal HP/stat
 * gains (pluslvl). Also bumps u.uexp to the new level threshold so the next
 * newexplevel() won't undo it. Caller steps once to refresh blstats.
 * Returns 0 on success. */
int
nle_level_up(nle_ctx_t *nle, int n)
{
    int i;
    boolean saved_window_inited;

    nle_anchor(nle);

    /* pluslvl() emits messages (You_feel/pline "Welcome to experience
     * level N"). Emitting through the window port from this bare entry
     * point (outside nle_step's render context) yields the coroutine and
     * crashes. Temporarily clear window_inited so pline() falls back to
     * raw_print (safe: no coroutine yield), then restore it. The caller
     * steps once afterward to re-render normally. */
    saved_window_inited = iflags.window_inited;
    iflags.window_inited = FALSE;

    for (i = 0; i < n && u.ulevel < 30; i++)
        pluslvl(FALSE);

    iflags.window_inited = saved_window_inited;

    /* Keep experience points consistent with the new level (mirrors the
     * xp_level setter in nle_set_state). */
    if (u.uexp < newuexp(u.ulevel - 1))
        u.uexp = newuexp(u.ulevel - 1);

    context.botl = TRUE; /* bottom-line status is now stale */
    return 0;
}

/* ===================================================================
 * Curriculum traversal: cross-branch goto + dungeon-table query.
 * nle_goto_depth can only move WITHIN the current branch (it pins
 * dest.dnum = u.uz.dnum) and is clamped to that branch's length, so it
 * cannot reach Gehennom (levels ~26-50) or the Elemental Planes. These
 * entry points expose the dungeon layout and an arbitrary (dnum, dlevel)
 * jump so a curriculum can stitch e.g. DoD 1-3 to Gehennom 48-50.
 * =================================================================== */

extern int NDECL(nle_n_dgns); /* dungeon.c */

/* Number of dungeon branches currently defined (DoD, Gehennom, Mines, ...). */
int
nle_num_dungeons(nle_ctx_t *nle)
{
    nle_anchor(nle);
    return nle_n_dgns();
}

/* Report the layout of dungeon branch `idx`: its name, logical depth_start
 * (the absolute depth of its first level) and number of levels. Any of the
 * out pointers may be NULL. Returns 0 on success, 1 if idx is out of range. */
int
nle_dungeon_info(nle_ctx_t *nle, int idx, char *name_out, int name_cap,
                 int *depth_start_out, int *num_dunlevs_out)
{
    nle_anchor(nle);
    if (idx < 0 || idx >= nle_n_dgns())
        return 1;
    if (name_out && name_cap > 0) {
        (void) strncpy(name_out, dungeons[idx].dname, (size_t) (name_cap - 1));
        name_out[name_cap - 1] = '\0';
    }
    if (depth_start_out)
        *depth_start_out = dungeons[idx].depth_start;
    if (num_dunlevs_out)
        *num_dunlevs_out = (int) dungeons[idx].num_dunlevs;
    return 0;
}

/* Schedule a DEFERRED move of the hero to an ARBITRARY (dnum, dlevel),
 * including a dungeon branch other than the hero's current one (e.g.
 * Gehennom or the Elemental Planes). Two-phase like nle_goto_depth: the
 * actual goto_level() runs via deferred_goto() on the next nle_step(), which
 * handles cross-branch movement and generates the destination level on
 * demand (mklev) if it has not been visited.
 *
 * Entering the endgame (the Elemental Planes) requires the Amulet of Yendor
 * (goto_level returns early without it); we grant it here so the curriculum
 * can reach the planes.
 *
 * Returns 0 on success, nonzero for an out-of-range (dnum, dlevel). */
int
nle_goto_abs(nle_ctx_t *nle, int dnum, int dlevel)
{
    d_level dest;

    nle_anchor(nle);

    if (dnum < 0 || dnum >= nle_n_dgns())
        return 1;
    if (dlevel < 1 || dlevel > (int) dungeons[dnum].num_dunlevs)
        return 1;

    dest.dnum = (xchar) dnum;
    dest.dlevel = (xchar) dlevel;

    if (on_level(&u.uz, &dest))
        return 0; /* already there; nothing to schedule */

    /* The endgame gate in goto_level() needs the Amulet. */
    if (dest.dnum == astral_level.dnum)
        u.uhave.amulet = 1;

    schedule_goto(&dest, FALSE, FALSE, 0, (char *) 0, (char *) 0);
    return 0;
}

/* Report whether the hero is standing on a staircase: +1 on the down stair,
 * -1 on the up stair, 0 otherwise. The BRANCH staircase (sstairs) -- e.g. the
 * Gnomish Mines entrance -- reports with a DISTINCT magnitude (2 = branch
 * down, -2 = branch up) so callers can tell it apart from the level's own
 * main stair. */
int
nle_hero_on_stair(nle_ctx_t *nle)
{
    nle_anchor(nle);
    if (xdnstair > 0 && u.ux == xdnstair && u.uy == ydnstair)
        return 1;
    if (xupstair > 0 && u.ux == xupstair && u.uy == yupstair)
        return -1;
    if (sstairs.sx > 0 && u.ux == sstairs.sx && u.uy == sstairs.sy)
        return sstairs.up ? -2 : 2;
    return 0;
}

/* Grant the pre-primed invocation kit straight into the hero's pack so the
 * curriculum agent can actually perform the invocation ritual (the only way
 * down from the Invocation level to Moloch's Sanctum -- that level has no
 * down-staircase by design). Build objects with mksobj, then addinv() (which
 * also sets u.uhave.menorah/bell/book via addinv_core1, so the ritual's
 * carry-checks pass).
 *
 * The kit is pre-primed so the agent does NOT have to hunt candles or light
 * anything: the Candelabrum arrives with all 7 candles (spe=7) and lit, the
 * Bell is charged, and all three are uncursed and pre-identified. The Bell is
 * intentionally NOT pre-rung -- the honest flow has the agent ring it live. */
int
nle_grant_invocation_kit(nle_ctx_t *nle)
{
    struct obj *cand, *bell, *book;

    nle_anchor(nle);

    /* Candelabrum of Invocation: 7 candles attached, uncursed, lit w/ deep fuel. */
    cand = mksobj(CANDELABRUM_OF_INVOCATION, TRUE, FALSE);
    if (cand) {
        cand->spe = 7;        /* all 7 candles (deadbook checks spe==7) */
        cand->cursed = 0;
        cand->blessed = 0;
        cand->age = 5000L;    /* turns of fuel; outlasts any navigation */
        cand->quan = 1L;
        cand->known = cand->dknown = cand->bknown = cand->rknown = 1;
        cand->owt = weight(cand);
        makeknown(CANDELABRUM_OF_INVOCATION);
        cand = addinv(cand);      /* sets u.uhave.menorah; returns live ptr */
        if (cand)
            begin_burn(cand, FALSE);  /* lamplit=1 + burn timer + light source */
    }

    /* Bell of Opening: charged, uncursed, NOT pre-rung (agent rings it live). */
    bell = mksobj(BELL_OF_OPENING, TRUE, FALSE);
    if (bell) {
        bell->spe = 3;
        bell->cursed = 0;
        bell->blessed = 0;
        bell->known = bell->dknown = bell->bknown = bell->rknown = 1;
        bell->owt = weight(bell);
        makeknown(BELL_OF_OPENING);
        (void) addinv(bell);      /* sets u.uhave.bell */
    }

    /* Book of the Dead: uncursed. */
    book = mksobj(SPE_BOOK_OF_THE_DEAD, TRUE, FALSE);
    if (book) {
        book->cursed = 0;
        book->blessed = 0;
        book->known = book->dknown = book->bknown = book->rknown = 1;
        book->owt = weight(book);
        makeknown(SPE_BOOK_OF_THE_DEAD);
        (void) addinv(book);      /* sets u.uhave.book */
    }

    context.botl = TRUE;
    return 0;
}

/* Report the vibrating-square (invocation) position. Writes inv_pos into
 * *x,*y and returns 0 on the Invocation level; otherwise writes (0,0) and
 * returns nonzero (inv_pos is only meaningful on that level). */
int
nle_invocation_pos(nle_ctx_t *nle, int *x, int *y)
{
    nle_anchor(nle);
    if (x)
        *x = 0;
    if (y)
        *y = 0;
    if (!Invocation_lev(&u.uz))
        return 1;
    if (x)
        *x = (int) inv_pos.x;
    if (y)
        *y = (int) inv_pos.y;
    return 0;
}

/* Stage the hero at the vibrating (invocation) square. With adjacent==0 the
 * hero lands ON the square; with adjacent!=0 the hero lands on an accessible,
 * unoccupied tile next to it (so the agent takes one honest step onto the
 * square before ringing the Bell / reading the Book). Returns 0 on the
 * Invocation level (hero relocated), nonzero otherwise. */
int
nle_seat_on_invocation_square(nle_ctx_t *nle, int adjacent)
{
    int dx, dy, nx, ny;
    static const int ord8[8][2] = {
        { 0, -1 }, { 0, 1 }, { -1, 0 }, { 1, 0 },
        { -1, -1 }, { 1, -1 }, { -1, 1 }, { 1, 1 }
    };
    int i;

    nle_anchor(nle);
    if (!Invocation_lev(&u.uz) || inv_pos.x <= 0)
        return 1;

    if (!adjacent) {
        u_on_newpos(inv_pos.x, inv_pos.y);
        context.botl = TRUE;
        return 0;
    }

    /* Find an accessible, monster-free tile next to the square. */
    for (i = 0; i < 8; i++) {
        dx = ord8[i][0];
        dy = ord8[i][1];
        nx = inv_pos.x + dx;
        ny = inv_pos.y + dy;
        if (isok(nx, ny) && ACCESSIBLE(levl[nx][ny].typ)
            && !m_at(nx, ny) && !(nx == inv_pos.x && ny == inv_pos.y)) {
            u_on_newpos(nx, ny);
            context.botl = TRUE;
            return 0;
        }
    }
    /* No free neighbor (fully walled/occupied) -- fall back to the square. */
    u_on_newpos(inv_pos.x, inv_pos.y);
    context.botl = TRUE;
    return 0;
}

/* From unixtty.c */
/* fatal error */
/*VARARGS1*/
void error
VA_DECL(const char *, s)
{
    VA_START(s);
    VA_INIT(s, const char *);

    if (iflags.window_inited)
        exit_nhwindows((char *) 0); /* for tty, will call settty() */

    fprintf(stderr, s, VA_ARGS);
    fprintf(stderr, "\n");
    VA_END();
    nethack_exit(EXIT_FAILURE);
}

/* From unixtty.c. There is no real terminal: these stay zero for the whole
 * process (tools/collect_globals/whitelist.txt). */
char erase_char, intr_char, kill_char;

void
gettty()
{
    /* Should set erase_char, intr_char, kill_char */
}

void
settty(const char *s)
{
    end_screen();
    if (s)
        raw_print(s);
}

void
setftty()
{
    start_screen();

    iflags.cbreak = ON;
    iflags.echo = OFF;
}

void
intron()
{
}

void
introff()
{
}

#ifdef __linux__ /* via Jesse Thilo and Ben Gertzfield */
#include <sys/ioctl.h>
#include <sys/vt.h>

/* whether stdin is a Linux console: process-level terminal identity */
int linux_flag_console = 0;

void NDECL(linux_mapon);
void NDECL(linux_mapoff);
void NDECL(check_linux_console);
void NDECL(init_linux_cons);

void
linux_mapon()
{
#ifdef TTY_GRAPHICS
    if (WINDOWPORT("tty") && linux_flag_console) {
        write(1, "\033(B", 3);
    }
#endif
}

void
linux_mapoff()
{
#ifdef TTY_GRAPHICS
    if (WINDOWPORT("tty") && linux_flag_console) {
        write(1, "\033(U", 3);
    }
#endif
}

void
check_linux_console()
{
    struct vt_mode vtm;

    if (isatty(0) && ioctl(0, VT_GETMODE, &vtm) >= 0) {
        linux_flag_console = 1;
    }
}

void
init_linux_cons()
{
#ifdef TTY_GRAPHICS
    if (WINDOWPORT("tty") && linux_flag_console) {
        atexit(linux_mapon);
        linux_mapoff();
#ifdef TEXTCOLOR
        /*if (has_colors())*/ /* Assume true in NLE. */
        iflags.use_color = TRUE;
#endif
    }
#endif
}
#endif /* __linux__ */
