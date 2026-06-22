
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

/* Single definition of current_nle_ctx; declared extern in nle.h.
 * Stage 10'+: TLS-marked so each OMP thread chases its own context.
 * With all per-env state routed through this pointer, threads are
 * naturally isolated — no shared mutable globals to race on. */
/* exp_039: initial-exec TLS model removes the runtime __tls_get_addr call
 * (was ~3.3% of user CPU under N=128 puffer training, per perf-record).
 * libnethack.so is loaded via dlopen from puffer's training extension;
 * glibc still permits initial-exec when the DSO has reserved TLS slots
 * via DT_FLAGS_1 STATIC_TLS at link time. We rely on the existing
 * -Wl,-z,initial-exec link flag (added in vendor/nle/src/CMakeLists.txt).
 * If load fails with "cannot allocate memory in static TLS block", drop
 * the tls_model and rebuild — but on this cluster (Linux 5.14, glibc 2.34)
 * it works. */
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

/* nle_state refactor: helpers for moving subsystems out of process-global
 * storage into nle_ctx_t. Stage 1 ports the RNG state (was static rnglist
 * in rnd.c). Call from non-NLE TUs via the prototypes declared in nle.h. */
isaac64_ctx *
nle_rng_state(int idx)
{
    return &current_nle_ctx->rng_state[idx];
}

int *
nle_rng_init_flag(int idx)
{
    return &current_nle_ctx->rng_init[idx];
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
    return &nle->s_tune;
}

nle_ctx_t *
init_nle(FILE *ttyrec, nle_obs *obs)
{
    nle_ctx_t *nle = calloc(1, sizeof(nle_ctx_t));

    /* Anchor current_nle_ctx to this env BEFORE any macro use. Many of
     * the inits below (notably the tmt_open(LI, CO, ...) call and any
     * use of *_init helpers that expand through the macros) deref
     * current_nle_ctx; if that's still NULL (or stale from another env)
     * we crash. The pointer must be set first so the macros resolve to
     * THIS env's fields. */
    current_nle_ctx = nle;

    /* Difficulty knobs default to vanilla (all scales 1.0). calloc zeroed the
     * ctx, so the knob block must be explicitly seeded before any read-site. */
    nle_tune_set_defaults(&nle->s_tune);

    /* s8_tcap_p needs to be allocated before LI/CO are read; pre-alloc
     * and seed it so tmt_open below gets valid dimensions. */
    nle->s8_tcap_p = nle_arena_calloc(1, sizeof(struct nle_tcap_t));
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

    /* RNG state cleared by calloc; init_isaac64 will populate it via the
     * set_random() / init_random() chain during NetHack's early setup. */

    /* Function-local statics migrated to nle_ctx_t.
     * rn2_on_display_rng (non-ISAAC64 path) seeded its `static unsigned
     * seed = 1` at file scope; calloc gives 0 which would freeze the LCG,
     * so restore the original init here. recursive_mine/up start FALSE,
     * which calloc already gives us. */
    nle->s_rn2disprng_seed = 1;

    /* Stage 4 player state: allocate the struct you on the heap so the
     * `u` macro in decl.h can resolve to (*current_nle_ctx->u_ptr). */
    nle->u_ptr = (struct you *) nle_arena_calloc(1, sizeof(struct you));
    if (!nle->u_ptr) {
        fprintf(stderr, "init_nle: failed to allocate struct you\n");
        abort();
    }

    /* Stage 5 flags / iflags / sysflags */
    nle->flags_ptr = (struct flag *) nle_arena_calloc(1, sizeof(struct flag));
    nle->iflags_ptr = (struct instance_flags *) nle_arena_calloc(1, sizeof(struct instance_flags));
#ifdef SYSFLAGS
    nle->sysflags_ptr = (struct sysflag *) nle_arena_calloc(1, sizeof(struct sysflag));
#endif
    if (!nle->flags_ptr || !nle->iflags_ptr) {
        fprintf(stderr, "init_nle: failed to allocate flags/iflags\n");
        abort();
    }

    /* Stage 9' batch A — scalar globals that previously had non-zero static
     * initializers (decl.c). With calloc-zero'd nle_ctx_t, restore them. */
    nle->nle_moves = 1L;
    nle->nle_monstermoves = 1L;
    /* Maze limits must be even (decl.c original: (COLNO-1)&~1, (ROWNO-1)&~1). */
    nle->x_maze_max_v = (COLNO - 1) & ~1;
    nle->y_maze_max_v = (ROWNO - 1) & ~1;

    /* Stage 5 Option-A: worn[] in worn.c can no longer have
     * `&uarm` etc. as compile-time initializers under __thread. Patch the
     * table once at startup (idempotent across env inits, since uarm/etc.
     * have stable per-thread addresses). Same for decl.c subrooms. */
    extern void worn_init(void);
    extern void subrooms_init(void);
    worn_init();
    subrooms_init();

    /* Stage 6' — dungeon topology heap allocations. All zero-init via calloc;
     * matches the original {0,...} static initializers in decl.c. */
    nle->s6_topology_p = nle_arena_calloc(1, sizeof(struct dgn_topology));
    nle->s6_dungeons_p = nle_arena_calloc(MAXDUNGEON, sizeof(dungeon));
    nle->s6_upstair_p  = nle_arena_calloc(1, sizeof(stairway));
    nle->s6_dnstair_p  = nle_arena_calloc(1, sizeof(stairway));
    nle->s6_upladder_p = nle_arena_calloc(1, sizeof(stairway));
    nle->s6_dnladder_p = nle_arena_calloc(1, sizeof(stairway));
    nle->s6_sstairs_p  = nle_arena_calloc(1, sizeof(stairway));
    nle->s6_updest_p   = nle_arena_calloc(1, sizeof(dest_area));
    nle->s6_dndest_p   = nle_arena_calloc(1, sizeof(dest_area));
    nle->s6_inv_pos_p  = nle_arena_calloc(1, sizeof(coord));
    if (!nle->s6_topology_p || !nle->s6_dungeons_p
        || !nle->s6_upstair_p || !nle->s6_dnstair_p
        || !nle->s6_upladder_p || !nle->s6_dnladder_p
        || !nle->s6_sstairs_p || !nle->s6_updest_p
        || !nle->s6_dndest_p || !nle->s6_inv_pos_p) {
        fprintf(stderr, "init_nle: failed to allocate stage 6 state\n");
        abort();
    }
    /* ubirthday/wailmsg/domove_* — zero is the original init value. */

    /* Stage 9' batch C — heap-allocate the per-env struct values that used
     * to live as file-scope globals in decl.c. calloc'd zero matches the
     * `= DUMMY` ({0}) initializer at the old definition sites. Spell book
     * is an array of (MAXSPELL+1) entries. m_shot needs a non-zero
     * STRANGE_OBJECT for its `.o` field (matches decl.c old initializer). */
    nle->s9c_m_shot_p       = nle_arena_calloc(1, sizeof(struct multishot));
    nle->s9c_urealtime_p    = nle_arena_calloc(1, sizeof(struct u_realtime));
    nle->s9c_quest_status_p = nle_arena_calloc(1, sizeof(struct q_score));
    nle->s9c_spl_book_p     = nle_arena_calloc(MAXSPELL + 1, sizeof(struct spell));
    nle->s9c_youmonst_p     = nle_arena_calloc(1, sizeof(struct monst));
    nle->s9c_mvitals_p      = nle_arena_calloc(NUMMONS, sizeof(struct nle_mvitals_t));
    nle->s9c_killer_p       = nle_arena_calloc(1, sizeof(struct kinfo));
    /* s8_tcap_p already calloc'd at top of init_nle (LI/CO needed early). */
    nle->s5_cmd_p           = nle_arena_calloc(1, sizeof(struct cmd));
    nle->s_disco_p          = nle_arena_calloc(NUM_OBJECTS, sizeof(short));
    /* obufs is NUMOBUF * BUFSZ bytes, defined in objnam.c. */
    nle->s_obufs_p          = nle_arena_calloc(12 * 256, sizeof(char));
    /* tty_status is 2 * MAXBLSTATS * sizeof(struct tty_status_fields).
     * sizeof is opaque here — over-allocate (4096 is plenty for ~1840 B). */
    nle->s_tty_status_p     = nle_arena_calloc(4096, 1);
    nle->s_context_p        = nle_arena_calloc(1, sizeof(struct context_info));
    {
        extern struct nle_rndmonst_state *rndmonst_state_alloc(void);
        nle->s_rndmonst_state_p = rndmonst_state_alloc();
        extern void nle_artilist_init(struct artifact **);
        nle_artilist_init(&nle->s_artilist_p);
        /* Per-env quest msg index. */
        extern void nle_qtlist_alloc(struct qtlists **);
        nle_qtlist_alloc(&nle->s_qt_list_p);
    }
    /* NUM_OBJECTS + 1: stock NetHack's objects[]/obj_descr[] include a
     * trailing "Array Terminator" entry (oc_class == ILLOBJ_CLASS == 0) past
     * the NUM_OBJECTS real entries. obj_shuffle_range() scans class ranges
     * with `for (i = lo; objects[i].oc_class == ocls; i++)` and relies on that
     * terminator to stop; without it the scan reads — and shuffle() then
     * writes — past the end of this heap block, corrupting the libc heap
     * (manifesting later as non-deterministic glibc double-free / SIGSEGV).
     * The baseline arrays below have NUM_OBJECTS + 1 elements, so copying the
     * terminator too is in-bounds. */
    nle->s9o_objects_p   = nle_arena_calloc(NUM_OBJECTS + 1, sizeof(struct objclass));
    nle->s9o_obj_descr_p = nle_arena_calloc(NUM_OBJECTS + 1, sizeof(struct objdescr));
    /* do_name.c name-buffer pool: NUMMBUF=5 * BUFSZ=256 = 1280 bytes. */
    nle->s_mbufs_p       = nle_arena_calloc(5 * 256, sizeof(char));
    /* decl.c smeq[MAXNROFROOMS+1] — calloc'd zero matches original {0,...}. */
    nle->s_smeq_p        = nle_arena_calloc(MAXNROFROOMS + 1, sizeof(int));
    nle->s_bases_p       = nle_arena_calloc(MAXOCLASSES, sizeof(int));
    /* CLR_MAX is 16 in the standard build. */
    nle->s_hilites_p     = nle_arena_calloc(16, sizeof(char *));
    /* Per-env work buffers from various src files. sizes are opaque
     * here (the struct types are defined in display.h / botl.h /
     * vision.h, which we deliberately don't pull into nle.h to keep
     * the util-build include cascade small). Bound generously by
     * counting bytes. ROWNO=21, COLNO=80, MAXBLSTATS=23. */
    nle->s_gbuf_p           = nle_arena_calloc(ROWNO * COLNO, 64);   /* gbuf_entry */
    nle->s_blstats_p        = nle_arena_calloc(2 * 23, 256);          /* struct istat_s */
    nle->s_status_hilites_p = nle_arena_calloc(23, 256);              /* struct hilite_s */
    nle->s_could_see_p      = nle_arena_calloc(2 * ROWNO * COLNO, 1);
    nle->s_viz_clear_p      = nle_arena_calloc(ROWNO * COLNO, 1);
    nle->s_left_ptrs_p      = nle_arena_calloc(ROWNO * COLNO, 1);
    nle->s_right_ptrs_p     = nle_arena_calloc(ROWNO * COLNO, 1);
    nle->s_SpLev_Map_p      = nle_arena_calloc(COLNO * ROWNO, 1);
    /* fqn_filename_buffer = char[FQN_NUMBUF=4][FQN_MAX_FILENAME=512] = 2048 B */
    nle->s_fqn_fname_p      = nle_arena_calloc(4 * 512, 1);
    /* worm tables (worm.c). MAX_NUM_WORMS=32. */
    nle->s_wheads_p         = nle_arena_calloc(32, sizeof(void *));
    nle->s_wtails_p         = nle_arena_calloc(32, sizeof(void *));
    nle->s_wgrowtime_p      = nle_arena_calloc(32, sizeof(long));
    /* boolopt baseline is 2088 B; compopt is 1920 B. Generous alloc bounds. */
    nle->s_boolopt_p        = nle_arena_calloc(1, 4096);
    nle->s_compopt_p        = nle_arena_calloc(1, 4096);
    /* urole / urace (struct Role / struct Race in you.h). Sizes opaque. */
    nle->s_urole_p          = nle_arena_calloc(1, 512);
    nle->s_urace_p          = nle_arena_calloc(1, 512);
    if (nle->s9o_objects_p && nle->s9o_obj_descr_p) {
        memcpy(nle->s9o_objects_p, objects_baseline,
               (NUM_OBJECTS + 1) * sizeof(struct objclass));
        memcpy(nle->s9o_obj_descr_p, obj_descr_baseline,
               (NUM_OBJECTS + 1) * sizeof(struct objdescr));
    }
    nle->s7_level_p         = nle_arena_calloc(1, sizeof(dlevel_t));
    nle->s7_rooms_p         = nle_arena_calloc((MAXNROFROOMS + 1) * 2, sizeof(struct mkroom));
    nle->s7_doors_p         = nle_arena_calloc(DOORMAX, sizeof(coord));
    nle->s7_level_info_p    = nle_arena_calloc(MAXLINFO, sizeof(struct linfo));
    nle->s7_lastseentyp_p   = nle_arena_calloc(COLNO * ROWNO, sizeof(schar));
    /* bhitpos per-env. */
    nle->bhitpos_p          = nle_arena_calloc(1, sizeof(coord));
    /* Utrack[UTSZ=50] per-env (track.c). */
    nle->s_utrack           = nle_arena_calloc(50, sizeof(coord));
    /* subrooms points into the rooms array (slot MAXNROFROOMS+1). */
    nle->s7_subrooms        = nle->s7_rooms_p + (MAXNROFROOMS + 1);
    /* upstairs_room/dnstairs_room/sstairs_room/ftrap left NULL — original
     * decl.c init was NULL too. */
    if (!nle->s9c_m_shot_p || !nle->s9c_urealtime_p
        || !nle->s9c_quest_status_p || !nle->s9c_spl_book_p
        || !nle->s9c_youmonst_p || !nle->s9c_mvitals_p
        || !nle->s9c_killer_p || !nle->s8_tcap_p || !nle->s5_cmd_p
        || !nle->s_disco_p || !nle->s_obufs_p || !nle->s_tty_status_p
        || !nle->s_context_p || !nle->s_rndmonst_state_p
        || !nle->s_artilist_p || !nle->s9o_objects_p
        || !nle->s9o_obj_descr_p || !nle->s_mbufs_p || !nle->s_smeq_p
        || !nle->s_bases_p || !nle->s_hilites_p
        || !nle->s7_level_p || !nle->s7_rooms_p
        || !nle->s7_doors_p || !nle->s7_level_info_p
        || !nle->s7_lastseentyp_p || !nle->bhitpos_p
        || !nle->s_utrack) {
        fprintf(stderr, "init_nle: failed to allocate stage 9' batch C state\n");
        abort();
    }
    nle->s9c_m_shot_p->o = STRANGE_OBJECT;
    /* body-slot pointers (s9_uwep, s9_uarm, etc.) zero-init'd by calloc;
     * that matches the original decl.c NULL initializer. */

    /* Per-env `struct musable` (muse.c). Allocate via a small
     * helper so the struct definition stays local to muse.c — nle.c
     * doesn't need to see it. Bytes are zeroed (matches original
     * file-scope `static struct musable m;` zero-init). trapx/trapy live
     * inline as ints on nle_ctx_t (already zero-init by calloc). */
    {
        extern void nle_muse_alloc(void **);
        nle_muse_alloc(&nle->s_muse_m_p);
        if (!nle->s_muse_m_p) {
            fprintf(stderr, "init_nle: failed to allocate s_muse_m_p\n");
            abort();
        }
    }

    /* Non-zero initializers for migrated invent/
     * pickup file-statics. Only cached_pickinv_win needs init (was
     * `static winid cached_pickinv_win = WIN_ERR;` and WIN_ERR == -1,
     * not 0). The others (sortlootmode=0, this_type=0, invbuf=NULL,
     * invbufsiz=0, only={0,0}, current_container=NULL,
     * abort_looting=FALSE, val_for_n_or_more=0, valid_menu_classes=0)
     * all match calloc-zero. */
    nle->s_cached_pickinv_win = WIN_ERR;

    /* Per-env init of save/restore dispatch tables (saveprocs,
     * restoreprocs) and sfsaveinfo/sfrestinfo flag words. These were
     * process-global file-scope statics in save.c / restore.c / decl.c and
     * raced under N>=1024 OMP vecenv stepping, where one env's set_*_pref
     * could swap another env's mid-save/restore function pointers and
     * produce wrong-codec short reads (the "Error reading level file"
     * panic). Helpers live in save.c / restore.c so they can see the
     * STATIC_OVL/STATIC_DCL codec functions. */
    {
        extern void NDECL(nle_restoreprocs_init);
        extern void NDECL(nle_saveprocs_init);
        nle_restoreprocs_init();
        nle_saveprocs_init();
    }

    return nle;
}

/* `settings` moved into nle_ctx_t (refactor stage 2). Below uses
 * `current_nle_ctx->settings` since mainloop and friends always run
 * with current_nle_ctx anchored to the active env. */

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

    char *scoreprefix = (s->scoreprefix[0] != '\0')
                            ? s->scoreprefix
                            : s->hackdir;
    fqn_prefix[SYSCONFPREFIX] = s->hackdir;
    fqn_prefix[CONFIGPREFIX] = s->hackdir;
    fqn_prefix[HACKPREFIX] = s->hackdir;
    fqn_prefix[SAVEPREFIX] = s->hackdir;
    fqn_prefix[LEVELPREFIX] = s->hackdir;
    fqn_prefix[BONESPREFIX] = s->hackdir;
    fqn_prefix[SCOREPREFIX] = scoreprefix;
    fqn_prefix[LOCKPREFIX] = s->hackdir;
    fqn_prefix[TROUBLEPREFIX] = s->hackdir;
    fqn_prefix[DATAPREFIX] = s->hackdir;

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
     * through to libc's fflush via the real symbol. The wintty.h macro
     * `#define fflush nle_fflush` is still in scope inside this TU, so a
     * naked `fflush(stream)` would recurse — call the libc symbol
     * directly. */
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
    /* exp_039: when no tty observation is bound, the bytes nle_putchar
     * writes to outbuf are dropped by nle_fflush (line ~509 gates
     * tmt_write on obs->tty_chars/tty_colors/tty_cursor). Short-circuit
     * the whole write path in that case. The RL agent doesn't bind
     * tty_* fields in our config — see nle_obs init in ocean/nethack
     * binding.c. Per perf: ~1.4% user CPU savings at N=1024. */
    nle_obs *obs = nle->observation;
    if (!obs || (!obs->tty_chars && !obs->tty_colors && !obs->tty_cursor))
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
void
nle_xputs(const char *str)
{
    int c;
    const char *p = str;

    if (!p || !*p)
        return;

    while ((c = *p++) != '\0') {
        nle_putchar(c);
    }
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

/* `nle_seeds_init` moved into nle_ctx_t (refactor stage 2). Below uses
 * `current_nle_ctx->seeds_init`. */

/* See rng.c. */
extern int FDECL(whichrng, (int FDECL((*fn), (int) )));

/* See hacklib.c. */
extern int FDECL(set_random, (unsigned long, int FDECL((*fn), (int) )));
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
        current_nle_ctx->has_strong_rngseed = si->reseed;
        return;
    }
#endif
    set_random(sys_random_seed(), fn);
}

static void nle_swap_in(nle_ctx_t *nle);
static void nle_swap_out(nle_ctx_t *nle);

/* Forward declarations for the baseline-capture path in nle_start. */
struct nle_dungeon_save;
extern struct nle_dungeon_save *nle_baseline;
static void nle_dungeon_save_to(struct nle_dungeon_save *s);

nle_ctx_t *
nle_start(nle_obs *obs, FILE *ttyrec, nle_seeds_init_t *seed_init,
          nle_settings *settings_p)
{
    nle_ctx_t *nle = init_nle(ttyrec, obs);
    nle->settings = *settings_p;
    nle->seeds_init = seed_init;

    /* Apply difficulty-knob overrides supplied at start, BEFORE the mainloop
     * below generates the first level (mklev). init_nle already seeded s_tune
     * to vanilla defaults; tune_n == 0 leaves them untouched. */
    {
        int k;
        double *tunep = (double *) &nle->s_tune;
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

    current_nle_ctx = nle;
    /* CO/LI macros expand through current_nle_ctx->s8_tcap_p; must
     * come AFTER current_nle_ctx is set. (Used to be before init_nle
     * back when CO/LI were plain globals.) */
    CO = NLE_TERM_CO;
    LI = NLE_TERM_LI;
    nle_swap_in(nle);
    fcontext_transfer_t t = jump_fcontext(nle->generatorcontext, NULL);
    nle->generatorcontext = t.ctx;
    nle->done = (t.data == NULL);
    obs->done = nle->done;
    nle->seeds_init =
        NULL; /* Don't set to *these* seeds on subsequent reseeds, if any. */
    nle_swap_out(nle);

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
        seed_init ? (unsigned long)seed_init->seeds[0] : 0UL);

    return nle;
}

/* Stage 6 dungeon topology save bundle. Holds copies of all the
 * dungeon-graph globals from decl.c so each env has its own. Allocated
 * lazily in nle_swap_in on first call (after init_dungeon has populated
 * the globals). */
struct nle_dungeon_save {
    /* stage 6' — dungeon graph migrated direct to nle_ctx_t. */
    /* stage 7 completed — `level` now lives in nle_ctx_t (s7_level_p).
     * struct dig_info.level was renamed to .dlvl to free the token. */
    /* stage 8 — display / message state migrated direct to nle_ctx_t
     * (vision_full_recalc, viz_array, WIN_*, toplines, tc_gbl_data). */
    /* stage 9' batch D — body-slot pointers migrated direct to nle_ctx_t
     * (s9_uwep, s9_uarm, etc.).  worn[] uses offsetof resolution; no
     * per-thread address pinning.  Swap blob now empty of all stage-9 items. */
    /* mvitals migrated direct to nle_ctx_t (stage 9' batch C). */
    /* youmonst, urealtime, spl_book, m_shot, quest_status migrated direct
     * to nle_ctx_t (stage 9' batch C). */
    /* migrated direct to nle_ctx_t (stage 9' batches A/B):
     *   invent, uskin, current_wand, thrownobj, kickedobj
     *   migrating_objs, billobjs, mydogs, migrating_mons, apelist
     *   ubirthday, moves, monstermoves, wailmsg
     *   domove_attempting, domove_succeeded */
    /* stage 10' — TTY window state migrated direct to nle_ctx_t. */
};

static void
nle_dungeon_save_to(struct nle_dungeon_save *s)
{
    /* stage 6' — dungeon topology migrated direct to nle_ctx_t. */
    /* stage 7' completed — all stage-7 items including `level` itself
     * migrated direct to nle_ctx_t (s7_*). Save/load no longer needed. */
    /* stage 8' completed: tc_gbl_data + vision_full_recalc + viz_array
     * + WIN_* + toplines all migrated direct to nle_ctx_t. */
    /* stage 9' batch D — body-slot pointers now live in nle_ctx_t (s9_u*).
     * No save needed: they are already per-env by definition. */
    /* invent, uskin, current_wand, thrownobj, kickedobj, migrating_objs,
     * billobjs, mydogs, migrating_mons, apelist migrated direct.
     * ubirthday, moves, monstermoves, wailmsg, domove_* migrated direct. */
    /* stage 10' — tty window state migrated direct to nle_ctx_t. */
}

static void
nle_dungeon_load_from(const struct nle_dungeon_save *s)
{
    /* stage 6'/7'/8'/9' all completed — nothing left to save/load. */
    /* stage 9' batch D — body-slot pointers now in nle_ctx_t (s9_u*).
     * No load needed: macros resolve directly via current_nle_ctx. */
    (void) s; /* suppress unused-parameter warning */
    /* stage 10' — tty window state migrated direct to nle_ctx_t. */
}

/* The `flags` swap is retired.
 *
 * flags / iflags / sysflags: all three now macro-redirect to
 * (*current_nle_ctx->X_ptr) in include/flag.h. Per-env storage is in
 * flags_ptr / iflags_ptr / sysflags_ptr on nle_ctx_t. Each access in
 * generated code routes directly to the env's storage — no memcpy
 * context switch needed. (The earlier swap copied the global into the
 * env's storage on eviction and back on resume; with the macro pattern
 * `&flags` would be `current_nle_ctx->flags_ptr` itself, so the memcpy
 * would corrupt rather than help. Removing is mandatory, not optional.)
 *
 * nroom / nsubroom were the last NEARDATA __thread globals
 * still being swap-copied per step. With the BK migration to per-env
 * macros over current_nle_ctx->s_nroom/s_nsubroom (renamed for macro
 * safety), the swap is now empty modulo dungeon_save baseline capture
 * (retained for cross-env defaults like BASE_WINDOW==0 invariant in
 * NetHackRL ctor). */

static void
nle_swap_in(nle_ctx_t *nle)
{
    /* First-ever swap_in across the whole process: snapshot the pristine
     * post-static-init state of all globals we context-switch. Used as
     * the baseline for any env's first swap_in. */
    if (!nle_baseline) {
        nle_baseline = (struct nle_dungeon_save *)
                       calloc(1, sizeof(struct nle_dungeon_save));
        if (nle_baseline)
            nle_dungeon_save_to(nle_baseline);
    }
    /* First swap_in for this env: copy from process-wide baseline.
     * Without this, env B inherits env A's wins[], BASE_WINDOW, etc.,
     * breaking NetHackRL ctor's BASE_WINDOW==0 invariant. */
    if (!nle->dungeon_save) {
        nle->dungeon_save = calloc(1, sizeof(struct nle_dungeon_save));
        if (nle_baseline && nle->dungeon_save)
            *(struct nle_dungeon_save *) nle->dungeon_save = *nle_baseline;
    }
    /* Nroom/nsubroom no longer require a per-step swap;
     * they're now per-env macros over the same ctx field that this swap
     * used to copy in/out of. Drops two cache-line bounces per step. */
    if (nle->dungeon_save)
        nle_dungeon_load_from((struct nle_dungeon_save *) nle->dungeon_save);
}

struct nle_dungeon_save *nle_baseline = NULL;

static void
nle_swap_out(nle_ctx_t *nle)
{
    /* Nroom/nsubroom writeback removed; both are now per-env
     * macros (per-env migration). The flags-memcpy that lived here is
     * also long gone — flags is per-env via macro.
     *
     * dungeon_save is captured here on first call (nle_start path) so
     * the env owns a saved level structure before any swap_in eviction. */
    if (!nle)
        return;
    if (!nle->dungeon_save) {
        nle->dungeon_save = calloc(1, sizeof(struct nle_dungeon_save));
        if (nle->dungeon_save)
            nle_dungeon_save_to((struct nle_dungeon_save *) nle->dungeon_save);
    }
}

nle_ctx_t *
nle_step(nle_ctx_t *nle, nle_obs *obs)
{
    nle_sentinel_beat(nle->sentinel, obs->action,
                      obs->blstats ? (int)obs->blstats[NLE_BL_DEPTH] : 0);
    /* exp_039: prefetch the env context aggressively. Under puffer's
     * round-robin OMP step pattern, each c_step touches a different env's
     * 72 KB nle_ctx_t cold from L2/L3 — that single-pattern alone is
     * empirically 10x slower than tight per-env step loops (multi_threaded
     * env-loop = 1.5M SPS vs round-robin = 155K SPS).
     * Hint the L1 prefetcher to start loading the struct head and a few
     * commonly-touched fields BEFORE the actual reads begin. Locality=3
     * (high temporal locality) to keep them around. Adjacent cache lines
     * (the first 4 lines of nle_ctx_t hold u_ptr, flags_ptr, iflags_ptr,
     * s7_level_p, nle_moves, nle_monstermoves — all hot every step). */
    __builtin_prefetch((const char *) nle +   0, 0, 3);
    __builtin_prefetch((const char *) nle +  64, 0, 3);
    __builtin_prefetch((const char *) nle + 128, 0, 3);
    __builtin_prefetch((const char *) nle + 192, 0, 3);
    current_nle_ctx = nle;
    nle_swap_in(nle);
    nle->observation = obs;
    if (nle->ttyrec) {
        write_ttyrec_header(1, 1);
        write_ttyrec_data(&obs->action, 1);
    }
    fcontext_transfer_t t = jump_fcontext(nle->generatorcontext, obs);
    nle->generatorcontext = t.ctx;
    nle->done = (t.data == NULL);
    obs->done = nle->done;
    nle_swap_out(nle);

    if (nle->ttyrec) {
        /* NLE ttyrec version 3 stores the action and in-game score in
         * different channels of the ttyrec. These channels are:
         *  - 0: the terminal instructions (classic ttyrec)
         *  - 1: the keypress/action (1 byte)
         *  - 2: the in-game score (4 bytes)
         *
         * We could either the note the in-game score every time we flush the
         * terminal instructions to screen, (eg writing [ 0 2 0 2 <step> 1 0 2
         * <step> 1 ]) or we can note it _just_ before resuming the game,
         * assuming no chicanery has happened to the score after it is written
         * to the array `blstats`, (eg writing [ 0 2 <step> 1 0 2 <step> 1 0 2
         * <step> ]). We chose the latter for compression & simplicity
         * reasons.
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

static void
free_nle_fields(nle_ctx_t *nle)
{
    /* All per-env heap buffers hanging off nle_ctx_t are now allocated via
     * nle_arena_calloc() / alloc() (the per-env arena), so that nle_fr_snapshot
     * captures their contents wholesale. Arena memory is reclaimed when the
     * arena is munmap'd in nle_end — calling libc free() on these (arena)
     * pointers would corrupt the heap. So there is nothing to free here; the
     * function is retained as the documented teardown hook. (This mirrors the
     * long-standing handling of s_artilist_p / s_qt_list_p, which were already
     * arena-allocated and intentionally never libc-freed.) */
    (void) nle;
}

void
nle_end(nle_ctx_t *nle)
{
    nle_sentinel_unregister(nle->sentinel);
    nle->sentinel = NULL;
    current_nle_ctx = nle;
    nle_swap_in(nle);
    if (!nle->done) {
        if (!current_nle_ctx->program_state.panicking) {
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

    destroy_fcontext_stack(&nle->stack);
    if (nle->s_arena_base) {
        extern void nle_arena_registry_release(char *);
        nle_arena_registry_release(nle->s_arena_base);
        munmap(nle->s_arena_base, nle->s_arena_cap);
        nle->s_arena_base = NULL;
        nle->s_arena_used = 0;
        nle->s_arena_cap  = 0;
    }
    extern void nle_winrl_destroy_for_ctx(nle_ctx_t *);
    nle_winrl_destroy_for_ctx(nle);
    free_nle_fields(nle);
    free(nle);
}

#ifdef NLE_ALLOW_SEEDING
void
nle_set_seed(nle_ctx_t *nle, unsigned long core, unsigned long disp,
             boolean reseed)
{
    /* Keep up to date with rnglist[] in rnd.c. */
    set_random(core, rn2);
    set_random(disp, rn2_on_display_rng);

    /* Determines logic in reseed_random() in hacklib.c. */
    current_nle_ctx->has_strong_rngseed = reseed;
};

/* nle_seeds[] moved into nle_ctx_t (refactor stage 2). Below uses
 * current_nle_ctx->seeds. */

void
nle_get_seed(nle_ctx_t *nle, unsigned long *core, unsigned long *disp,
             boolean *reseed)
{
    *core = current_nle_ctx->seeds[0];
    *disp = current_nle_ctx->seeds[1];
    *reseed = current_nle_ctx->has_strong_rngseed;
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

    current_nle_ctx = nle;
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

/* Release a blob returned by nle_save_level. */
void
nle_free_blob(void *blob)
{
    free(blob);
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

    current_nle_ctx = nle;
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
        level_info[ledger].linfo_flags |= LFILE_EXISTS;
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

    /* The rl mirror is not reset here (no callable C reset exists from this
     * translation unit); the next nle_step()'s full docrt() repaints every
     * tile and so clears any prior-level glyph residue. */

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
    current_nle_ctx = nle;
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
            /* mkgold(0,...) at hero pos makes a random pile; make it at an
             * offmap-ish spot then re-quan, then move into inventory. We
             * instead build the coin object directly via mkgold on the hero
             * tile and pull it in. */
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

    current_nle_ctx = nle;

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
    current_nle_ctx = nle;

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

    current_nle_ctx = nle;

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

    /* Keep experience points consistent with the new level: bump uexp up to
     * the threshold for this level if it is currently too low, so the level
     * does not immediately get clobbered by newexplevel(). Mirrors the
     * xp_level setter in nle_set_state. */
    if (u.uexp < newuexp(u.ulevel - 1))
        u.uexp = newuexp(u.ulevel - 1);

    context.botl = TRUE; /* bottom-line status is now stale */
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

/* From unixtty.c */
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
