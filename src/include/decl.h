/* NetHack 3.6  decl.h  $NHDT-Date: 1573869061 2019/11/16 01:51:01 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.165 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2007. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DECL_H
#define DECL_H

#define E extern

/* `struct sinfo program_state` was here in vanilla; migrated to
 * nle_ctx_t. Files that access current_nle_ctx->program_state.X need
 * to #include "nle.h" themselves (decl.h does NOT pull it in to avoid
 * forcing fcontext into util-binary include paths). */

#ifdef NLE_OBJECTS_GLOBAL
E int NDECL((*occupation));
E int NDECL((*afternmv));
#else
#define occupation (current_nle_ctx->occupation_v)
#define afternmv   (current_nle_ctx->afternmv_v)
#endif

E const char *hname;
/* `hackpid` migrated to nle_ctx_t (stage 3c). Callers use
 * current_nle_ctx->hackpid (and must #include "nle.h"). */
#if defined(UNIX) || defined(VMS)
#endif
#ifdef DEF_PAGER
E char *catmore;
#endif /* DEF_PAGER */

#ifdef NLE_PER_ENV_FILES
/* SAVEF, lock, fqn_prefix migrated to nle_ctx_t. Macros
 * below redirect the legacy identifier so existing call sites in
 * libnethack.so see the per-env field. Util binaries (lev_main,
 * recover, ...) build without NLE_PER_ENV_FILES and keep their own
 * local definitions. */
#include "nle.h"
#define SAVEF       (current_nle_ctx->s_SAVEF)
#else
E char SAVEF[];
#endif
#ifdef MICRO
E char SAVEP[];
#endif

/* Max size of a windowtype option */
#define WINTYPELEN 16
E char chosen_windowtype[WINTYPELEN];

#ifdef NLE_OBJECTS_GLOBAL
/* Build-tool view: regular writable global (matches objects[]). */
E int bases[MAXOCLASSES];
#else
/* Bases — per-env object-class base-index table migrated to nle_ctx_t. */
#define bases (current_nle_ctx->s_bases_p)
#endif

#ifdef NLE_OBJECTS_GLOBAL
E NEARDATA int nroom;
E NEARDATA int nsubroom;
#else
/* Nroom/nsubroom migrated to per-env nle_ctx_t. Drops the
 * nle_swap_in/out pair in nle.c and removes the cross-thread asymmetric-
 * write hazard (mklev.c wrote the __thread global; mkroom.c wrote
 * current_nle_ctx->nroom — readers couldn't tell which view was current
 * after an OMP yield mid-mklev). The ctx field was renamed from `nroom`
 * to `s_nroom` so the macro can route bare references through it without
 * the recursive expansion `current_nle_ctx->(current_nle_ctx->nroom)`. */
#define nroom    (current_nle_ctx->s_nroom)
#define nsubroom (current_nle_ctx->s_nsubroom)
#endif

#define WARNCOUNT 6 /* Number of different warning levels */
E nhsym warnsyms[WARNCOUNT];

#ifdef NLE_OBJECTS_GLOBAL
E int x_maze_max, y_maze_max;
#else
#define x_maze_max (current_nle_ctx->x_maze_max_v)
#define y_maze_max (current_nle_ctx->y_maze_max_v)
#endif

/* In_doagain — per-env, migrated to nle_ctx_t (cluster V). */
#ifdef NLE_OBJECTS_GLOBAL
E NEARDATA int in_doagain;
#else
#define in_doagain (current_nle_ctx->in_doagain_v)
#endif

struct dgn_topology { /* Special dungeon levels for speed */
    d_level d_oracle_level;
    d_level d_bigroom_level; /* Unused */
    d_level d_rogue_level;
    d_level d_medusa_level;
    d_level d_stronghold_level;
    d_level d_valley_level;
    d_level d_wiz1_level;
    d_level d_wiz2_level;
    d_level d_wiz3_level;
    d_level d_juiblex_level;
    d_level d_orcus_level;
    d_level d_baalzebub_level; /* Unused */
    d_level d_asmodeus_level;  /* Unused */
    d_level d_portal_level;    /* Only in goto_level() [do.c] */
    d_level d_sanctum_level;
    d_level d_earth_level;
    d_level d_water_level;
    d_level d_fire_level;
    d_level d_air_level;
    d_level d_astral_level;
    xchar d_tower_dnum;
    xchar d_sokoban_dnum;
    xchar d_mines_dnum, d_quest_dnum;
    d_level d_qstart_level, d_qlocate_level, d_nemesis_level;
    d_level d_knox_level;
    d_level d_mineend_level;
    d_level d_sokoend_level;
};
/* Dungeon_topology migrated to nle_ctx_t (stage 6'). */
#define dungeon_topology (*current_nle_ctx->s6_topology_p)
/* Macros for accessing the dungeon levels by their old names */
/* Clang-format off */
#define oracle_level            (dungeon_topology.d_oracle_level)
#define bigroom_level           (dungeon_topology.d_bigroom_level)
#define rogue_level             (dungeon_topology.d_rogue_level)
#define medusa_level            (dungeon_topology.d_medusa_level)
#define stronghold_level        (dungeon_topology.d_stronghold_level)
#define valley_level            (dungeon_topology.d_valley_level)
#define wiz1_level              (dungeon_topology.d_wiz1_level)
#define wiz2_level              (dungeon_topology.d_wiz2_level)
#define wiz3_level              (dungeon_topology.d_wiz3_level)
#define juiblex_level           (dungeon_topology.d_juiblex_level)
#define orcus_level             (dungeon_topology.d_orcus_level)
#define baalzebub_level         (dungeon_topology.d_baalzebub_level)
#define asmodeus_level          (dungeon_topology.d_asmodeus_level)
#define portal_level            (dungeon_topology.d_portal_level)
#define sanctum_level           (dungeon_topology.d_sanctum_level)
#define earth_level             (dungeon_topology.d_earth_level)
#define water_level             (dungeon_topology.d_water_level)
#define fire_level              (dungeon_topology.d_fire_level)
#define air_level               (dungeon_topology.d_air_level)
#define astral_level            (dungeon_topology.d_astral_level)
#define tower_dnum              (dungeon_topology.d_tower_dnum)
#define sokoban_dnum            (dungeon_topology.d_sokoban_dnum)
#define mines_dnum              (dungeon_topology.d_mines_dnum)
#define quest_dnum              (dungeon_topology.d_quest_dnum)
#define qstart_level            (dungeon_topology.d_qstart_level)
#define qlocate_level           (dungeon_topology.d_qlocate_level)
#define nemesis_level           (dungeon_topology.d_nemesis_level)
#define knox_level              (dungeon_topology.d_knox_level)
#define mineend_level           (dungeon_topology.d_mineend_level)
#define sokoend_level           (dungeon_topology.d_sokoend_level)
/* Clang-format on */

/* Dnstair/upstair/dnladder/upladder/sstairs migrated to nle_ctx_t (stage 6'). */
#define dnstair  (*current_nle_ctx->s6_dnstair_p)
#define upstair  (*current_nle_ctx->s6_upstair_p)
#define xdnstair (dnstair.sx)
#define ydnstair (dnstair.sy)
#define xupstair (upstair.sx)
#define yupstair (upstair.sy)

#define dnladder (*current_nle_ctx->s6_dnladder_p)
#define upladder (*current_nle_ctx->s6_upladder_p)
#define xdnladder (dnladder.sx)
#define ydnladder (dnladder.sy)
#define xupladder (upladder.sx)
#define yupladder (upladder.sy)

#define sstairs  (*current_nle_ctx->s6_sstairs_p)

/* Updest/dndest migrated to nle_ctx_t (stage 6'). */
#define updest (*current_nle_ctx->s6_updest_p)
#define dndest (*current_nle_ctx->s6_dndest_p)

/* Inv_pos / dungeons / sp_levchn migrated to nle_ctx_t (stage 6'). */
#define inv_pos   (*current_nle_ctx->s6_inv_pos_p)
#define dungeons  (current_nle_ctx->s6_dungeons_p)
#define sp_levchn (current_nle_ctx->s6_sp_levchn)
#define dunlev_reached(x) (dungeons[(x)->dnum].dunlev_ureached)

#include "quest.h"
/* Quest_status — stage 9' batch C migrated to nle_ctx_t. */
#define quest_status (*current_nle_ctx->s9c_quest_status_p)

/* Pl_character/pl_race/pl_fruit/tune — per-env, migrated to nle_ctx_t
 * (cluster W). */
#ifdef NLE_OBJECTS_GLOBAL
E NEARDATA char pl_character[PL_CSIZ];
E NEARDATA char pl_race;
E NEARDATA char pl_fruit[PL_FSIZ];
E NEARDATA char tune[6];
#else
#define pl_character (current_nle_ctx->pl_character_v)
#define pl_race      (current_nle_ctx->pl_race_v)
#define pl_fruit     (current_nle_ctx->pl_fruit_v)
#define tune         (current_nle_ctx->tune_v)
#endif

/* Ffruit — per-env fruit list head, migrated to nle_ctx_t (cluster X). */
#ifdef NLE_OBJECTS_GLOBAL
E NEARDATA struct fruit *ffruit;
#else
#define ffruit (current_nle_ctx->ffruit_v)
#endif

#define MAXLINFO (MAXDUNGEON * MAXLEVEL)
/* Level_info — stage 7' partial migrated to nle_ctx_t. */
#define level_info (current_nle_ctx->s7_level_info_p)

/* `struct sinfo program_state` moved to nle_ctx_t (refactor stage 3b).
 * Definition of struct sinfo is now in nle.h. Callers use
 * current_nle_ctx->program_state.X for per-instance access. */


E const char quitchars[];
E const char vowels[];
E const char ynchars[];
E const char ynqchars[];
E const char ynaqchars[];
E const char ynNaqchars[];

E const char disclosure_options[];

/* Smeq[] — per-env room-equivalence work array migrated to nle_ctx_t. */
#define smeq (current_nle_ctx->s_smeq_p)
/* Doorindex — per-env, migrated to nle_ctx_t (cluster V). */
#ifdef NLE_OBJECTS_GLOBAL
E NEARDATA int doorindex;
#else
#define doorindex (current_nle_ctx->doorindex_v)
#endif
/* Save_cm — per-env, migrated to nle_ctx_t (cluster X). */
#ifdef NLE_OBJECTS_GLOBAL
E NEARDATA char *save_cm;
#else
#define save_cm (current_nle_ctx->save_cm_v)
#endif

struct kinfo {
    struct kinfo *next; /* Chain of delayed killers */
    int id;             /* Uprop keys to ID a delayed killer */
    int format;         /* One of the killer formats */
#define KILLED_BY_AN 0
#define KILLED_BY 1
#define NO_KILLER_PREFIX 2
    char name[BUFSZ]; /* Actual killer name */
};
/* Killer — stage 9' batch C migrated to nle_ctx_t. The struct
 * u_conduct.killer field was renamed to u_conduct.killcount in you.h
 * to free the `killer` token for this macro. */
#define killer (*current_nle_ctx->s9c_killer_p)

/* Plname — per-env player name, migrated to nle_ctx_t (cluster W). */
#ifdef NLE_OBJECTS_GLOBAL
E NEARDATA char plname[PL_NSIZ];
#else
#define plname (current_nle_ctx->plname_v)
#endif
/* Dogname/catname/horsename — per-env pet names, migrated to nle_ctx_t
 * (cluster X). */
#ifdef NLE_OBJECTS_GLOBAL
E NEARDATA char dogname[];
E NEARDATA char catname[];
E NEARDATA char horsename[];
#else
#define dogname   (current_nle_ctx->dogname_v)
#define catname   (current_nle_ctx->catname_v)
#define horsename (current_nle_ctx->horsename_v)
#endif
#ifdef NLE_OBJECTS_GLOBAL
E char preferred_pet;
#else
#define preferred_pet (current_nle_ctx->preferred_pet_v)
#endif
#ifdef NLE_OBJECTS_GLOBAL
E const char *occtxt;
#else
#define occtxt (current_nle_ctx->occtxt_v)
#endif
#ifdef NLE_OBJECTS_GLOBAL
E const char *nomovemsg;
#else
#define nomovemsg (current_nle_ctx->nomovemsg_v)
#endif
#ifdef NLE_PER_ENV_FILES
/* `lock` redirects to current_nle_ctx->s_lock. See nle.h
 * for the per-env field and the rationale (cross-env level-file clash
 * from a process-shared lock buffer). */
#define lock        (current_nle_ctx->s_lock)
#else
E char lock[];
#endif

E const schar xdir[], ydir[], zdir[];


struct multishot {
    int n, i;
    short o;
    boolean s;
};
/* M_shot — stage 9' batch C migrated to nle_ctx_t. */
#define m_shot (*current_nle_ctx->s9c_m_shot_p)

/* Moves/monstermoves/wailmsg — stage 9' migrated to nle_ctx_t. */
#define moves        (current_nle_ctx->nle_moves)
#define monstermoves (current_nle_ctx->nle_monstermoves)
#define wailmsg      (current_nle_ctx->nle_wailmsg)

/* In_mklev — per-env, migrated to nle_ctx_t (cluster V). */
#ifdef NLE_OBJECTS_GLOBAL
E NEARDATA boolean in_mklev;
#else
#define in_mklev (current_nle_ctx->in_mklev_v)
#endif
/* `in_steed_dismounting` migrated to nle_ctx_t (stage 3d) */

/* `has_strong_rngseed` migrated into nle_ctx_t (refactor stage 3a, NLE
 * subsystem). Callers now use current_nle_ctx->has_strong_rngseed. */

E const int shield_static[];

#include "spell.h"
/* Spl_book — stage 9' batch C migrated to nle_ctx_t. */
#define spl_book (current_nle_ctx->s9c_spl_book_p)

#include "color.h"
#ifdef TEXTCOLOR
E const int zapcolors[];
#endif

E const struct class_sym def_oc_syms[MAXOCLASSES]; /* Default class symbols */
E uchar oc_syms[MAXOCLASSES];                      /* Current class symbols */
E const struct class_sym def_monsyms[MAXMCLASSES]; /* Default class symbols */
E uchar monsyms[MAXMCLASSES];                      /* Current class symbols */

#include "obj.h"
/* Body-slot pointers — stage 9' batch D migrated to nle_ctx_t.
 * worn[] now uses byte-offset resolution (worn.c worn_init) so these
 * no longer need to be TLS globals with stable addresses. */
#ifdef NLE_OBJECTS_GLOBAL
/* Build-tool view: regular writable globals (lev_comp, makedefs, etc.) */
E NEARDATA struct obj *uarm, *uarmc, *uarmh, *uarms, *uarmg, *uarmf,
    *uarmu, /* Under-wear, so to speak */
    *uamul, *uleft, *uright, *ublindf, *uwep, *uswapwep, *uquiver;
E NEARDATA struct obj *uchain; /* Defined only when punished */
E NEARDATA struct obj *uball;
#else
#define uarm     (current_nle_ctx->s9_uarm)
#define uarmc    (current_nle_ctx->s9_uarmc)
#define uarmh    (current_nle_ctx->s9_uarmh)
#define uarms    (current_nle_ctx->s9_uarms)
#define uarmg    (current_nle_ctx->s9_uarmg)
#define uarmf    (current_nle_ctx->s9_uarmf)
#define uarmu    (current_nle_ctx->s9_uarmu)
#define uamul    (current_nle_ctx->s9_uamul)
#define uleft    (current_nle_ctx->s9_uleft)
#define uright   (current_nle_ctx->s9_uright)
#define ublindf  (current_nle_ctx->s9_ublindf)
#define uwep     (current_nle_ctx->s9_uwep)
#define uswapwep (current_nle_ctx->s9_uswapwep)
#define uquiver  (current_nle_ctx->s9_uquiver)
#define uchain   (current_nle_ctx->s9_uchain)  /* Defined only when punished */
#define uball    (current_nle_ctx->s9_uball)
#endif
/* Invent / uskin / current_wand / thrownobj / kickedobj / migrating_objs /
 * billobjs migrated direct (no static-init refs to address-of). */
#define invent         (current_nle_ctx->invent_p)
#define uskin          (current_nle_ctx->uskin_p)
#define current_wand   (current_nle_ctx->current_wand_p)
#define thrownobj      (current_nle_ctx->thrownobj_p)
#define kickedobj      (current_nle_ctx->kickedobj_p)
#define migrating_objs (current_nle_ctx->migrating_objs_p)
#define billobjs       (current_nle_ctx->billobjs_p)

E const struct obj zeroobj; /* For init; also, &zeroobj is used
                                      * as special value */

E const anything zeroany;   /* Init'd and defined in decl.c */

#include "you.h"
/* Player state migrated to nle_ctx_t (stage 4 — was 'struct you u' here).
 * Macro form keeps NetHack code using `u.field` unchanged; expansion is
 * (*current_nle_ctx->u_ptr).field. Note: `nle.h` is included for the
 * extern declaration of `current_nle_ctx`; this is already the case in
 * every src/*.c via the refactor's universal include pattern. */
#define u (*current_nle_ctx->u_ptr)
/* Ubirthday — stage 9' migrated to nle_ctx_t. */
#define ubirthday (current_nle_ctx->nle_ubirthday)
/* Urealtime — stage 9' batch C migrated to nle_ctx_t. */
#define urealtime (*current_nle_ctx->s9c_urealtime_p)

#include "onames.h"
#ifndef PM_H /* (pm.h has already been included via youprop.h) */
#include "pm.h"
#endif

E const struct monst zeromonst; /* For init of new or temp monsters */
/* Youmonst — stage 9' batch C migrated to nle_ctx_t. */
#define youmonst (*current_nle_ctx->s9c_youmonst_p)
/* Mydogs / migrating_mons — stage 9' migrated to nle_ctx_t. */
#define mydogs         (current_nle_ctx->mydogs_p)
#define migrating_mons (current_nle_ctx->migrating_mons_p)

/* The struct tag was 'mvitals' upstream — renamed to nle_mvitals_t to
 * free the `mvitals` token for the macro below (preprocessor would
 * otherwise rewrite the tag too). */
struct nle_mvitals_t {
    uchar born;
    uchar died;
    uchar mvflags;
};
/* Mvitals — stage 9' batch C migrated to nle_ctx_t (array of NUMMONS). */
#define mvitals (current_nle_ctx->s9c_mvitals_p)

/* Domove_attempting / domove_succeeded — stage 9' migrated to nle_ctx_t. */
#define domove_attempting (current_nle_ctx->nle_domove_attempting)
#define domove_succeeded  (current_nle_ctx->nle_domove_succeeded)
#define DOMOVE_WALK         0x00000001
#define DOMOVE_RUSH         0x00000002

/* C_color_names — read-only color-name table; non-TLS, shared. */
struct c_color_names {
    const char *const c_black, *const c_amber, *const c_golden,
        *const c_light_blue, *const c_red, *const c_green, *const c_silver,
        *const c_blue, *const c_purple, *const c_white, *const c_orange;
};
E const struct c_color_names c_color_names;
#define NH_BLACK c_color_names.c_black
#define NH_AMBER c_color_names.c_amber
#define NH_GOLDEN c_color_names.c_golden
#define NH_LIGHT_BLUE c_color_names.c_light_blue
#define NH_RED c_color_names.c_red
#define NH_GREEN c_color_names.c_green
#define NH_SILVER c_color_names.c_silver
#define NH_BLUE c_color_names.c_blue
#define NH_PURPLE c_color_names.c_purple
#define NH_WHITE c_color_names.c_white
#define NH_ORANGE c_color_names.c_orange

/* The names of the colors used for gems, etc. */
E const char *c_obj_colors[];

E struct c_common_strings {
    const char *const c_nothing_happens, *const c_thats_enough_tries,
        *const c_silly_thing_to, *const c_shudder_for_moment,
        *const c_something, *const c_Something, *const c_You_can_move_again,
        *const c_Never_mind, *c_vision_clears, *const c_the_your[2],
        *const c_fakename[2];
} c_common_strings;
#define nothing_happens c_common_strings.c_nothing_happens
#define thats_enough_tries c_common_strings.c_thats_enough_tries
#define silly_thing_to c_common_strings.c_silly_thing_to
#define shudder_for_moment c_common_strings.c_shudder_for_moment
#define something c_common_strings.c_something
#define Something c_common_strings.c_Something
#define You_can_move_again c_common_strings.c_You_can_move_again
#define Never_mind c_common_strings.c_Never_mind
#define vision_clears c_common_strings.c_vision_clears
#define the_your c_common_strings.c_the_your
/* Fakename[] used occasionally so vtense() won't be fooled by an assigned
   name ending in 's' */
#define fakename c_common_strings.c_fakename

/* Material strings */
E const char *materialnm[];

/* Monster name articles */
#define ARTICLE_NONE 0
#define ARTICLE_THE 1
#define ARTICLE_A 2
#define ARTICLE_YOUR 3

/* Monster name suppress masks */
#define SUPPRESS_IT 0x01
#define SUPPRESS_INVISIBLE 0x02
#define SUPPRESS_HALLUCINATION 0x04
#define SUPPRESS_SADDLE 0x08
#define EXACT_NAME 0x0F
#define SUPPRESS_NAME 0x10

/* Vision — stage 8' migrated to nle_ctx_t (macros in vision.h). */

/* Window system stuff — stage 8' migrated to nle_ctx_t. */
#define WIN_MESSAGE (current_nle_ctx->win_message)
#define WIN_STATUS  (current_nle_ctx->win_status)
#define WIN_MAP     (current_nle_ctx->win_map)
#define WIN_INVEN   (current_nle_ctx->win_inven)

/* Pline (et al) for a single string argument (suppress compiler warning) */
#define pline1(cstr) pline("%s", cstr)
#define Your1(cstr) Your("%s", cstr)
#define You1(cstr) You("%s", cstr)
#define verbalize1(cstr) verbalize("%s", cstr)
#define You_hear1(cstr) You_hear("%s", cstr)
#define Sprintf1(buf, cstr) Sprintf(buf, "%s", cstr)
#define panic1(cstr) panic("%s", cstr)

/* Toplines — stage 8' migrated to nle_ctx_t (macro). */
#define toplines (current_nle_ctx->top_lines)
#ifndef TCAP_H
/* Tc_gbl_data — stage 8' migrated to nle_ctx_t. Struct tag renamed
 * to nle_tcap_t to free the `tc_gbl_data` token for the macro. */
struct nle_tcap_t {
    char *tc_AS, *tc_AE; /* Graphics start and end (tty font swapping) */
    int tc_LI, tc_CO;    /* Lines and columns */
};
#define tc_gbl_data (*current_nle_ctx->s8_tcap_p)
#define AS tc_gbl_data.tc_AS
#define AE tc_gbl_data.tc_AE
#define LI tc_gbl_data.tc_LI
#define CO tc_gbl_data.tc_CO
#endif

/* Xxxexplain[] is in drawing.c */
E const char *const monexplain[], invisexplain[], *const oclass_names[];

/* Some systems want to use full pathnames for some subsets of file names,
 * rather than assuming that they're all in the current directory.  This
 * provides all the subclasses that seem reasonable, and sets up for all
 * prefixes being null.  Port code can set those that it wants.
 */
#define HACKPREFIX 0
#define LEVELPREFIX 1
#define SAVEPREFIX 2
#define BONESPREFIX 3
#define DATAPREFIX 4 /* This one must match hardcoded value in dlb.c */
#define SCOREPREFIX 5
#define LOCKPREFIX 6
#define SYSCONFPREFIX 7
#define CONFIGPREFIX 8
#define TROUBLEPREFIX 9
#define PREFIX_COUNT 10
/* Used in files.c; xxconf.h can override if needed */
#ifndef FQN_MAX_FILENAME
#define FQN_MAX_FILENAME 512
#endif

#if defined(NOCWD_ASSUMPTIONS) || defined(VAR_PLAYGROUND)
/* The bare-bones stuff is unconditional above to simplify coding; for
 * ports that actually use prefixes, add some more localized things
 */
#define PREFIXES_IN_USE
#endif

#ifdef NLE_PER_ENV_FILES
/* `fqn_prefix` redirects to current_nle_ctx->s_fqn_prefix.
 * Each env now keeps its own pointer table; the strings themselves
 * live in nle->settings.hackdir (which is already per-env). */
#define fqn_prefix  (current_nle_ctx->s_fqn_prefix)
#else
E char *fqn_prefix[PREFIX_COUNT];
#endif
#ifdef WIN32
E boolean fqn_prefix_locked[PREFIX_COUNT];
#endif
#ifdef PREFIXES_IN_USE
E const char *fqn_prefix_names[PREFIX_COUNT];
#endif

/* Sfrestinfo and sfsaveinfo migrated to nle_ctx_t (per-env).
 * sfcap is const-after-init (build capabilities) and remains shared. */
E NEARDATA struct savefile_info sfcap;

struct opvar {
    xchar spovartyp; /* One of SPOVAR_foo */
    union {
        char *str;
        long l;
    } vardata;
};

struct autopickup_exception {
    struct nhregex *regex;
    char *pattern;
    boolean grab;
    struct autopickup_exception *next;
};
/* Apelist — stage 9' migrated to nle_ctx_t. */
#define apelist (current_nle_ctx->apelist_p)

struct plinemsg_type {
    xchar msgtype;  /* One of MSGTYP_foo */
    struct nhregex *regex;
    char *pattern;
    struct plinemsg_type *next;
};

#define MSGTYP_NORMAL   0
#define MSGTYP_NOREP    1
#define MSGTYP_NOSHOW   2
#define MSGTYP_STOP     3
/* Bitmask for callers of hide_unhide_msgtypes() */
#define MSGTYP_MASK_REP_SHOW ((1 << MSGTYP_NOREP) | (1 << MSGTYP_NOSHOW))

E struct plinemsg_type *plinemsg_types;

enum bcargs {override_restriction = -1};
struct breadcrumbs {
    const char *funcnm;
    int linenum;
    boolean in_effect;
};

#ifdef PANICTRACE
E const char *ARGV0;
#endif

enum earlyarg {ARG_DEBUG, ARG_VERSION, ARG_SHOWPATHS
#ifdef WIN32
    ,ARG_WINDOWS
#endif
};

struct early_opt {
    enum earlyarg e;
    const char *name;
    int minlength;
    boolean valallowed;
};

#undef E

#endif /* DECL_H */
