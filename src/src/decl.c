/* NetHack 3.6	decl.c	$NHDT-Date: 1573869062 2019/11/16 01:51:02 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.149 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2009. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "nle.h" /* current_nle_ctx, refactor */

#ifdef NLE_OBJECTS_GLOBAL
int NDECL((*afternmv));
int NDECL((*occupation));
#endif

/* from xxxmain.c */
const char *hname = 0; /* name of the game (argv[0] of main) */
/* hackpid migrated to nle_ctx_t (refactor stage 3c). */
#if defined(UNIX) || defined(VMS)
#endif
#ifdef DEF_PAGER
char *catmore = 0; /* default pager */
#endif
char chosen_windowtype[WINTYPELEN];

#ifdef NLE_OBJECTS_GLOBAL
int bases[MAXOCLASSES];
/* Build-tool stubs — only used by makedefs/lev_comp linking. */
NEARDATA int doorindex = 0;
NEARDATA int in_doagain = 0;
NEARDATA boolean in_mklev = FALSE;
NEARDATA coord bhitpos = DUMMY;
NEARDATA char plname[PL_NSIZ] = DUMMY;
NEARDATA char pl_character[PL_CSIZ] = DUMMY;
NEARDATA char pl_race = '\0';
NEARDATA char pl_fruit[PL_FSIZ] = DUMMY;
NEARDATA char tune[6] = DUMMY;
/* Build-tool stubs. */
NEARDATA char dogname[PL_PSIZ] = DUMMY;
NEARDATA char catname[PL_PSIZ] = DUMMY;
NEARDATA char horsename[PL_PSIZ] = DUMMY;
NEARDATA char *save_cm = 0;
struct fruit;
NEARDATA struct fruit *ffruit = (struct fruit *) 0;
#endif
/* For libnethack, bases is a macro to nle_ctx_t. See decl.h. */

/* Nroom/nsubroom were NEARDATA __thread globals that the
 * per-step swap in nle.c copied in and out of the env's nle_ctx_t slot.
 * The struct field has existed since stage 3f (nle.h:136-137); this
 * migration drops the storage from decl.c and rewires decl.h (which is
 * the canonical extern point) to a per-env macro. The corresponding
 * swap in nle_swap_in/out (nle.c:825/845) is removed. Build-tool
 * binaries (makedefs, lev_comp) need the storage; they are now declared
 * under #ifdef NLE_OBJECTS_GLOBAL alongside other build-tool stubs. */
#ifdef NLE_OBJECTS_GLOBAL
NEARDATA int nroom = 0;
NEARDATA int nsubroom = 0;
#endif

/* maze limits must be even; masking off lowest bit guarantees that */
#ifdef NLE_OBJECTS_GLOBAL
int x_maze_max = (COLNO - 1) & ~1, y_maze_max = (ROWNO - 1) & ~1;
#endif


/* in_doagain — migrated to nle_ctx_t. */

/*
 *      The following structure will be initialized at startup time with
 *      the level numbers of some "important" things in the game.
 */
/* dungeon_topology migrated to nle_ctx_t (stage 6'). Allocated in init_nle. */

/* quest_status — stage 9' batch C migrated to nle_ctx_t. */

/* smeq — per-env room-equivalence work array, migrated to nle_ctx_t. */
/* doorindex — migrated to nle_ctx_t. */
/* save_cm — migrated to nle_ctx_t. */

/* killer — stage 9' batch C migrated to nle_ctx_t. */
#ifdef NLE_OBJECTS_GLOBAL
const char *nomovemsg = 0;
#endif
/* plname/pl_character/pl_race/pl_fruit/tune — migrated to nle_ctx_t. */
/* ffruit — migrated to nle_ctx_t. */

#ifdef NLE_OBJECTS_GLOBAL
const char *occtxt = DUMMY;
#endif
const char quitchars[] = " \r\n\033";
const char vowels[] = "aeiouAEIOU";
const char ynchars[] = "yn";
const char ynqchars[] = "ynq";
const char ynaqchars[] = "ynaq";
const char ynNaqchars[] = "yn#aq";

const char disclosure_options[] = "iavgco";

#if defined(MICRO) || defined(WIN32)
char hackdir[PATHLEN]; /* where rumors, help, record are */
#ifdef MICRO
char levels[PATHLEN]; /* where levels are */
#endif
#endif /* MICRO || WIN32 */

#ifdef MFLOPPY
char permbones[PATHLEN]; /* where permanent copy of bones go */
int ramdisk = FALSE;     /* whether to copy bones to levels or not */
int saveprompt = TRUE;
const char *alllevels = "levels.*";
const char *allbones = "bones*.*";
#endif

/* level_info — stage 7' partial migrated to nle_ctx_t (MAXLINFO entries). */

/* struct sinfo program_state migrated to nle_ctx_t (refactor stage 3b). */

/* x/y/z deltas for the 10 movement directions (8 compass pts, 2 up/down) */
const schar xdir[10] = { -1, -1, 0, 1, 1, 1, 0, -1, 0, 0 };
const schar ydir[10] = { 0, -1, -1, -1, 0, 1, 1, 1, 0, 0 };
const schar zdir[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, -1 };


/* for xname handling of multiple shot missile volleys:
   number of shots, index of current one, validity check, shoot vs throw */
/* m_shot — stage 9' batch C migrated to nle_ctx_t. Initial value
 * ({0,0,STRANGE_OBJECT,FALSE}) gets re-set in init_nle. */

/* dungeons[], sp_levchn, upstair, dnstair, upladder, dnladder, sstairs,
 * updest, dndest, inv_pos migrated to nle_ctx_t (stage 6').
 * All heap-allocated zero-init in init_nle; equivalent to the previous
 * { 0, 0, { 0, 0 }, 0 } / { 0, ... } / {0,0} static initializers. */

/* in_mklev — migrated to nle_ctx_t. */
/* weapon picked is merged with wielded one */

/* has_strong_rngseed migrated to nle_ctx_t (refactor stage 3a). */

/* bhitpos — migrated to nle_ctx_t. Allocated in init_nle. */
/* doors / rooms / subrooms / upstairs_room / dnstairs_room / sstairs_room
 * / ftrap — stage 7' partial migrated to nle_ctx_t (heap, see init_nle).
 * subrooms is initialized to point at rooms[MAXNROFROOMS+1] in init_nle
 * directly (no longer needs the per-startup subrooms_init() shim). */
/* level — stage 7' completed: per-env on nle_ctx_t (s7_level_p).
 * The blocker (struct dig_info.level token collision) was solved by
 * renaming that struct field to `dlvl`. */

void
subrooms_init(void)
{
    /* no-op retained: init_nle now points subrooms at rooms[MAXNROFROOMS+1]
     * directly. Kept so the existing call-site doesn't break. */
}
/* youmonst — stage 9' batch C migrated to nle_ctx_t. */
/* context — migrated to nle_ctx_t (per-game state). */
/* flags — migrated to nle_ctx_t.flags_ptr (per-env, heap).
 * The 12 struct-field `flags` collisions were renamed first so the
 * `#define flags (*current_nle_ctx->flags_ptr)` macro in flag.h is now
 * unambiguous. The storage formerly here was a process-global swapped on
 * every nle_step; that swap is retired in nle.c. */
#ifdef SYSFLAGS
/* sysflags — migrated to nle_ctx_t.sysflags_ptr. */
#endif
/* iflags — migrated to nle_ctx_t.iflags_ptr (per-env, heap). */
/* struct you u migrated to nle_ctx_t (stage 4). Heap-allocated in
 * init_nle, accessed via the `u` macro in decl.h. */
/* ubirthday migrated direct (stage 9' batch A). */
/* urealtime — stage 9' batch C migrated to nle_ctx_t. */

/* lastseentyp — stage 7' partial migrated to nle_ctx_t. */

/* Body-slot pointers — stage 9' batch D migrated to nle_ctx_t (s9_u*).
 * worn[] now uses offsetof-based resolution; no TLS address pinning.
 * Definitions retained here only for build-tool (NLE_OBJECTS_GLOBAL) builds. */
#ifdef NLE_OBJECTS_GLOBAL
NEARDATA struct obj
    *uwep = (struct obj *) 0, *uarm = (struct obj *) 0,
    *uswapwep = (struct obj *) 0,
    *uquiver = (struct obj *) 0,       /* quiver */
        *uarmu = (struct obj *) 0,     /* under-wear, so to speak */
                *uarmc = (struct obj *) 0, *uarmh = (struct obj *) 0,
    *uarms = (struct obj *) 0, *uarmg = (struct obj *) 0,
    *uarmf = (struct obj *) 0, *uamul = (struct obj *) 0,
    *uright = (struct obj *) 0, *uleft = (struct obj *) 0,
    *ublindf = (struct obj *) 0, *uchain = (struct obj *) 0,
    *uball = (struct obj *) 0;
#endif /* NLE_OBJECTS_GLOBAL */
/* invent, uskin, current_wand, thrownobj, kickedobj migrated to nle_ctx_t
 * (stage 9' batch A/B). */

#ifdef TEXTCOLOR
/*
 *  This must be the same order as used for buzz() in zap.c.
 *  (They're only used in mapglyph.c so probably shouldn't be here.)
 */
const int zapcolors[NUM_ZAP] = {
    HI_ZAP,     /* 0 - missile */
    CLR_ORANGE, /* 1 - fire */
    CLR_WHITE,  /* 2 - frost */
    HI_ZAP,     /* 3 - sleep */
    CLR_BLACK,  /* 4 - death */
    CLR_WHITE,  /* 5 - lightning */
    /* 3.6.3: poison gas zap used to be yellow and acid zap was green,
       which conflicted with the corresponding dragon colors */
    CLR_GREEN,  /* 6 - poison gas */
    CLR_YELLOW, /* 7 - acid */
};
#endif /* text color */

const int shield_static[SHIELD_COUNT] = {
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4, /* 7 per row */
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,
};

/* spl_book — stage 9' batch C migrated to nle_ctx_t (MAXSPELL+1 entries). */

/* moves/monstermoves/wailmsg migrated direct (stage 9' batch A).
 * Init to 1L,1L,0L deferred to init_nle / c_reset path. */

/* migrating_objs, billobjs migrated direct (stage 9' batch B). */

/* used to zero all elements of a struct obj and a struct monst */
/* zeroobj / zeromonst — read-only sentinels (DUMMY = {0}). Dropped
 * NEARDATA so they live as a single shared symbol. */
const struct obj zeroobj = DUMMY;
const struct monst zeromonst = DUMMY;
/* used to zero out union any; initializer deliberately omitted */
const anything zeroany;

/* originally from dog.c */
/* dogname/catname/horsename — migrated to nle_ctx_t. */
#ifdef NLE_OBJECTS_GLOBAL
char preferred_pet; /* '\0', 'c', 'd', 'n' (none) */
#endif
/* mydogs, migrating_mons, apelist migrated direct (stage 9' batch B). */
/* mvitals — stage 9' batch C migrated to nle_ctx_t (heap, NUMMONS entries). */
/* domove_attempting, domove_succeeded migrated direct (stage 9' batch A). */

/* c_color_names — read-only color-name table; drop NEARDATA. */
const struct c_color_names c_color_names = {
    "black",  "amber", "golden", "light blue", "red",   "green",
    "silver", "blue",  "purple", "white",      "orange"
};

struct menucoloring *menu_colorings = NULL;

const char *c_obj_colors[] = {
    "black",          /* CLR_BLACK */
    "red",            /* CLR_RED */
    "green",          /* CLR_GREEN */
    "brown",          /* CLR_BROWN */
    "blue",           /* CLR_BLUE */
    "magenta",        /* CLR_MAGENTA */
    "cyan",           /* CLR_CYAN */
    "gray",           /* CLR_GRAY */
    "transparent",    /* no_color */
    "orange",         /* CLR_ORANGE */
    "bright green",   /* CLR_BRIGHT_GREEN */
    "yellow",         /* CLR_YELLOW */
    "bright blue",    /* CLR_BRIGHT_BLUE */
    "bright magenta", /* CLR_BRIGHT_MAGENTA */
    "bright cyan",    /* CLR_BRIGHT_CYAN */
    "white",          /* CLR_WHITE */
};

struct c_common_strings c_common_strings = { "Nothing happens.",
                                             "That's enough tries!",
                                             "That is a silly thing to %s.",
                                             "shudder for a moment.",
                                             "something",
                                             "Something",
                                             "You can move again.",
                                             "Never mind.",
                                             "vision quickly clears.",
                                             { "the", "your" },
                                             { "mon", "you" } };

/* NOTE: the order of these words exactly corresponds to the
   order of oc_material values #define'd in objclass.h. */
const char *materialnm[] = { "mysterious", "liquid",  "wax",        "organic",
                             "flesh",      "paper",   "cloth",      "leather",
                             "wooden",     "bone",    "dragonhide", "iron",
                             "metal",      "copper",  "silver",     "gold",
                             "platinum",   "mithril", "plastic",    "glass",
                             "gemstone",   "stone" };

/* Vision — stage 8' migrated to nle_ctx_t (vision_full_recalc, viz_array). */

/* Global windowing data — stage 8' migrated to nle_ctx_t
 * (WIN_MESSAGE/STATUS/MAP/INVEN, toplines).
 * tc_gbl_data deferred (struct-tag self-reference). */
/* tc_gbl_data — stage 8' migrated to nle_ctx_t. Heap-alloc'd in init_nle. */

/* `fqn_prefix[]` migrated to current_nle_ctx->s_fqn_prefix.
 * Each env's table is zero-initialized when nle_ctx_t is calloc'd in
 * nle_start(); nle.c's main_loop_real() then populates each slot from
 * settings->hackdir (per-env) on the first step. */
#ifdef WIN32
boolean fqn_prefix_locked[PREFIX_COUNT] = { FALSE, FALSE, FALSE,
                                            FALSE, FALSE, FALSE,
                                            FALSE, FALSE, FALSE,
                                            FALSE };
#endif

#ifdef PREFIXES_IN_USE
const char *fqn_prefix_names[PREFIX_COUNT] = {
    "hackdir",  "leveldir", "savedir",    "bonesdir",  "datadir",
    "scoredir", "lockdir",  "sysconfdir", "configdir", "troubledir"
};
#endif

NEARDATA struct savefile_info sfcap = {
#ifdef NHSTDC
    0x00000000UL
#else
    0x00000000L
#endif
#if defined(COMPRESS) || defined(ZLIB_COMP)
        | SFI1_EXTERNALCOMP
#endif
#if defined(ZEROCOMP)
        | SFI1_ZEROCOMP
#endif
#if defined(RLECOMP)
        | SFI1_RLECOMP
#endif
    ,
#ifdef NHSTDC
    0x00000000UL, 0x00000000UL
#else
    0x00000000L, 0x00000000L
#endif
};

/* Sfrestinfo and sfsaveinfo migrated to nle_ctx_t (per-env).
 * Per-env init mirroring the original sfsaveinfo initializer happens in
 * init_nle (nle.c). sfrestinfo is calloc-zero-initialized like before. */

struct plinemsg_type *plinemsg_types = (struct plinemsg_type *) 0;

#ifdef PANICTRACE
const char *ARGV0;
#endif

/* support for lint.h */
unsigned nhUse_dummy = 0;

/* dummy routine used to force linkage */
void
decl_init()
{
    return;
}

/*decl.c*/
