/* NetHack 3.6	decl.c	$NHDT-Date: 1573869062 2019/11/16 01:51:02 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.149 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2009. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* afternmv: per-env, see nh_globals.h */
/* occupation: per-env, see nh_globals.h */

/* from xxxmain.c */
/* hname: per-env, see nh_globals.h */ /* name of the game (argv[0] of main) */
/* hackpid: per-env, see nh_globals.h */       /* current process id */
#if defined(UNIX) || defined(VMS)
/* locknum: per-env, see nh_globals.h */ /* max num of simultaneous users */
#endif
#ifdef DEF_PAGER
char *catmore = 0; /* default pager */
#endif
/* chosen_windowtype: per-env, see nh_globals.h */

/* bases: per-env, see nh_globals.h */

/* multi: per-env, see nh_globals.h */
/* multi_reason: per-env, see nh_globals.h */
/* nroom: per-env nh_g->nroom */
/* nsubroom: per-env, see nh_globals.h */
/* occtime: per-env, see nh_globals.h */

/* maze limits must be even; masking off lowest bit guarantees that */
/* x_maze_max: per-env, see nh_globals.h */
const int nh_tmpl_x_maze_max = (COLNO - 1) & ~1;
/* y_maze_max: per-env, see nh_globals.h */
const int nh_tmpl_y_maze_max = (ROWNO - 1) & ~1;

/* otg_temp: per-env, see nh_globals.h */ /* used by object_to_glyph() [otg] */

/* in_doagain: per-env, see nh_globals.h */

/*
 *      The following structure will be initialized at startup time with
 *      the level numbers of some "important" things in the game.
 */
/* dungeon_topology: per-env, see nh_globals.h */
const struct dgn_topology nh_tmpl_dungeon_topology =
{ DUMMY };

/* quest_status: per-env, see nh_globals.h */

/* warn_obj_cnt: per-env, see nh_globals.h */
/* smeq: per-env, see nh_globals.h */
/* doorindex: per-env, see nh_globals.h */
/* save_cm: per-env, see nh_globals.h */

/* killer: per-env nh_g->killer */
/* done_money: per-env, see nh_globals.h */
/* nomovemsg: per-env, see nh_globals.h */
/* plname: per-env, see nh_globals.h */ /* player name */
/* pl_character: per-env, see nh_globals.h */
/* pl_race: per-env, see nh_globals.h */

/* pl_fruit: per-env, see nh_globals.h */
/* ffruit: per-env, see nh_globals.h */

/* tune: per-env, see nh_globals.h */
/* ransacked: per-env, see nh_globals.h */

/* occtxt: per-env, see nh_globals.h */
const char quitchars[] = " \r\n\033";
const char vowels[] = "aeiouAEIOU";
const char ynchars[] = "yn";
const char ynqchars[] = "ynq";
const char ynaqchars[] = "ynaq";
const char ynNaqchars[] = "yn#aq";
/* yn_number: per-env, see nh_globals.h */

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

/* level_info: per-env, see nh_globals.h */

/* program_state: per-env nh_g->program_state */

/* x/y/z deltas for the 10 movement directions (8 compass pts, 2 up/down) */
const schar xdir[10] = { -1, -1, 0, 1, 1, 1, 0, -1, 0, 0 };
const schar ydir[10] = { 0, -1, -1, -1, 0, 1, 1, 1, 0, 0 };
const schar zdir[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, -1 };

/* tbx: per-env, see nh_globals.h */
/* tby: per-env, see nh_globals.h */ /* mthrowu: target */

/* for xname handling of multiple shot missile volleys:
   number of shots, index of current one, validity check, shoot vs throw */
/* m_shot: per-env, see nh_globals.h */
const struct multishot nh_tmpl_m_shot =
{ 0, 0, STRANGE_OBJECT, FALSE };

/* dungeons: per-env, see nh_globals.h */ /* ini'ed by init_dungeon() */
/* sp_levchn: per-env, see nh_globals.h */
/* upstair: per-env, see nh_globals.h */
const stairway nh_tmpl_upstair = { 0, 0, { 0, 0 }, 0 };
/* dnstair: per-env, see nh_globals.h */
const stairway nh_tmpl_dnstair = { 0, 0, { 0, 0 }, 0 };
/* upladder: per-env, see nh_globals.h */
const stairway nh_tmpl_upladder = { 0, 0, { 0, 0 }, 0 };
/* dnladder: per-env, see nh_globals.h */
const stairway nh_tmpl_dnladder = { 0, 0, { 0, 0 }, 0 };
/* sstairs: per-env, see nh_globals.h */
const stairway nh_tmpl_sstairs =
{ 0, 0, { 0, 0 }, 0 };
/* updest: per-env, see nh_globals.h */
const dest_area nh_tmpl_updest =
{ 0, 0, 0, 0, 0, 0, 0, 0 };
/* dndest: per-env, see nh_globals.h */
const dest_area nh_tmpl_dndest =
{ 0, 0, 0, 0, 0, 0, 0, 0 };
/* inv_pos: per-env, see nh_globals.h */
const coord nh_tmpl_inv_pos =
{ 0, 0 };

/* defer_see_monsters: per-env, see nh_globals.h */
/* in_mklev: per-env, see nh_globals.h */
/* stoned: per-env, see nh_globals.h */ /* done to monsters hit by 'c' */
/* unweapon: per-env, see nh_globals.h */
/* mrg_to_wielded: per-env, see nh_globals.h */
/* weapon picked is merged with wielded one */

/* in_steed_dismounting: per-env, see nh_globals.h */
/* has_strong_rngseed: per-env, see nh_globals.h */

/* bhitpos: per-env, see nh_globals.h */
/* doors: per-env, see nh_globals.h */
const coord nh_tmpl_doors[DOORMAX] =
{ DUMMY };

/* rooms: per-env, see nh_globals.h */
const struct mkroom nh_tmpl_rooms[(MAXNROFROOMS+1)*2] =
{ DUMMY };
/* subrooms: per-env, see nh_globals.h */
/* upstairs_room: per-env, see nh_globals.h */
/* dnstairs_room: per-env, see nh_globals.h */
/* sstairs_room: per-env, see nh_globals.h */

/* level: per-env nh_g->level */ /* level map */
/* ftrap: per-env, see nh_globals.h */
/* youmonst: per-env, see nh_globals.h */
/* context: per-env, see nh_globals.h */
/* flags: per-env nh_g->flags */
#ifdef SYSFLAGS
NEARDATA struct sysflag sysflags = DUMMY;
#endif
/* iflags: per-env, see nh_globals.h */
/* u: per-env, see nh_globals.h */
/* ubirthday: per-env, see nh_globals.h */
/* urealtime: per-env, see nh_globals.h */

/* lastseentyp: per-env, see nh_globals.h */
const schar nh_tmpl_lastseentyp[COLNO][ROWNO] =
{
    DUMMY
}; /* last seen/touched dungeon typ */

/* invent: per-env, see nh_globals.h */
/* uwep: per-env, see nh_globals.h */
/* uarm: per-env, see nh_globals.h */
/* uswapwep: per-env, see nh_globals.h */
/* uquiver: per-env, see nh_globals.h */
/* uarmu: per-env, see nh_globals.h */
/* uskin: per-env, see nh_globals.h */
/* uarmc: per-env, see nh_globals.h */
/* uarmh: per-env, see nh_globals.h */
/* uarms: per-env, see nh_globals.h */
/* uarmg: per-env, see nh_globals.h */
/* uarmf: per-env, see nh_globals.h */
/* uamul: per-env, see nh_globals.h */
/* uright: per-env, see nh_globals.h */
/* uleft: per-env, see nh_globals.h */
/* ublindf: per-env, see nh_globals.h */
/* uchain: per-env, see nh_globals.h */
/* uball: per-env, see nh_globals.h */
/* some objects need special handling during destruction or placement */
/* current_wand: per-env, see nh_globals.h */
/* thrownobj: per-env, see nh_globals.h */
/* kickedobj: per-env, see nh_globals.h */     /* object in flight due to kicking */

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

/* spl_book: per-env, see nh_globals.h */
const struct spell nh_tmpl_spl_book[MAXSPELL+1] =
{ DUMMY };

/* moves: per-env, see nh_globals.h */
const long nh_tmpl_moves = 1L;
/* monstermoves: per-env, see nh_globals.h */
const long nh_tmpl_monstermoves = 1L;
/* These diverge when player is Fast */
/* wailmsg: per-env, see nh_globals.h */

/* objects that are moving to another dungeon level */
/* migrating_objs: per-env, see nh_globals.h */
/* objects not yet paid for */
/* billobjs: per-env, see nh_globals.h */

/* used to zero all elements of a struct obj and a struct monst */
NEARDATA const struct obj zeroobj = DUMMY;
NEARDATA const struct monst zeromonst = DUMMY;
/* used to zero out union any; initializer deliberately omitted */
NEARDATA const anything zeroany;

/* originally from dog.c */
/* dogname: per-env, see nh_globals.h */
/* catname: per-env, see nh_globals.h */
/* horsename: per-env, see nh_globals.h */
/* preferred_pet: per-env, see nh_globals.h */ /* '\0', 'c', 'd', 'n' (none) */
/* monsters that went down/up together with @ */
/* mydogs: per-env, see nh_globals.h */
/* monsters that are moving to another dungeon level */
/* migrating_mons: per-env, see nh_globals.h */
/* apelist: per-env, see nh_globals.h */

/* mvitals: per-env nh_g->mvitals */
/* domove_attempting: per-env, see nh_globals.h */
/* domove_succeeded: per-env, see nh_globals.h */

/* c_color_names: per-env nh_g->c_color_names */
const struct c_color_names nh_tmpl_c_color_names =
{
    "black",  "amber", "golden", "light blue", "red",   "green",
    "silver", "blue",  "purple", "white",      "orange"
};

/* menu_colorings: per-env, see nh_globals.h */

/* c_obj_colors: per-env, see nh_globals.h */
const char *const nh_tmpl_c_obj_colors[] =
{
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

/* c_common_strings: per-env nh_g->c_common_strings */
const struct c_common_strings nh_tmpl_c_common_strings =
{ "Nothing happens.",
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
/* materialnm: per-env, see nh_globals.h */
const char *const nh_tmpl_materialnm[] =
{ "mysterious", "liquid",  "wax",        "organic",
                             "flesh",      "paper",   "cloth",      "leather",
                             "wooden",     "bone",    "dragonhide", "iron",
                             "metal",      "copper",  "silver",     "gold",
                             "platinum",   "mithril", "plastic",    "glass",
                             "gemstone",   "stone" };

/* Vision */
/* vision_full_recalc: per-env, see nh_globals.h */
/* viz_array: per-env, see nh_globals.h */ /* used in cansee() and couldsee() macros */

/* Global windowing data, defined here for multi-window-system support */
/* WIN_MESSAGE: per-env, see nh_globals.h */
const winid nh_tmpl_WIN_MESSAGE =
WIN_ERR;
/* WIN_STATUS: per-env, see nh_globals.h */
const winid nh_tmpl_WIN_STATUS =
WIN_ERR;
/* WIN_MAP: per-env, see nh_globals.h */
const winid nh_tmpl_WIN_MAP = WIN_ERR;
/* WIN_INVEN: per-env, see nh_globals.h */
const winid nh_tmpl_WIN_INVEN = WIN_ERR;
/* toplines: per-env, see nh_globals.h */
/* Windowing stuff that's really tty oriented, but present for all ports */
/* tc_gbl_data: per-env nh_g->tc_gbl_data */
const struct tc_gbl_data nh_tmpl_tc_gbl_data =
{ 0, 0, 0, 0 }; /* AS,AE, LI,CO */

/* fqn_prefix: per-env, see nh_globals.h */
char *const nh_tmpl_fqn_prefix[PREFIX_COUNT] =
{ (char *) 0, (char *) 0, (char *) 0,
                                   (char *) 0, (char *) 0, (char *) 0,
                                   (char *) 0, (char *) 0, (char *) 0,
                                   (char *) 0 };
#ifdef WIN32
boolean fqn_prefix_locked[PREFIX_COUNT] = { FALSE, FALSE, FALSE,
                                            FALSE, FALSE, FALSE,
                                            FALSE, FALSE, FALSE,
                                            FALSE };
#endif

#ifdef PREFIXES_IN_USE
/* fqn_prefix_names: per-env, see nh_globals.h */
const char *const nh_tmpl_fqn_prefix_names[PREFIX_COUNT] =
{
    "hackdir",  "leveldir", "savedir",    "bonesdir",  "datadir",
    "scoredir", "lockdir",  "sysconfdir", "configdir", "troubledir"
};
#endif

/* sfcap: per-env, see nh_globals.h */
const struct savefile_info nh_tmpl_sfcap =
{
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

/* sfrestinfo: per-env, see nh_globals.h */
/* sfsaveinfo: per-env, see nh_globals.h */
const struct savefile_info nh_tmpl_sfsaveinfo = {
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

/* plinemsg_types: per-env, see nh_globals.h */

#ifdef PANICTRACE
/* ARGV0: per-env, see nh_globals.h */
#endif

/* support for lint.h */
/* nhUse_dummy: per-env, see nh_globals.h */

/* dummy routine used to force linkage */
void
decl_init()
{
    return;
}

/*decl.c*/


/* nh_globals: copy this file's initialized per-env objects into the
 * current context. Generated by tools/collect_globals. */
#ifndef NH_INIT_DECL_C_DONE
#define NH_INIT_DECL_C_DONE
void
nh_init_decl_c(void)
{
    {
        struct mkroom *nh_tmp = &rooms[MAXNROFROOMS + 1];
        memcpy(&subrooms, &nh_tmp, sizeof nh_tmp);
    }
}
#endif
