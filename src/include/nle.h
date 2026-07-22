#ifndef NLE_H
#define NLE_H

#define NLE_BZ2_TTYRECS

#include <stdio.h>
#include <time.h>  /* time_t for stage 9' ubirthday */

#include <fcontext/fcontext.h>

#include "nleobs.h"
#include "isaac64.h"

/* ---------------------------------------------------------------------------
 * nle_tune_t — parametric difficulty knobs.
 *
 * The knob catalog is defined ONCE via the X-macro below; the struct, the
 * defaults, and the name table are all generated from it, and the binding
 * treats nle_tune_t as a flat array of doubles indexed by the name table.
 * Adding a knob = add one X() line here + read it at one engine decision
 * site; no get/set plumbing changes anywhere (binding or C).
 *
 * Every knob is a double so the generic get/set works uniformly: floats are
 * scales (1.0 = vanilla), bools are 0.0/1.0, ints are truncated. A scale knob
 * that should be "off"/"use vanilla" uses the documented sentinel in its
 * read-site (e.g. vision_radius <= 0 means "vanilla").
 *
 * X(name, default) — v1 = Layer 3 live (per-step) mechanics knobs.
 */
#define NLE_TUNE_FIELDS(X)                 \
    X(dmg_to_player_scale,      1.0)       \
    X(dmg_by_player_scale,      1.0)       \
    X(player_hp_scale,          1.0)       \
    X(hp_regen_scale,           1.0)       \
    X(vision_radius,            0.0)       \
    X(reveal_map,               0.0)       \
    X(hunger_rate_scale,        1.0)       \
    X(ongoing_spawn_scale,      1.0)       \
    X(monster_difficulty_scale, 1.0)       \
    X(monster_speed_scale,      1.0)       \
    X(xp_gain_scale,            1.0)       \
    X(room_density,             1.0)       \
    X(mob_spawn,                1.0)       \
    X(trap_density,             1.0)       \
    X(locked_door,              1.0)       \
    X(corridor_connectivity,    1.0)       \
    X(room_size,                1.0)

typedef struct nle_tune {
#define NLE_TUNE_DECL(name, dflt) double name;
    NLE_TUNE_FIELDS(NLE_TUNE_DECL)
#undef NLE_TUNE_DECL
} nle_tune_t;

/* Convenience accessor for engine read-sites: `nle_tuning.<knob>`. Resolves to
 * the current env's knob block (current_nle_ctx is anchored on every step). */
#define nle_tuning (current_nle_ctx->s_tune)

/* TODO: Fix this. */
#undef SIG_RET_TYPE
#define SIG_RET_TYPE void (*)(int)

typedef struct TMT TMT;

/* Forward declarations for migrated NetHack structs (stage 4 — player).
 * Real definitions in include/you.h, which is too heavy to pull into nle.h
 * (would force monst.h, prop.h, skills.h cascade into util binaries).
 * Use heap pointers in nle_ctx_t and let nle.c (which includes hack.h)
 * allocate them. */
struct you;
struct flag;             /* include/flag.h */
struct instance_flags;   /* include/flag.h */
struct sysflag;          /* include/flag.h, only #ifdef SYSFLAGS */
struct WinDesc;          /* include/wintty.h (stage 10') */
struct DisplayDesc;      /* include/wintty.h (stage 10') */
struct obj;              /* include/obj.h (stage 9') */
struct monst;            /* include/monst.h (stage 9') */
struct autopickup_exception; /* include/decl.h (stage 9') */
struct dgn_topology;     /* include/decl.h (stage 6') */
struct dungeon;          /* include/dungeon.h (stage 6') */
struct s_level;          /* include/dungeon.h (stage 6') */
struct stairway;         /* include/dungeon.h (stage 6') */
struct dest_area;        /* include/dungeon.h (stage 6') */
struct tmp_glyph;        /* display.c file-local */
struct toptenentry;      /* topten.c file-local */
struct nhcoord;          /* include/coord.h (stage 6'); typedef'd as `coord` */
struct multishot;        /* include/decl.h (stage 9' batch C) */
struct u_realtime;       /* include/you.h (stage 9' batch C) */
struct q_score;          /* include/quest.h (stage 9' batch C) */
struct spell;            /* include/spell.h (stage 9' batch C) */
struct nle_mvitals_t;    /* include/decl.h (stage 9' batch C) */
struct kinfo;            /* include/decl.h (stage 9' batch C) */
struct nle_tcap_t;       /* include/decl.h (stage 8' completion) */
struct mkroom;           /* include/mkroom.h (stage 7' partial) */
struct linfo;            /* include/dungeon.h (stage 7' partial) */
struct trap;             /* include/trap.h   (stage 7' partial) */
struct nle_dlevel;       /* include/rm.h     (stage 7' completion) */
struct cmd;              /* include/flag.h   (Cmd migration) */
struct context_info;     /* include/context.h (context migration) */
struct nle_rndmonst_state; /* makemon.c (rndmonst_state migration) */
struct artifact;         /* include/artifact.h (artilist migration) */
struct objclass;         /* include/objclass.h (objects migration) */
struct objdescr;         /* include/objclass.h (obj_descr migration) */
struct fruit;            /* include/youprop.h via hack.h (restore.c oldfruit) */
struct qtlists;          /* include/qtext.h (questpgr.c qt_list) */
struct dlb_handle;       /* include/dlb.h (questpgr.c msg_file; dlb is typedef'd to this) */
struct engr;             /* include/engrave.h (engrave.c head_engr) */
struct litmon;           /* defined locally in src/read.c (read.c gremlins) */
struct nle_lev_region_s; /* mkmaze.c bughack; tag added in include/sp_lev.h */
union any;               /* include/wintype.h (`typedef union any anything;`) (hack.c tmp_anything) */
struct opvar;            /* include/sp_lev.h (do_name.c gloc_filter_map) */

/* Misc-2 forward decls. Pointers in nle_ctx_t, real
 * struct definitions remain local to their .c files (extralev.c, dbridge.c). */
struct rogueroom;        /* src/extralev.c */
struct entity;           /* src/dbridge.c */
struct breadcrumbs;      /* include/decl.h (ball.c bc[pu]breadcrumbs) */

/* `struct sinfo` was defined inline at the variable declaration in
 * decl.h. Moved here for the refactor (stage 3b) so nle_ctx_t can host
 * a per-instance copy. The original macro storm in decl.h is replaced
 * by direct current_nle_ctx->program_state.X access at each callsite. */
struct sinfo {
    int gameover;  /* self explanatory? */
    int stopprint; /* inhibit further end of game disclosure */
#ifdef HANGUPHANDLING
    volatile int done_hup; /* SIGHUP or moral equivalent received
                            * -- no more screen output */
    int preserve_locks;    /* don't remove level files prior to exit */
#endif
    int something_worth_saving; /* in case of panic */
    int panicking;              /* `panic' is in progress */
    int exiting;                /* an exit handler is executing */
    int in_moveloop;
    int in_impossible;
#ifdef PANICLOG
    int in_paniclog;
#endif
    int wizkit_wishing;
};

typedef struct nle_globals {
    fcontext_stack_t stack;
    fcontext_t returncontext;
    fcontext_t generatorcontext;

    FILE *ttyrec;
    TMT *vterminal;
    char outbuf[BUFSIZ];
    char *outbuf_write_ptr;
    char *outbuf_write_end;

#ifdef NLE_BZ2_TTYRECS
    void *ttyrec_bz2;
#endif

    boolean done;
    nle_obs *observation;

    void *sentinel; /* opaque nle_sentinel slot for this env (may be NULL) */

    /* nle_state refactor — RNG subsystem (stage 1, was static rnglist
     * in rnd.c). Accessed via nle_rng_state(idx) / nle_rng_init_flag(idx). */
    isaac64_ctx rng_state[2];
    int rng_init[2]; /* boolean flag per rng; 0 = uninitialized */

    /* nle_state refactor — NLE wrapper layer (stage 2). Moved out of
     * file-scope statics in nle.c (`settings`, `nle_seeds_init`) and
     * hacklib.c (`nle_seeds`). seeds[] tracks the last seed set via
     * set_random() for inspection via nle_get_seed(). */
    nle_settings        settings;
    nle_seeds_init_t   *seeds_init;
    unsigned long       seeds[2];
    boolean             has_strong_rngseed; /* was NEARDATA in decl.c */
    struct sinfo        program_state;      /* was NEARDATA in decl.c */
    int                 hackpid;            /* was 'int hackpid' in decl.c */
    boolean             restoring;          /* was decl.c flag (savefile) */
    boolean             ransacked;          /* was decl.c flag (mkmaze) */
    boolean             in_steed_dismounting; /* was decl.c flag (steed) */
    const char         *multi_reason;       /* was decl.c ('Speed', 'Slowness', ...) */
    int                 occtime;            /* was decl.c (occupation duration) */
    /* Per-env moveloop function pointers + occupation text (decl.c).
     * Were process-globals — fatal under OMP T>1 (fn-ptr clobber → jump
     * to garbage). `afternmv` is set/read ~41 sites, `occupation` ~14. */
    int               (*afternmv_v)(void);
    int               (*occupation_v)(void);
    const char         *occtxt_v;
    const char         *nomovemsg_v;
    char               *configfile_v;
    /* Per-env maze/map bounds (decl.c / mkmaze.c / mkmap.c / sp_lev.c).
     * Written during level generation — fatal race under OMP T>1. */
    int                 x_maze_max_v;
    int                 y_maze_max_v;
    int                 min_rx_v;
    int                 max_rx_v;
    int                 min_ry_v;
    int                 max_ry_v;
    /* Per-env steal state (steal.c / uhitm.c). */
    unsigned            stealmid_v;
    unsigned            stealoid_v;
    /* Per-env level-transition scratch (do.c). */
    /* save_dlevel: d_level is {schar dnum, schar dlevel}. Store as two schars. */
    signed char         save_dlevel_dnum;
    signed char         save_dlevel_dlevel;
    /* Per-env pet preference (options.c / role.c). */
    char                preferred_pet_v;
    /* Per-env tty wait state (getline.c). */
    int                 xwaitingforspace_v;
    /* Per-env timed occupation callback (cmd.c timed_occ_fn).
     * Function pointer — fatal race under OMP T>1. */
    int               (*timed_occ_fn_v)(void);
    /* Per-env lock-picking state (lock.c xlock struct).
     * Contains door/box pointers — cross-env deref under T>1. */
    /* Matches layout of struct xlock_s in lock.c exactly. */
    void               *xlock_door_v;   /* struct rm * */
    void               *xlock_box_v;    /* struct obj * */
    int                 xlock_picktyp_v;
    int                 xlock_chance_v;
    int                 xlock_usedtime_v;
    unsigned char       xlock_magic_key_v; /* boolean = uchar */
    unsigned char       xlock_pad_[3];
    /* stage 3f — level-building + input replay state */
    /* Renamed from `nroom`/`nsubroom` so the per-env macros in
     * decl.h (`#define nroom (current_nle_ctx->s_nroom)`) can route bare
     * references without expanding inside `current_nle_ctx->nroom`. */
    int                 s_nroom;            /* was decl.c (rooms on current level) */
    int                 s_nsubroom;         /* was decl.c (subrooms in shop/temple) */
    int                 doorindex_v;        /* macro: doorindex */
    boolean             in_mklev_v;         /* macro: in_mklev */
    int                 in_doagain_v;       /* macro: in_doagain */
    /* stage 3g — combat / inventory transient flags */
    boolean             stoned;             /* was decl.c (monster being stoned) */
    boolean             unweapon;           /* was decl.c (player unwielded) */
    boolean             mrg_to_wielded;     /* was decl.c (merge picked-up to wield) */
    boolean             defer_see_monsters; /* was decl.c (suppress see_monsters refresh) */
    /* stage 3h — small misc globals */
    schar               tbx;                /* was decl.c (throw target x) */
    schar               tby;                /* was decl.c (throw target y) */
    int                 otg_temp;           /* was decl.c (object_to_glyph scratch) */
    long                yn_number;          /* was decl.c (last numeric y/n response) */
    /* stage 3i — trivial-scope globals */
    int                 locknum;            /* was decl.c (UNIX simultaneous-user count) */
    long                done_money;         /* was decl.c (cash at death) */
    int                 warn_obj_cnt;       /* was decl.c (warn-mon counter) */
    /* bhitpos — per-env throw/zap impact point. Stored as a pointer so
     * we can keep coord.h out of nle.h. Allocated in init_nle. */
    struct nhcoord      *bhitpos_p;
    /* Per-env player identity buffers. PL_NSIZ=32, PL_CSIZ=20,
     * PL_FSIZ=32. Allocated inline (small enough). */
    char                 plname_v[32];        /* PL_NSIZ */
    char                 pl_character_v[32];  /* PL_CSIZ */
    char                 pl_race_v;
    char                 pl_fruit_v[32];      /* PL_FSIZ */
    char                 tune_v[6];
    /* Pet name buffers and a couple of pointers. */
    char                 dogname_v[63];       /* PL_PSIZ */
    char                 catname_v[63];
    char                 horsename_v[63];
    struct fruit        *ffruit_v;
    char                *save_cm_v;
    /* stage 3j — turn loop state (big migration, ~40 callsites) */
    int                 multi;              /* was decl.c (multi-step action counter) */
    /* stage 4 — player state (the big one — ~94 files, hundreds of refs) */
    struct you         *u_ptr;              /* was 'struct you u' in decl.c */
    /* stage 5 — game options & flags (another macro-storm: wizard,
     * discover, use_color etc. expand to flags.X / iflags.X) */
    struct flag                  *flags_ptr;  /* was 'struct flag flags' */
    struct instance_flags        *iflags_ptr; /* was 'struct instance_flags iflags' */
    struct sysflag               *sysflags_ptr; /* was 'struct sysflag sysflags' */
    /* stage 6 — dungeon topology (multiple structs/arrays, context-switched
     * around each nle_step). The exact layout is internal to nle.c which has
     * the full type definitions. Opaque blob here. */
    void                         *dungeon_save;
    /* stage 10' — TTY window port state (was wintty.c/getline.c globals).
     * Field names are distinct from the public macros (see wintty.h) so
     * that those macros don't accidentally expand inside this struct
     * declaration — hack.h includes wintty.h before any TU includes nle.h.
     * Same trick as `u_ptr` field vs `u` macro (stage 4). */
    int                  base_window;     /* macro: BASE_WINDOW */
    struct WinDesc      *tty_wins[20];    /* macro: wins; MAXWIN=20 */
    struct DisplayDesc  *tty_display;     /* macro: ttyDisplay */
    char                 tty_morc;        /* macro: morc */
    /* stage 8' — display / message state (was decl.c globals).
     * Field names distinct from macros for the same hack.h-pulls-decl.h
     * -before-nle.h reason as stage 10'. tc_gbl_data deferred (struct-tag
     * self-reference; addressed in the collision batch). */
    int                  win_message;     /* macro: WIN_MESSAGE */
    int                  win_status;      /* macro: WIN_STATUS */
    int                  win_map;         /* macro: WIN_MAP */
    int                  win_inven;       /* macro: WIN_INVEN */
    char                 vision_recalc;   /* macro: vision_full_recalc */
    char               **vision_array;    /* macro: viz_array */
    char                 top_lines[300];  /* macro: toplines; TBUFSZ=300 */
    /* stage 9' batch A — scalars (no struct cascade). */
    time_t               nle_ubirthday;       /* macro: ubirthday */
    long                 nle_moves;           /* macro: moves */
    long                 nle_monstermoves;    /* macro: monstermoves */
    long                 nle_wailmsg;         /* macro: wailmsg */
    long                 nle_domove_attempting; /* macro: domove_attempting */
    long                 nle_domove_succeeded;  /* macro: domove_succeeded */
    /* stage 9' batch B — already-pointers; just relocate. */
    struct obj          *invent_p;            /* macro: invent */
    struct obj          *uskin_p;             /* macro: uskin */
    struct obj          *current_wand_p;      /* macro: current_wand */
    struct obj          *thrownobj_p;         /* macro: thrownobj */
    struct obj          *kickedobj_p;         /* macro: kickedobj */
    struct obj          *migrating_objs_p;    /* macro: migrating_objs */
    struct obj          *billobjs_p;          /* macro: billobjs */
    struct monst        *mydogs_p;            /* macro: mydogs */
    struct monst        *migrating_mons_p;    /* macro: migrating_mons */
    struct autopickup_exception *apelist_p;   /* macro: apelist */
    /* stage 6' — dungeon topology (heap-allocated to avoid dungeon.h
     * cascade in nle.h). All allocated in init_nle. */
    struct dgn_topology *s6_topology_p;       /* macro: dungeon_topology */
    struct dungeon      *s6_dungeons_p;       /* macro: dungeons (array head) */
    struct s_level      *s6_sp_levchn;        /* macro: sp_levchn */
    struct stairway     *s6_upstair_p;        /* macro: upstair */
    struct stairway     *s6_dnstair_p;        /* macro: dnstair */
    struct stairway     *s6_upladder_p;       /* macro: upladder */
    struct stairway     *s6_dnladder_p;       /* macro: dnladder */
    struct stairway     *s6_sstairs_p;        /* macro: sstairs */
    struct dest_area    *s6_updest_p;         /* macro: updest */
    struct dest_area    *s6_dndest_p;         /* macro: dndest */
    struct nhcoord      *s6_inv_pos_p;        /* macro: inv_pos */
    /* stage 9' batch C — heap-allocated per-env. Each ctor calloc's in
     * init_nle; nle_end frees. Replaces the dungeon_save round-trip. */
    struct multishot    *s9c_m_shot_p;        /* macro: m_shot */
    struct u_realtime   *s9c_urealtime_p;     /* macro: urealtime */
    struct q_score      *s9c_quest_status_p;  /* macro: quest_status */
    struct spell        *s9c_spl_book_p;      /* macro: spl_book (array head) */
    struct monst        *s9c_youmonst_p;      /* macro: youmonst */
    struct nle_mvitals_t *s9c_mvitals_p;      /* macro: mvitals (array head) */
    struct kinfo        *s9c_killer_p;        /* macro: killer */
    struct nle_tcap_t   *s8_tcap_p;           /* macro: tc_gbl_data */
    /* stage 7' partial — easy items from the current-level swap bundle
     * that don't have the `level` token-collision problem. All heap-
     * allocated in init_nle. */
    struct mkroom       *s7_rooms_p;          /* macro: rooms; size (MAXNROFROOMS+1)*2 */
    struct nhcoord      *s7_doors_p;          /* macro: doors; size DOORMAX */
    struct linfo        *s7_level_info_p;     /* macro: level_info; size MAXLINFO */
    schar               *s7_lastseentyp_p;    /* macro: lastseentyp; size COLNO*ROWNO */
    struct mkroom       *s7_subrooms;         /* macro: subrooms (just a pointer) */
    struct mkroom       *s7_upstairs_room;    /* macro: upstairs_room */
    struct mkroom       *s7_dnstairs_room;    /* macro: dnstairs_room */
    struct mkroom       *s7_sstairs_room;     /* macro: sstairs_room */
    struct trap         *s7_ftrap;            /* macro: ftrap */
    /* stage 7' completion — `dlevel_t level` (40 KB, the largest single
     * global) heap-allocated per-env. The macro pattern works now that
     * `struct dig_info.level` was renamed to `.dlvl` in context.h.
     * Forward-declared `struct nle_dlevel` (tag added in rm.h). */
    struct nle_dlevel   *s7_level_p;          /* macro: level */
    /* Cmd — command bindings (struct cmd in flag.h). */
    struct cmd          *s5_cmd_p;            /* macro: Cmd */
    /* small file-local-static migrations (each defined as a macro
     * inside the .c file that owns the global). */
    short               *s_disco_p;           /* o_init.c disco[NUM_OBJECTS] */
    char                *s_obufs_p;           /* objnam.c obufs[NUMOBUF][BUFSZ] */
    char                 s_prevmsg[256];      /* pline.c prevmsg[BUFSZ=256] */
    void                *s_tty_status_p;      /* wintty.c tty_status[2][MAXBLSTATS] */
    unsigned long       *s_tty_colormasks;    /* wintty.c tty_colormasks */
    long                 s_tty_condition_bits; /* wintty.c tty_condition_bits */
    int                  s_hpbar_percent;     /* wintty.c hpbar_percent */
    int                  s_hpbar_color;       /* wintty.c hpbar_color */
    struct context_info *s_context_p;         /* macro: context */
    struct nle_rndmonst_state *s_rndmonst_state_p; /* makemon.c rndmonst_state */
    struct artifact     *s_artilist_p;        /* artifact.c artilist[] */
    struct objclass     *s9o_objects_p;       /* macro: objects (NUM_OBJECTS entries) */
    struct objdescr     *s9o_obj_descr_p;     /* macro: obj_descr (NUM_OBJECTS entries) */
    /* per-env status-line state (windows.c / wintty.c). MAXBLSTATS=23. */
    const char          *s_status_fieldnm[23];
    const char          *s_status_fieldfmt[23];
    char                *s_status_vals[23];
    boolean              s_status_activefields[23];
    /* per-env "name buffers" pool used by do_name.c nextmbuf().
     * NUMMBUF=5, BUFSZ=256 → 1280 bytes flat. */
    char                *s_mbufs_p;
    int                  s_mbuf_idx;
    /* per-env room-equivalence work array (decl.c smeq[]). */
    int                 *s_smeq_p;            /* size MAXNROFROOMS+1 */
    /* per-env object-class base-index table (decl.c bases[MAXOCLASSES]). */
    int                 *s_bases_p;
    /* per-env terminal color escapes (tty/termcap.c hilites[CLR_MAX]). */
    char               **s_hilites_p;
    /* display buffer (display.c gbuf_entry[ROWNO][COLNO]) + bookkeeping. */
    void                *s_gbuf_p;            /* malloc'd ROWNO*COLNO*sizeof(gbuf_entry) */
    char                 s_gbuf_start[21];    /* ROWNO=21 */
    char                 s_gbuf_stop[21];
    /* bottom-line stats (botl.c blstats[2][MAXBLSTATS]) + flags. */
    void                *s_blstats_p;
    boolean              s_blinit;
    boolean              s_update_all;
    /* Per-env once-per-game init flags (formerly file-scope
     * static booleans that tripped in shared-libnethack vecenv when env 2
     * inherited env 1's TRUE state). */
    char                 s_blstats_initalready; /* botl.c init_blstats */
    /* Vision.c transient computation state. These are set at
     * the top of view_from() and used by left_side/right_side recursively.
     * If a vecenv env yields mid-vision_recalc, another env will clobber
     * the statics, breaking the recursion → infinite loop. Moved per-env.
     * (genericptr_t typed as void* via opaque cast.) */
    int                  s_vis_start_row;
    int                  s_vis_start_col;
    int                  s_vis_step;
    char               **s_vis_cs_rows;
    char                *s_vis_cs_left;
    char                *s_vis_cs_right;
    void               (*s_vis_func)();
    void                *s_vis_varg;
    /* Timeout.c timer queue — was __thread, broken under vecenv
     * because all envs share one thread; env A's timers fire while env B
     * holds the globals → "extract_nexthere: object lost" panic. */
    void                *s_timer_base;            /* timer_element * */
    unsigned long        s_timer_id;
    /* Light source list head (light.c light_base). Was
     * __thread; under vecenv env A's lights leaked into env B's
     * vision_recalc → impossible objects on the wrong levels →
     * eventual cascade in left_ptrs causing infinite recursion. */
    void                *s_light_base;            /* light_source * */
    /* Deferred-goto messages (do.c). Were __thread;
     * env A schedules level change with messages, env B's deferred_goto
     * sees A's leftover strings (now potentially dangling). */
    char                *s_dfr_pre_msg;
    char                *s_dfr_post_msg;
    /* Region.c per-env region table (gas clouds, force-fields).
     * Was process-global (regions) + __thread (n/max). Cross-env contamination
     * was severe — env A's gas cloud could be applied to env B's monsters. */
    void                *s_regions;       /* NhRegion ** */
    int                  s_n_regions;
    int                  s_max_regions;
    /* Assorted small __thread to per-env. */
    unsigned             s_pline_flags;
    int                  s_polearm_range_min;
    int                  s_polearm_range_max;
    int                  s_lastinvnr;       /* invent.c menu nrf */
    int                  s_bcrestriction;   /* ball/chain */
    int                  s_mkot_trap_warn_count;
    /* Function-local static recursion guards (pline.c, hack.c). */
    int                  s_pline_in_pline;
    int                  s_inspoteffects;
    int                  s_artifact_nesting;
    /* Vision recursion depth guard. */
    int                  s_vision_recur_depth;
    /* Vision.c viz_rmin/viz_rmax. Set during vision_recalc;
     * if env A yields mid-recalc, env B overwrites these. (viz_array
     * itself already moved to nle_ctx_t->vision_array in stage 8'.) */
    char                *s_viz_rmin;
    char                *s_viz_rmax;
    boolean              s_valset[23];        /* MAXBLSTATS */
    void                *s_status_hilites_p;
    /* vision work buffers (vision.c). */
    void                *s_could_see_p;
    void                *s_viz_clear_p;
    void                *s_left_ptrs_p;
    void                *s_right_ptrs_p;
    char                *s_cs_rows0[21];
    char                *s_cs_rows1[21];
    char                 s_cs_rmin0[21];
    char                 s_cs_rmax0[21];
    char                 s_cs_rmin1[21];
    char                 s_cs_rmax1[21];
    char                *s_viz_clear_rows[21];
    /* special-level position map (sp_lev.c). */
    void                *s_SpLev_Map_p;
    /* file-path scratch buffers (files.c fqn_filename_buffer[FQN_NUMBUF][FQN_MAX_FILENAME]). */
    void                *s_fqn_fname_p;       /* 2048 bytes */
    /* per-env worm tables (worm.c). MAX_NUM_WORMS=32. */
    void                *s_wheads_p;
    void                *s_wtails_p;
    void                *s_wgrowtime_p;
    /* options.c boolopt[] — per-env copy of the boolean-options
     * table. Baseline is const in options.c; init_nle calloc's a
     * mutable slot, options.c seeds it from baseline at game init. */
    void                *s_boolopt_p;
    void                *s_compopt_p;
    /* per-env role/race description (role.c urole/urace). */
    void                *s_urole_p;
    void                *s_urace_p;
    /* Per-env NetHackRL singleton (winrl.cc).
     * Was `static thread_local std::unique_ptr<NetHackRL> instance`. Under
     * PufferLib's OMP-parallel cpu_vec_step, worker threads have a null
     * thread_local instance and segfault in rl_nhgetch. Owned by this
     * pointer; nle_end deletes it via NetHackRL::destroy_for_ctx(). */
    void                *s_netHackRL_instance;
    /* Per-env win-procedure trace deque (winrl.cc).
     * Was `thread_local std::deque<std::string> win_proc_calls`. Same OMP
     * coroutine-resume hazard as s_netHackRL_instance: push happens on
     * init thread, pop on worker thread → empty-deque pop_back UB. Owned
     * by this pointer; nle_end frees it via NetHackRL::destroy_for_ctx(). */
    void                *s_win_proc_calls;
    /* Per-env prompt-state booleans (winrl.cc).
     * Were file-scope globals — unsafe under OMP T>1. */
    int                  s_in_yn_function;
    int                  s_in_getlin;
    /* Per-env artifact combat coordination (artifact.c).
     * Was STATIC_OVL file-scope int — unsafe under OMP T>1. */
    int                  s_spec_dbon_applies;
    /* Per-env graphics mode (drawing.c).
     * Was file-scope int — unsafe under OMP T>1. */
    int                  s_currentgraphics;
    /* Per-env tty backend state (win/tty/*.c).
     * Each file owns its own struct; void* here so nle.h doesn't have
     * to pull in MAX_PER_ROW, BUFSIZ, enum statusfields. Owned by the
     * respective .c file's accessor; nle_end frees via nle_tty_destroy_for_ctx().
     *
     * wintty.c:  obuf / clipping / clipx,y / vt_tile_current_window /
     *            fieldorder / finalx / windowdata_init / cond_shrinklvl /
     *            enclev,enc_shrinklvl / dlvl_shrinklvl / truncation_expected /
     *            do_field_opt
     * topl.c:    snapshot_mesgs
     * termcap.c: KS, KE
     *
     * Was `static __thread X foo`. Cross-thread coroutine resume saw the
     * worker pthread's TLS slots empty after the env was init'd on the
     * main thread, triggering jump_fcontext+103 SIGSEGV. Now per-env so
     * resume on any thread reads the same env state. */
    void                *s_wintty_state;
    void                *s_topl_state;
    void                *s_termcap_state;
    /* Per-env src-file local state. Each `void*` is owned by
     * the corresponding .c file; lazy-alloced through a file-local
     * accessor that resolves via current_nle_ctx. Same pattern as the
     * tty group above. Frees in nle_end. */
    void                *s_pline_state;       /* pline.c: you_buf / you_buf_siz */
    void                *s_save_state;        /* save.c: bw_fd / buffering */
    void                *s_files_state;       /* files.c: nesting / lockfd / config_error_data / symset_* */
    void                *s_objnam_state;      /* objnam.c: obufidx / distantname */
    void                *s_uhitm_state;       /* uhitm.c: override_confirmation */
    void                *s_shk_state;         /* shk.c: auto_credit */
    void                *s_end_state;         /* end.c: Schroedingers_cat */
    void                *s_sounds_state;      /* sounds.c: soundmap */
    void                *s_fast_reset_state;  /* nle_fast_reset.c: nle_arena_base */
    void                *s_botl_state;        /* botl.c */
    void                *s_cmd_state;         /* cmd.c */
    void                *s_do_name_state;     /* do_name.c */
    void                *s_dokick_state;      /* dokick.c */
    void                *s_do_wear_state;     /* do_wear.c */
    void                *s_apply_state;       /* apply.c */
    void                *s_rumors_state;      /* rumors.c */
    void                *s_options_state;     /* options.c */
    void                *s_rip_state;         /* rip.c */
    void                *s_display_state;     /* display.c */
    void                *s_windows_state;     /* windows.c */
    void                *s_sp_lev_state;      /* sp_lev.c */
    void                *s_pager_state;       /* pager.c */
    /* Per-env bump arena. Replaces the process-wide arena +
     * __sync_fetch_and_add in alloc.c. Each env's coroutine is the sole
     * writer of its own arena, so bumps are race-free without atomics.
     * Lazily mmap'd on first alloc() with current_nle_ctx set; munmap'd
     * in nle_end. Legacy file-scope globals in alloc.c remain as a
     * fallback for the rare allocation made before current_nle_ctx is
     * anchored (very early process init / util binaries). */
    char                *s_arena_base;
    size_t               s_arena_used;
    size_t               s_arena_cap;
    /* Stage 9' batch D — body-slot pointers.  Were TLS NEARDATA in decl.c,
     * pinned there by worn[] referencing &uarm etc. at static-init time.
     * Now live per-env on nle_ctx_t; worn[] uses byte-offset resolution.
     * Prefixed s9_ to match the batch naming used for other stage-9' work. */
    struct obj          *s9_uarm;
    struct obj          *s9_uarmc;
    struct obj          *s9_uarmh;
    struct obj          *s9_uarms;
    struct obj          *s9_uarmg;
    struct obj          *s9_uarmf;
    struct obj          *s9_uarmu;
    struct obj          *s9_uleft;
    struct obj          *s9_uright;
    struct obj          *s9_uwep;
    struct obj          *s9_uswapwep;
    struct obj          *s9_uquiver;
    struct obj          *s9_uamul;
    struct obj          *s9_ublindf;
    struct obj          *s9_uball;
    struct obj          *s9_uchain;
    /* Botl.c per-env status state.
     * cond_hilites[] was a plain static (process-global) unsigned long array;
     * it holds condition highlight masks computed per-env during render_status.
     * bl_hilite_moves and now_or_before_idx were __thread; broken under OMP
     * vecenv for the same coroutine-resume reason as the tty backend group.
     * status_hilite_str / status_hilite_str_id were __thread linked-list
     * head+id; thread-local values are zero on worker threads after env was
     * init'd on main thread, so the list is lost and allocs leak. */
    unsigned long        s_cond_hilites[21]; /* BL_ATTCLR_MAX = CLR_MAX(16)+5 */
    long                 s_bl_hilite_moves;  /* botl.c bl_hilite_moves */
    int                  s_now_or_before_idx; /* botl.c now_or_before_idx */
    void                *s_status_hilite_str_p; /* botl.c status_hilite_str */
    int                  s_status_hilite_str_id; /* botl.c status_hilite_str_id */
    /* Cmd.c per-env key-input queues.
     * pushq/saveq/phead/ptail/shead/stail were plain statics (process-global);
     * concurrent OMP envs sharing one thread could interleave reads/writes
     * from different envs' input replay sequences. */
    char                 s_pushq[20];       /* cmd.c pushq[BSIZE], BSIZE=20 */
    char                 s_saveq[20];       /* cmd.c saveq[BSIZE] */
    int                  s_phead;           /* cmd.c phead */
    int                  s_ptail;           /* cmd.c ptail */
    int                  s_shead;           /* cmd.c shead */
    int                  s_stail;           /* cmd.c stail */
    /* Wintty.c/getline.c per-env scratch.
     * compress_str() cbuf was a function-local static used by tty_putstr
     * on every message output — a hot per-env buffer shared across envs.
     * tty_nhgetch nesting was __thread; marks re-entrant getc under UNIX.
     * suppress_history in getline.c was a plain STATIC_VAR (process-global). */
    char                 s_compress_cbuf[256]; /* wintty.c compress_str cbuf, BUFSZ=256 */
    int                  s_tty_nhgetch_nesting; /* wintty.c tty_nhgetch nesting */
    boolean              s_suppress_history; /* getline.c suppress_history */
    /* Cmd.c enlightenment-window state.
     * en_win was a plain static (process-global winid); concurrent envs
     * both running enlightenment (e.g. at game-over) would race on it.
     * en_via_menu was __thread; OMP cross-thread resume hazard. */
    short                s_en_win;           /* cmd.c en_win (winid=short) */
    boolean              s_en_via_menu;      /* cmd.c en_via_menu */
    /* Remaining functional __thread variables.
     * Each was __thread (broken under OMP coroutine-resume) or a plain
     * process-global static (racy under concurrent envs). */
    /* rumors.c oracle state — __thread (flg/loc) or plain static (cnt).
     * oracle_cnt is decremented as oracles are used, so it must be per-env. */
    int                  s_oracle_flg;       /* rumors.c oracle_flg */
    unsigned long       *s_oracle_loc;       /* rumors.c oracle_loc (heap ptr) */
    unsigned             s_oracle_cnt;       /* rumors.c oracle_cnt */
    /* do_wear.c initial_don — __thread; per-env flag for startup auto-wear */
    boolean              s_initial_don;
    /* sp_lev.c special-level generation state — __thread; each env's level
     * gen is independent. container_obj[] is already a plain static (not TLS),
     * so container_idx (index into it) must be per-env to avoid aliasing. */
    boolean              s_splev_init_present;
    boolean              s_icedpools;
    int                  s_container_idx;
    /* spell.c sort state — __thread; sort mode and index array per-env. */
    int                  s_spl_sortmode;
    int                 *s_spl_orderindx;    /* heap ptr, NULL=not alloced */
    /* restore.c ID-mapping state — __thread; used during savefile restore. */
    int                  s_n_ids_mapped;
    void                *s_id_map;           /* struct bucket *, heap */
    /* eat.c eatmbuf — __thread; allocated string for mimic-eating feedback. */
    char                *s_eatmbuf;          /* heap ptr, NULL=none */
    /* options.c n_menu_mapped — __thread; count of mapped menu cmds per env. */
    short                s_n_menu_mapped;
    /* windows.c last_winchoice — __thread; window-system choice during init;
     * used only at startup, but must be per-env if envs init concurrently. */
    void                *s_last_winchoice;   /* struct win_choices * */
    /* Makemon.c align_shift() per-env cache.
     * oldmoves and lev were `static NEARDATA` (plain process-global) inside
     * align_shift(). Two OMP threads in makemon() simultaneously race on
     * the oldmoves/lev update, corrupting lev and causing a SIGSEGV when
     * one thread dereferences the other env's stale s_level pointer.
     * NOTE: `s_level` is not declared in nle.h; use void* + cast in .c. */
    long                 s_align_shift_oldmoves; /* makemon.c align_shift oldmoves */
    void                *s_align_shift_lev;      /* makemon.c align_shift lev (s_level*) */

    /* Per-env dungeon graph + level-builder + key-cmd state.
     * Was: process-global mutable in dungeon.c/mklev.c/do.c/decl.c. With
     * N envs in one process, env A's dungeon graph would be walked by env
     * B's level transition code, causing the save_room(r=NULL) crash. */
    void                         *s_branches;          /* branch * (dungeon.c) */
    int                           s_branch_id_ctr;     /* dungeon.c add_branch */
    void                         *s_mapseenchn;        /* mapseen * (dungeon.c) */
    int                           s_n_dgns;            /* dungeon.c */
    signed char                   s_vault_x;           /* xchar (mklev.c) */
    signed char                   s_vault_y;           /* xchar (mklev.c) */
    char                          s_made_branch;       /* boolean (mklev.c) */
    char                          s_at_ladder;         /* boolean (do.c) */
    void                         *s_save_cm;           /* struct ext_func_tab * (decl.c) */
    /* Per-env level/save filename buffers and prefix table.
     * Was: process-global `char lock[PL_NSIZ+14]`, `char SAVEF[SAVESIZE]`,
     * `char bones[]`, `char *fqn_prefix[PREFIX_COUNT]` in files.c/decl.c.
     * With N envs in one process, all envs collided on the same buffer:
     * getlock() builds lock="<uid><plname>" (same for every env's wizard
     * default), and fqn_prefix[] was set once per env at mainloop entry
     * then overwritten by the next env. Result: env j writes its level
     * file over env i's; env i later mreads a torn level and panics.
     * Sizes match compile-time arrays in files.c (PL_NSIZ=32). */
    char                          s_lock[46];      /* PL_NSIZ+14 */
    char                          s_SAVEF[45];     /* SAVESIZE = PL_NSIZ+13 (UNIX) */
    char                          s_bones[16];    /* "bonesnn.xxx" + slack */
    char                         *s_fqn_prefix[10]; /* PREFIX_COUNT */

    /* Save/restore session state (save.c + restore.c).
     * Eleven file-statics that race across envs during c_reset save paths.
     * Direct fields on nle_ctx_t (no swap struct); macros at the top of
     * each .c file rewrite accesses to current_nle_ctx->s_<name>.
     *
     * Note: s_outbuf is sized 256 here because nle.h is included by util
     * binaries that don't pull in hack.h (where BUFSZ=256 is defined).
     * A _Static_assert in save.c enforces BUFSZ == 256 / ZEROCOMP_BUFSIZ
     * to catch any future config drift. */
    int                  s_count_only;            /* save.c (MFLOPPY-gated; harmless on UNIX) */
    unsigned             s_ustuck_id;             /* save.c (preserve monster id across save) */
    unsigned             s_usteed_id;             /* save.c (preserve steed id across save) */
    FILE                *s_bw_FILE;               /* save.c (def_bufon fdopen'd save stream) */
    unsigned char        s_outbuf[256];           /* save.c (BUFSZ == ZEROCOMP_BUFSIZ on UNIX) */
    unsigned short       s_outbufp;               /* save.c (zerocomp output cursor) */
    short                s_outrunlength;          /* save.c (RLE run len; -1 == no run) */
    int                  s_bwritefd;              /* save.c (zerocomp active fd) */
    boolean              s_compressing;           /* save.c (zerocomp mode flag) */
    struct fruit        *s_oldfruit;              /* restore.c (ghost-level fruit chain) */
    long                 s_omoves;                /* restore.c (ghost-level monstermoves) */

    /* Quest/pray/artifact per-env state.
     * Nine file-statics across questpgr.c, pray.c, artifact.c that
     * race across envs in shared-libnethack vecenv (most notable:
     * artiexist[] leaking "Excalibur exists" across env universes).
     * Direct fields; macros at the top of each .c file rewrite
     * accesses to current_nle_ctx->s_<name>.
     *
     * Notes:
     *  - s_artiexist sized to literal (1+NROFARTIFACTS+1) = 35 because
     *    nle.h doesn't pull onames.h (where NROFARTIFACTS == 33 is
     *    generated by makedefs). A _Static_assert in artifact.c
     *    enforces NROFARTIFACTS == 33 to catch drift.
     *  - s_qt_list_p is a pointer to a heap-allocated struct qtlists;
     *    nle.c allocates it via nle_qtlist_alloc() in init_nle so
     *    nle.c doesn't have to pull qtext.h either.
     *  - s_msg_file uses struct dlb_handle * (the typedef target of
     *    `dlb`), so questpgr.c's macro can use it as a plain `dlb *`
     *    once dlb.h is included. */
    char                 s_cvt_buf[64];           /* questpgr.c (convert_arg work buf) */
    char                 s_nambuf[64];            /* questpgr.c (ldrname/neminame staging) */
    struct qtlists      *s_qt_list_p;             /* questpgr.c (quest text msg index) */
    struct dlb_handle   *s_msg_file;              /* questpgr.c (open quest.dat handle) */
    signed char          s_p_aligntyp;            /* pray.c (alignment of pending prayer; aligntyp == schar == signed char) */
    int                  s_p_trouble;             /* pray.c (trouble code at prayer start) */
    int                  s_p_type;                /* pray.c ((-1)..3: prayer outcome class) */
    boolean              s_artiexist[35];         /* artifact.c (1+NROFARTIFACTS+1; NROFARTIFACTS==33) */
    /* Artidisco[] migrated off process-shared static. Was a
     * STATIC_OVL xchar in artifact.c; init_artifacts() memset()s it on every
     * env reset, racing with discover_artifact() on other envs. xchar is a
     * typedef for schar (signed char); size matches NROFARTIFACTS == 33. */
    signed char          s_artidisco[33];         /* artifact.c (NROFARTIFACTS; xchar == schar) */
    boolean              s_touch_blasted;         /* artifact.c (retouch_object damage flag) */

    /* Single-action target caches per-env.
     * Eight file-statics that hold the "current action target" between
     * tick N (action started) and tick N+1 (continuation / y-n prompt).
     * At N>=128 envs, env A's cache (e.g. telescroll = the scroll
     * currently being read) was visible to env B's continuation,
     * corrupting both games. Direct fields here; macros at the top of
     * each .c file rewrite bare-name accesses to current_nle_ctx->s_<name>.
     *
     * struct litmon is defined locally inside src/read.c (a small linked
     * list node holding {struct monst*, struct litmon* nxt}); forward
     * decl in nle.h is sufficient because only read.c dereferences it. */
    boolean              s_obj_zapped;            /* zap.c (polyuse cookie) */
    int                  s_poly_zapped;           /* zap.c (polyuse cookie) */
    boolean              s_did_dig_msg;           /* dig.c (dig-msg latch) */
    struct engr         *s_head_engr;             /* engrave.c (engr list head) */
    struct litmon       *s_gremlins;              /* read.c (gremlin-spawn queue) */
    int                  s_force_mintrap;         /* trap.c (mintrap() flag stand-in) */
    struct obj          *s_telescroll;            /* teleport.c (scroll currently being read) */
    struct nhcoord      *s_utrack;                /* track.c (UTSZ=50 player-step ring; heap-alloc'd in init_nle to keep coord.h out of nle.h, matching s6_inv_pos_p / s7_doors_p / bhitpos_p pattern) */
    struct obj          *s_propellor;             /* weapon.c (ranged-weapon select cache) */

    /* Invent.c + pickup.c per-env state.
     * Ten file-statics that race across envs during inventory display,
     * sort, container loot, and object-class menu filtering. Direct
     * fields on nle_ctx_t (no swap struct); macros at the top of each
     * .c file rewrite accesses to current_nle_ctx->s_<name>.
     *
     * Notes:
     *   - s_cached_pickinv_win must be initialized to WIN_ERR (-1), not
     *     the calloc-zero default; init_nle() in nle.c sets it after
     *     allocation. (winid 0 is a valid window — calloc-zero would
     *     make the "no cache" check fail and crash destroy_nhwindow.)
     *   - s_only_x / s_only_y replace `static coord only` in invent.c;
     *     we use two raw signed-char fields instead of a coord pointer
     *     to keep coord.h out of nle.h (xchar == signed char). The 4
     *     access sites in invent.c are rewritten in-place.
     *   - s_invbuf is the heap buffer pointer itself (NetHack reallocs
     *     it via alloc()/free()); only the pointer + size move into
     *     the ctx, the buffer stays on the heap.
     *   - s_valid_menu_classes is MAXOCLASSES(18) + 1 + 4 + 1 = 24
     *     bytes; literal 24 here so nle.h doesn't need objclass.h. */
    unsigned             s_sortlootmode;          /* invent.c (sortloot_cmp mode) */
    int                  s_cached_pickinv_win;    /* invent.c (winid; init to WIN_ERR in init_nle) */
    int                  s_this_type;             /* invent.c (this_type_only filter) */
    char                *s_invbuf;                /* invent.c (let_to_name scratch ptr) */
    unsigned             s_invbufsiz;             /* invent.c (let_to_name scratch size) */
    signed char          s_only_x;                /* invent.c (only_here coord.x, was xchar) */
    signed char          s_only_y;                /* invent.c (only_here coord.y, was xchar) */
    struct obj          *s_current_container;     /* pickup.c (active container for loot) */
    boolean              s_abort_looting;         /* pickup.c (use_container abort flag) */
    long                 s_val_for_n_or_more;     /* pickup.c (n_or_more threshold) */
    char                 s_valid_menu_classes[24]; /* pickup.c (MAXOCLASSES+1+4+1) */

    /* Level-build per-env
     *
     * File-statics from sp_lev.c / mkmaze.c / mkmap.c that hold transient
     * state during mklev() and special-level loading. With N>=128 PufferLib
     * envs in one process, env A's mid-build values can be observed by env
     * B's continuation (mklev yields through pline/menu prompts), corrupting
     * level construction and surfacing as libc memcpy NULL-source GPFs.
     *
     * Heavy NetHack types are forward-declared (struct obj, struct monst,
     * struct trap) and stored as pointers / array-of-pointers so this block
     * does not pull objclass.h / monst.h / trap.h cascade into util TUs.
     * `lev_region` is a typedef; the underlying struct gets the tag
     * `nle_lev_region_s` (added in sp_lev.h) so it can be forward-declared
     * here without including sp_lev.h. */
    struct obj                  *s_container_obj[10]; /* MAX_CONTAINMENT == 10
                                                       * (sp_lev.h); literal
                                                       * to keep nle.h light;
                                                       * _Static_assert in
                                                       * sp_lev.c catches
                                                       * future drift. */
    /* s_container_idx already declared in the block above */
    struct monst                *s_invent_carrying_monster;
    int                          s_mines_prize_count;
    int                          s_soko_prize_count;
    schar                        s_floodfillchk_match_under_typ;
    /* mkmaze.c */
    struct nle_lev_region_s     *s_bughack;       /* lev_region *; lazily
                                                   * allocated on first use
                                                   * in mkmaze.c so nle.c
                                                   * does not need to pull
                                                   * sp_lev.h. */
    struct trap                 *s_wportal;
    /* mkmap.c */
    char                        *s_new_locations;
    int                          s_n_loc_filled;

    /* Combat tick per-env.
     * File-statics in mhitm.c / mhitu.c / mthrowu.c / muse.c that
     * carry state across a pline / Y-N prompt yield within one
     * combat tick. With N>=128 parallel envs, env A's pointer
     * could become dangling when env B reads it. Direct-ctx fields
     * (NOT swap pattern). */
    long          s_noisetime;        /* mhitm.c noisetime */
    struct obj   *s_mhitm_otmp;       /* mhitm.c otmp (file-static) */
    int           s_dieroll_mhitm;    /* mhitm.c dieroll */
    struct obj   *s_mon_currwep;      /* mhitu.c mon_currwep */
    int           s_dieroll_mhitu;    /* mhitu.c dieroll */
    int           s_mesg_given;       /* mthrowu.c mesg_given */
    boolean       s_zap_oseen;        /* muse.c zap_oseen */

    /* Misc-1 per-env. 14 file-scope statics scraped
     * out of cmd.c, hack.c, display.c, vision.c, do_name.c, topten.c, end.c.
     * Direct ctx fields (no swap). Macros at the top of each .c file rewrite
     * accesses to current_nle_ctx->s_<name>. Pointers used where the type
     * needs hack.h headers that are too heavy for nle.h (coord, opvar, the
     * generated close2d/far2d tables); inline scalars otherwise. */
    /* cmd.c */
    boolean                       s_alt_esc;           /* readchar() ESC<->META switch */
    struct nhcoord               *s_clicklook_cc;      /* last click-look coord (alloc in init_nle) */
    /* hack.c */
    union any                    *s_tmp_anything;      /* uint_to_any/long_to_any/... scratch */
    int                           s_wc;                /* inv_weight()'s last weight_cap() */
    /* display.c */
    int                           s_nul_gbuf_new;      /* nul_gbuf.new — zero glyph fill */
    int                           s_nul_gbuf_glyph;    /* nul_gbuf.glyph — cmap_to_glyph(S_stone) */
    int                           s_bad_count[36];     /* WA_VERBOSE; MAX_TYPE == 36 in rm.h */
    /* vision.c — close_dy/far_dy are arrays-of-pointers into the generated
     * close_table[]/far_table[] (vis_tab.h). Hold as opaque void * here and
     * memcpy the array contents in init_nle via a vision.c helper. */
    void                         *s_close_dy;          /* close2d *close_dy[CLOSE_MAX_BC_DY] */
    void                         *s_far_dy;            /* far2d   *far_dy[FAR_MAX_BC_DY] */
    /* do_name.c */
    struct opvar                 *s_gloc_filter_map;
    int                           s_gloc_filter_floodfill_match_glyph;
    int                           s_via_naming;
    /* topten.c — `toptenwin` collides with `iflags.toptenwin` (flag.h field).
     * Macro renamed to `nle_toptenwin` inside topten.c; the field is still
     * stored as s_toptenwin to keep the s_ naming convention. */
    long                          s_final_fpos;        /* UPDATE_RECORD_IN_PLACE — dead on UNIX */
    int                           s_toptenwin;         /* winid (typedef'd int) — TT scroll window */
    /* end.c */
    int                           s_vanq_sortmode;     /* VANQ_MLVL_MNDX default == 0, calloc OK */

    /* Misc-2 per-env state.
     * Migrated from file-statics in files.c, mon.c, extralev.c, rect.c,
     * rumors.c, dbridge.c, polyself.c. All are persistent per-env scratch
     * for level building / config parsing / polymorph cycles. */
    char            s_wizkit[128];          /* files.c wizkit[WIZKIT_MAX] (WIZKIT_MAX=128) */
    int             s_lockptr;              /* files.c lockptr (AMIGA / WIN32 / MSDOS) */
    char           *s_config_section_chosen;  /* files.c config_section_chosen */
    char           *s_config_section_current; /* files.c config_section_current */
    short          *s_animal_list;          /* mon.c animal_list (heap, lazy alloc'd) */
    int             s_animal_list_count;    /* mon.c animal_list_count */
    struct rogueroom *s_extralev_r;         /* extralev.c r[3][3] (heap, lazy alloc'd) */
    void           *s_rect;                 /* rect.c rect[MAXRECT+1]; void* to avoid pulling rect.h */
    int             s_rect_cnt;             /* rect.c rect_cnt */
    long            s_true_rumor_size;      /* rumors.c true_rumor_size */
    struct entity  *s_occupants;            /* dbridge.c occupants[ENTITIES=2] (heap, lazy alloc'd) */
    int             s_sex_change_ok;        /* polyself.c sex_change_ok */

    /* Function-local statics: rnd/trap/mkmaze */
    unsigned                     s_rn2disprng_seed;        /* rnd.c rn2_on_display_rng (non-ISAAC64) */
    boolean                      s_dotrap_recursive_mine;  /* trap.c dotrap landmine recursion guard */
    boolean                      s_movebubbles_up;         /* mkmaze.c movebubbles up/down latch */

    /* Function-local statics: hack/dog */
    long                          s_moverock_lastmovetime;     /* hack.c moverock */
    int                           s_domove_skates;             /* hack.c domove_core */
    signed char                   s_spoteffects_spotloc_x;     /* hack.c spoteffects (coord.x) */
    signed char                   s_spoteffects_spotloc_y;     /* hack.c spoteffects (coord.y) */
    int                           s_spoteffects_spotterrain;   /* hack.c spoteffects */
    struct trap                  *s_spoteffects_spottrap;      /* hack.c spoteffects */
    unsigned                      s_spoteffects_spottraptyp;   /* hack.c spoteffects (NO_TRAP==0) */
    int                           s_makedog_petname_used;      /* dog.c makedog */
    /* Function-local statics: display/topten */
    struct tmp_glyph    *s_tmp_at_tglyph;               /* display.c tmp_at() animation list head */
    boolean              s_cls_in_cls;                  /* display.c cls() recursion guard */
    int                  s_flush_screen_flushing;       /* display.c flush_screen() recursion guard */
    int                  s_flush_screen_delay_flushing; /* display.c flush_screen() delay latch */
    struct toptenentry  *s_get_rnd_toptenentry_tt_buf;  /* topten.c get_rnd_toptenentry scratch (lazy-alloc) */

    /* Residual race surfaces.
     * Final 7 file-statics flagged by the AX-fix-1 diagnostic that produce
     * incorrect gameplay (not crashes) until migrated. Direct ctx fields
     * (no swap); macros at top of each .c file rewrite bare-name accesses
     * to current_nle_ctx->s_<name>.
     *
     * Notes:
     *  - s_eat_msgbuf is sized literally to 256 (== BUFSZ on this build)
     *    because nle.h is included by util TUs that don't pull hack.h;
     *    a _Static_assert in eat.c enforces BUFSZ == 256 (same precedent
     *    as s_outbuf in save.c).
     *  - s_bcpbreadcrumbs / s_bcubreadcrumbs use pointers (lazy alloc in
     *    ball.c) because struct breadcrumbs is defined in decl.h, which
     *    nle.h cannot include without re-defining it; forward-decl here
     *    is sufficient for pointer-only use. */
    boolean              s_notonhead;             /* potion.c (worm-tail target flag, cross-file extern) */
    struct monst        *s_mthrowu_target;        /* mthrowu.c (combat target cache) */
    struct monst        *s_mthrowu_archer;        /* mthrowu.c (combat archer cache) */
    char                 s_eat_msgbuf[256];       /* eat.c (BUFSZ pline scratch) */
    boolean              s_eat_force_save_hs;     /* eat.c (force-save hunger state) */
    struct breadcrumbs  *s_bcpbreadcrumbs;        /* ball.c (ball/chain trail, lazy alloc) */
    struct breadcrumbs  *s_bcubreadcrumbs;        /* ball.c (ball/chain trail, lazy alloc) */

    /* Function-local statics (medium): single-action caches */
    long                s_breakobj_lastmovetime;          /* dothrow.c breakobj */
    boolean             s_breakobj_peaceful_shk;          /* dothrow.c breakobj */
    long                s_elemental_clog_msgmv;           /* mon.c elemental_clog */
    long                s_pick_pick_pickmovetime;         /* shk.c pick_pick */
    long                s_maybe_cannibal_ate_brains;      /* eat.c maybe_cannibal */
    unsigned            s_newuhs_save_hs;                 /* eat.c newuhs */
    boolean             s_newuhs_saved_hs;                /* eat.c newuhs */
    boolean             s_autopick_costly;                /* pickup.c autopick_testobj */
    int                 s_encumber_msg_oldcap;            /* pickup.c encumber_msg (UNENCUMBERED=0) */
    boolean             s_hitum_cleave_clockwise;         /* uhitm.c hitum_cleave */
    long                s_ck_server_admin_msg_lastchk;    /* mail.c ck_server_admin_msg */

    /* Residual file-statics flagged by AX diagnostic.
     * acid_ctx is opaque void* — struct h2o_ctx lives local in trap.c. */
    void                *s_acid_ctx;                      /* trap.c water_damage_chain ctx (heap, lazy) */

    /* Persistent counters / gated-path statics that still race
     * across envs in shared-libnethack vecenv. Direct fields; macros at the
     * top of each .c file rewrite bare-name accesses to current_nle_ctx->s_<name>.
     *  - jumping_is_magic: apply.c — set before walk_path callback path; races.
     *  - vmc_count:        pickup.c add_valid_menu_class — accumulates across calls.
     *  - topl_nxtidx / topl_initd: win/tty/topl.c — tty_getmsghistory / tty_putmsghistory
     *    persistent state, reached from RL frontend via winrl.cc rl_get/putmsghistory.
     * (branch_id in dungeon.c reuses the already-existing s_branch_id_ctr
     *  field; no new field needed.) */
    int                  s_jumping_is_magic;   /* apply.c get_valid_jump_position */
    int                  s_vmc_count;          /* pickup.c add_valid_menu_class */
    int                  s_topl_nxtidx;        /* topl.c tty_getmsghistory cursor */
    boolean              s_topl_initd;         /* topl.c tty_putmsghistory init flag */

    /* Per-env return buffers for ~22 "returns pointer to
     * internal static" functions. Each was a function-local static char buf:
     * racy under N>=2 vecenv when env A's mid-call buffer would be clobbered
     * by env B in the time between the function returning and the caller
     * consuming the pointer. Direct ctx fields (no swap); file-level macros
     * in each .c rewrite bare-name references to current_nle_ctx->s_<...>.
     * Where the original variable name collides with locals/parameters
     * elsewhere in the same TU (e.g. `buf` in hack.c, priest.c, do_name.c),
     * the source was renamed to a unique tag so the macro is conflict-free. */
    char                 s_allmain_pbar[80 /* COLNO */];           /* allmain.c do_positionbar */
    char                 s_artifact_resbuf[20];                    /* artifact.c glow_verb */
    char                 s_attrib_from_what_buf[256 /* BUFSZ */];  /* attrib.c from_what (renamed from buf) */
    char                 s_botl_strength_buf[32];                  /* botl.c get_strength_str (renamed from buf) */
    char                 s_dbridge_wholebuf[80];                   /* dbridge.c E_phrase */
    char                 s_do_dowipe_buf[39];                      /* do.c dowipe (renamed from buf) */
    char                 s_do_name_dxdy_buf[30];                   /* do_name.c dxdy_to_dist_descr (renamed from buf) */
    char                 s_do_name_rndmonnam_buf[256];             /* do_name.c rndmonnam (BUFSZ; renamed from buf) */
    char                 s_do_wear_offdelaybuf[60];                /* do_wear.c armoroff */
    char                 s_hack_in_rooms_buf[5];                   /* hack.c in_rooms (renamed from buf) */
    char                 s_hacklib_ing_suffix_buf[256];            /* hacklib.c ing_suffix (BUFSZ; renamed from buf) */
    int                  s_hacklib_visctrl_nbuf;                   /* hacklib.c visctrl rotating idx */
    char                 s_hacklib_visctrl_bufs[5][5];             /* hacklib.c visctrl pool [VISCTRL_NBUF][5] */
    char                 s_hacklib_datestr_yyyymmddhhmmss[15];     /* hacklib.c yyyymmddhhmmss (renamed from datestr) */
    char                 s_invent_armcat[8];                       /* invent.c sortloot_cmp */
    char                 s_invent_li[256];                         /* invent.c xprname (BUFSZ) */
    char                 s_invent_altbuf[256];                     /* invent.c lookup_feature_name (BUFSZ) */
    char                 s_mapglyph_encbuf[20];                    /* mapglyph.c encglyph */
    char                 s_mkobj_unknown[32];                      /* mkobj.c where_name */
    /* mkroom.c shrine_pos — was `static coord buf;`. nle.h forward-decls
     * struct nhcoord but cannot include coord.h, so store as two xchar
     * (signed char) fields instead and let the .c reconstitute a coord. */
    signed char          s_mkroom_shrine_buf_x;
    signed char          s_mkroom_shrine_buf_y;
    char                 s_priest_piousness_buf[32];               /* priest.c piousness (renamed from buf) */
    char                 s_shk_empty_shops[5];                     /* shk.c u_entered_shop */
    char                 s_trap_tnbuf[12];                         /* trap.c trapnote */
    char                 s_uhitm_msgbuf[256];                      /* uhitm.c gulpum (BUFSZ) */
    unsigned char        s_vision_colbump[81 /* COLNO+1 = 80+1 */]; /* vision.c vision_recalc */

    /* Save/restore-path dispatch tables and zerocomp read buffer.
     * Were file-scope `static` in restore.c and save.c — process-globals.
     *
     * Root cause of N=1024 short-read panics: under PufferLib's OMP-parallel
     * vecenv stepping, env A's mread() could see env B's just-mutated
     * `restoreprocs.restore_mread` (def_mread vs zerocomp_mread) after a
     * `set_restpref()` from another env's options/validate path, decoding
     * env A's level file with the wrong codec → short read → panic.
     *
     * Even more critical: the zerocomp READ-side buffer (inbuf/inbufp/
     * inbufsz/inrunlength/mreadfd) was a single shared array. Two envs
     * concurrently decoding savefiles would clobber each other's read fd
     * and partially-consumed buffer.
     *
     * Stored as untyped fields (void* / unsigned long) to avoid pulling
     * <stdio.h>-dependent typedefs into nle.h; restore.c / save.c cast
     * via macros. Use `#define <name> (current_nle_ctx->s_<name>)`.
     */
    /* restoreprocs */
    const char          *s_restoreprocs_name;
    int                  s_restoreprocs_mread_flags;
    void               (*s_restoreprocs_restore_minit)(void);
    void               (*s_restoreprocs_restore_mread)(int, void *, unsigned int);
    void               (*s_restoreprocs_restore_bclose)(int);
    /* saveprocs */
    const char          *s_saveprocs_name;
    void               (*s_saveprocs_save_bufon)(int);
    void               (*s_saveprocs_save_bufoff)(int);
    void               (*s_saveprocs_save_bflush)(int);
    void               (*s_saveprocs_save_bwrite)(int, void *, unsigned int);
    void               (*s_saveprocs_save_bclose)(int);
    /* sfrestinfo / sfsaveinfo (per-env feature flags during save/restore).
     * Laid out as 3 ulongs to match `struct savefile_info` (global.h:312).
     * .c files use a struct-cast macro so existing `sfrestinfo.sfi1` etc.
     * and `bwrite(fd, &sfsaveinfo, sizeof sfsaveinfo)` continue to work. */
    unsigned long        s_sfrestinfo_sfi1;
    unsigned long        s_sfrestinfo_sfi2;
    unsigned long        s_sfrestinfo_sfi3;
    unsigned long        s_sfsaveinfo_sfi1;
    unsigned long        s_sfsaveinfo_sfi2;
    unsigned long        s_sfsaveinfo_sfi3;
    /* zerocomp read-side buffer state (restore.c) */
    unsigned char        s_zc_inbuf[256 /* ZEROCOMP_BUFSIZ == BUFSZ */];
    unsigned short       s_zc_inbufp;
    unsigned short       s_zc_inbufsz;
    short                s_zc_inrunlength;
    int                  s_zc_mreadfd;

    /* 5 hot-path monster-turn statics migrated to per-env.
     * Identified by audit; all on the monster-turn hot path and likely
     * sources of cross-env cache-line contention under OMP-128 stepping.
     * Direct ctx fields; macros at top of each .c file rewrite bare-name
     * accesses to current_nle_ctx->s_<name>. */
    signed char          s_gtyp;            /* dogmove.c (xchar == schar) */
    signed char          s_gx;              /* dogmove.c (xchar == schar) */
    signed char          s_gy;              /* dogmove.c (xchar == schar) */
    boolean              s_m_using;         /* muse.c (cross-TU: zap.c) */
    boolean              s_vis;             /* mhitm.c (m-vs-m vis flag) */
    boolean              s_far_noise;       /* mhitm.c (m-vs-m noise flag) */
    boolean              s_vamp_rise_msg;   /* mon.c (vamp-rise message flag) */
    boolean              s_disintegested;   /* mon.c (digested/disintegrated flag) */
    boolean              s_read_known;      /* read.c (cross-TU: detect.c) */

    /* Per-action warm globals migrated to per-env.
     * Written on per-action paths (less frequent than every tick) but
     * still race-prone across envs under OMP. */
    boolean              s_class_filter;       /* pickup.c filter trio */
    boolean              s_bucx_filter;        /* pickup.c filter trio */
    boolean              s_shop_filter;        /* pickup.c filter trio */
    int                  s_potion_nothing;     /* potion.c per-quaff accumulator */
    int                  s_potion_unkn;        /* potion.c per-quaff accumulator */
    char                 s_safeq_xprn_let;     /* invent.c safeq_xprn_ctx.let */
    boolean              s_safeq_xprn_dot;     /* invent.c safeq_xprn_ctx.dot */
    signed char          s_swallowed_lastx;    /* display.c swallowed() (xchar) */
    signed char          s_swallowed_lasty;    /* display.c swallowed() (xchar) */
    signed char          s_under_water_lastx;  /* display.c under_water() (xchar) */
    signed char          s_under_water_lasty;  /* display.c under_water() (xchar) */
    boolean              s_under_water_dela;   /* display.c under_water() */
    boolean              s_under_ground_dela;  /* display.c under_ground() */

    /* Muse.c file-statics. `struct musable m` and `trapx/trapy`
     * were process-global writes on the per-monster-turn path in muse.c
     * (called from find_offensive/defensive/misc and use_*). With N>=192
     * envs stepping in parallel, two envs' monsters could both stomp m
     * concurrently, leaving one with another env's `m.offensive` pointer
     * and crashing in use_offensive() at muse.c:1421 on a stale heap
     * struct. m_using was already migrated; this completes
     * the muse.c migration. struct musable is opaque here; muse.c casts
     * the void* to its own struct. */
    void *               s_muse_m_p;           /* muse.c: struct musable */
    int                  s_muse_trapx;         /* muse.c: trapx */
    int                  s_muse_trapy;         /* muse.c: trapy */

    /* Sp_lev / track / mkmaze per-action globals migrated.
     *
     * sp_lev.c lev_message/lregions/num_lregions: NON-static cross-TU
     * globals. lev_message is a malloc'd char* set in sp_lev.c:2997 and
     * freed in questpgr.c:714 (deliver_splev_message). lregions is a
     * lev_region* freed in mkmaze.c:649. With N envs in one process,
     * env A's pending lev_message could be free()d by env B's level
     * entry, leaving env A with a dangling pointer (UAF) — the most
     * plausible source of intermittent obs=0x4 corruption.
     *
     * sp_lev.c xstart/ystart/xsize/ysize: bounding box for special-level
     * region currently loading. Was static NEARDATA (file-scope TLS);
     * if two envs concurrently gen a special level on the same thread,
     * the box gets clobbered → out-of-bounds levl[][] write.
     *
     * track.c utcnt/utpnt: index counters for utrack[] (already migrated
     * to s_utrack). The counter staying TLS while the array is per-env
     * means an OMP resume on a different thread reads 0 — env B then
     * writes past env A's UTSZ buffer into adjacent ctx fields.
     *
     * mkmaze.c bbubbles/ebubbles + xmin/ymin/xmax/ymax: head/tail
     * pointers of the water-level bubble linked list plus its bounds.
     * Was static (process-global). Two envs entering water level
     * concurrently would clobber each other's lists. struct bubble is
     * opaque here; mkmaze.c casts the void* to its own type. */
    char                *s_sp_lev_message_p;        /* sp_lev.c lev_message */
    struct nle_lev_region_s *s_sp_lregions_p;       /* sp_lev.c lregions */
    int                  s_sp_num_lregions;         /* sp_lev.c num_lregions */
    signed char          s_sp_xstart;               /* sp_lev.c xstart (xchar) */
    signed char          s_sp_ystart;               /* sp_lev.c ystart (xchar) */
    char                 s_sp_xsize;                /* sp_lev.c xsize */
    char                 s_sp_ysize;                /* sp_lev.c ysize */
    int                  s_utcnt;                   /* track.c utcnt (utrack count) */
    int                  s_utpnt;                   /* track.c utpnt (utrack write idx) */
    void                *s_bbubbles;                /* mkmaze.c bbubbles (struct bubble *) */
    void                *s_ebubbles;                /* mkmaze.c ebubbles (struct bubble *) */
    int                  s_water_xmin;              /* mkmaze.c xmin (water-level bound) */
    int                  s_water_ymin;              /* mkmaze.c ymin */
    int                  s_water_xmax;              /* mkmaze.c xmax */
    int                  s_water_ymax;              /* mkmaze.c ymax */

    /* role.c per-env state (pa[] + post_attribs). NUM_BP==4. */
    char                 s_role_pa[4];              /* role.c pa[NUM_BP] */
    char                 s_role_post_attribs;       /* role.c post_attribs */
    /* Per-env trap launch state (trap.c launchplace). */
    void                *s_launchplace_obj;
    signed char          s_launchplace_x;
    signed char          s_launchplace_y;
    /* Per-env topten linked list head (topten.c). */
    void                *s_tt_head;
    /* Parametric difficulty knobs. Embedded by value so nle_fr_snapshot
     * (which copies the ctx) captures it; read at engine decision sites. */
    nle_tune_t           s_tune;
} nle_ctx_t;

/*
 * Refactor stage 3: declared extern here, defined once in nle.c. Was a
 * tentative-definition (common symbol) — that broke under ASan ODR after
 * many TUs started including nle.h.
 *
 * Stage 10'+: __thread enables OMP-parallel stepping. Each thread chases
 * its own nle_ctx_t through this pointer; since all per-env state lives
 * in nle_ctx_t (after stages 1-10 + 5'-10' migrations), threads are
 * naturally isolated — no shared mutable globals to race on.
 *
 * macOS caveat: __thread on a dynamically-loaded library prevents
 * dlclose() from unloading. Not applicable for our Linux/HPC target;
 * if we ever need macOS dynamic-unload, gate this with #ifndef __APPLE__.
 */
extern __attribute__((tls_model("initial-exec")))
       __thread nle_ctx_t *current_nle_ctx;

nle_ctx_t *nle_start(nle_obs *, FILE *, nle_seeds_init_t *, nle_settings *);
nle_ctx_t *nle_step(nle_ctx_t *, nle_obs *);
void nle_end(nle_ctx_t *);

void nle_set_seed(nle_ctx_t *, unsigned long, unsigned long, boolean);
void nle_get_seed(nle_ctx_t *, unsigned long *, unsigned long *, boolean *);

/* Debug: dump the per-env arena memory map (named buffers + fmon/fobj chains +
 * monster grid with fmon-membership/data-validity) to `path` (NULL => stderr).
 * Diagnostic for arena-reuse / dangling-pointer investigations. */
void nle_dbg_memmap(nle_ctx_t *, const char *);

/* Single-level blob save/load.
 *
 * nle_save_level serializes the current dungeon level to a malloc'd byte
 * blob (length written to *out_len); the caller owns it and must release
 * it with nle_free_blob. nle_load_level stamps a previously saved blob over
 * the current level and reloads its contents in place.
 *
 * NOTE: nle_load_level is two-phase. It mutates engine state and resets
 * vision but does NOT re-render the map (re-rendering routes through the
 * window port, which yields the game coroutine and is unsafe from this
 * entry point). The caller must step the game once to render. */
void *nle_save_level(nle_ctx_t *, long *out_len);
void  nle_free_blob(void *blob);
int   nle_load_level(nle_ctx_t *, const void *blob, long len);

/* Hero (player) state blob save/load.
 *
 * nle_save_player serializes the full hero gamestate -- the `u` struct,
 * inventory, attributes, killers/timers/light-sources and the dungeon graph
 * (everything dosave0()'s gamestate tail writes), WITHOUT the level map -- to
 * a malloc'd byte blob (length written to *out_len). The live game is left
 * intact (WRITE_SAVE, not FREE_SAVE). Caller owns the blob; free it with
 * nle_free_blob. Pairs with nle_save_level: a checkpoint = level blob +
 * player blob. Returns the blob, or NULL on error.
 *
 * nle_load_player restores such a blob onto the CURRENT level. Returns 0 on
 * success, nonzero on error.
 *
 * LOAD ORDERING CONTRACT: call nle_load_level BEFORE nle_load_player. The
 * player restore relinks u.ustuck / u.usteed and the worn ball/chain against
 * the current level's monster/object chains, so the target level must already
 * be installed.
 *
 * NOTE: nle_load_player is two-phase, like nle_load_level. It mutates engine
 * state and resets vision but does NOT re-render (re-rendering routes through
 * the window port, which yields the game coroutine and is unsafe from this
 * entry point). The caller must step the game once to render. */
void *nle_save_player(nle_ctx_t *, long *out_len);
int   nle_load_player(nle_ctx_t *, const void *blob, long len);

/* Secure state-modification API.
 *
 * nle_set_state pokes a curated whitelist of simple integer player fields.
 * "field" is one of: "hp", "max_hp", "gold", "xp_level", "hunger", or one of
 * the six attributes "str"/"dex"/"con"/"int"/"wis"/"cha" (value is NetHack's
 * encoded attribute: 3..18, 19..118 == 18/01..18/00, 119..125 == 19..25).
 * Returns 0 on success, nonzero for an unknown field. The C side only
 * provides the setters; the binding is responsible for validating bounds.
 *
 * nle_goto_depth schedules a DEFERRED move of the hero to dungeon level n
 * in the current dungeon branch. It does not change levels synchronously
 * (goto_level routes through the window port and yields the coroutine,
 * which would crash from this entry point); instead it sets u.utolev /
 * u.utotype so the game loop performs the change via deferred_goto() on
 * the next nle_step(). Returns 0 on success, nonzero on bad target. */
int nle_set_state(nle_ctx_t *, const char *field, long value);
int nle_goto_depth(nle_ctx_t *, int n);

/* nle_seat_on_stair seats the hero on the down (down != 0) or up staircase of
 * the current level, if present. Two-phase like goto_depth: the caller steps
 * once to re-render. Returns 0 on success, nonzero if no such stair exists.
 *
 * nle_level_up raises the hero n experience levels with the normal HP/stat
 * gains (pluslvl), capped at level 30, and bumps u.uexp to the new level
 * threshold so the next newexplevel() won't undo it. Caller steps once to
 * refresh blstats. Returns 0 on success. */
int nle_seat_on_stair(nle_ctx_t *, int down);
int nle_level_up(nle_ctx_t *, int n);

/* Curriculum traversal: cross-branch goto + dungeon-table query.
 *
 * nle_num_dungeons returns the number of dungeon branches defined.
 * nle_dungeon_info reports branch `idx`'s name / depth_start / num_dunlevs
 * (any out pointer may be NULL); lets the caller map an absolute "Dlvl N" to
 * a (dnum, dlevel) and find Gehennom / the Elemental Planes by name.
 * nle_goto_abs schedules a DEFERRED move to an arbitrary (dnum, dlevel),
 * including a different branch than the hero's current one (unlike
 * nle_goto_depth, which pins the branch). Two-phase: goto_level() runs via
 * deferred_goto() on the next nle_step() and generates the level on demand.
 * Reaching the Elemental Planes grants the Amulet (the goto_level gate). All
 * return 0 on success, nonzero on a bad index / out-of-range target. */
int nle_num_dungeons(nle_ctx_t *);
int nle_dungeon_info(nle_ctx_t *, int idx, char *name_out, int name_cap,
                     int *depth_start_out, int *num_dunlevs_out);
int nle_goto_abs(nle_ctx_t *, int dnum, int dlevel);
/* nle_hero_on_stair: +1 on the down stair, -1 on the up stair, 0 otherwise. */
int nle_hero_on_stair(nle_ctx_t *);

/* nle_grant_invocation_kit: drop the pre-primed invocation artifacts (lit
 * spe=7 Candelabrum, charged Bell of Opening, Book of the Dead — all uncursed)
 * straight into the hero's pack so the curriculum agent can perform the
 * invocation ritual. Returns 0. */
int nle_grant_invocation_kit(nle_ctx_t *);
/* nle_invocation_pos: write the vibrating-square coords into *x,*y; returns 0
 * on the Invocation level (coords valid), nonzero otherwise (x,y set 0). */
int nle_invocation_pos(nle_ctx_t *, int *x, int *y);
/* nle_seat_on_invocation_square: stage the hero at the vibrating square (the
 * ritual site) on the Invocation level. adjacent!=0 places the hero on a tile
 * next to the square (agent steps on); adjacent==0 places it on the square.
 * Returns 0 on success. */
int nle_seat_on_invocation_square(nle_ctx_t *, int adjacent);

/* nle_state refactor — per-instance accessors. Called from rnd.c (and
 * other subsystems as they migrate). Each returns a pointer into the
 * current nle_ctx_t. CORE = 0 (gameplay RNG), DISP = 1 (display RNG). */
isaac64_ctx *nle_rng_state(int idx);
int          *nle_rng_init_flag(int idx);

/* Arena-backed calloc (alloc.c). Per-env state allocated through this lands
 * in the per-env arena and is captured by nle_fr_snapshot. */
void *nle_arena_calloc(size_t count, size_t size);

/* Difficulty knob catalog (nle.c). The binding calls count()/name() once to
 * learn the catalog, then reads/writes nle_get_tune(nle) as a flat double[]. */
int          nle_tune_count(void);
const char  *nle_tune_name(int index);
void         nle_tune_set_defaults(nle_tune_t *t);
nle_tune_t  *nle_get_tune(nle_ctx_t *nle);

#endif /* NLE_H */
