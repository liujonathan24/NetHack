/* NetHack 3.6	flag.h	$NHDT-Date: 1574900824 2019/11/28 00:27:04 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.160 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2006. */
/* NetHack may be freely redistributed.  See license for details. */

/* If you change the flag structure make sure you increment EDITLEVEL in   */
/* Patchlevel.h if needed.  Changing the instance_flags structure does     */
/* Not require incrementing EDITLEVEL.                                     */

#ifndef FLAG_H
#define FLAG_H

/*
 * Persistent flags that are saved and restored with the game.
 *
 */

struct flag {
    boolean acoustics;  /* Allow dungeon sound messages */
    boolean autodig;    /* MRKR: Automatically dig */
    boolean autoquiver; /* Automatically fill quiver */
    boolean autoopen;   /* Open doors by walking into them */
    boolean beginner;
    boolean biff;      /* Enable checking for mail */
    boolean bones;     /* Allow saving/loading bones */
    boolean confirm;   /* Confirm before hitting tame monsters */
    boolean dark_room; /* Show shadows in lit rooms */
    boolean debug;     /* In debugging mode */
#define wizard flags.debug
    boolean end_own; /* List all own scores */
    boolean explore; /* In exploration mode */
#define discover flags.explore
    boolean female;
    boolean friday13;        /* It's Friday the 13th */
    boolean help;            /* Look in data file for info about stuff */
    boolean ignintr;         /* Ignore interrupts */
    boolean ins_chkpt;       /* Checkpoint as appropriate; INSURANCE */
    boolean invlet_constant; /* Let objects keep their inventory symbol */
    boolean legacy;          /* Print game entry "story" */
    boolean lit_corridor;    /* Show a dark corr as lit if it is in sight */
    boolean nap;             /* `timed_delay' option for display effects */
    boolean null;            /* OK to send nulls to the terminal */
    boolean p__obsolete;     /* [3.6.2: perm_invent moved to iflags] */
    boolean pickup;          /* Whether you pickup or move and look */
    boolean pickup_thrown;   /* Auto-pickup items you threw */
    boolean pushweapon; /* When wielding, push old weapon into second slot */
    boolean rest_on_space;   /* Space means rest */
    boolean safe_dog;        /* Give complete protection to the dog */
    boolean showexp;         /* Show experience points */
    boolean showscore;       /* Show score */
    boolean silent;          /* Whether the bell rings or not */
    /* The story so far:
     * 'sortloot' originally took a True/False value but was changed
     * to use a letter instead.  3.6.0 was released without changing its
     * type from 'boolean' to 'char'.  A compiler was smart enough to
     * complain that assigning any of the relevant letters was not 0 or 1
     * so not appropriate for boolean (by a configuration which used
     * SKIP_BOOLEAN to bypass nethack's 'boolean' and use a C++-compatible
     * one).  So the type was changed to 'xchar', which is guaranteed to
     * match the size of 'boolean' (this guarantee only applies for the
     * !SKIP_BOOLEAN config, unfortunately).  Since xchar does not match
     * actual use, the type was later changed to 'char'.  But that would
     * break 3.6.0 savefile compatibility for configurations which typedef
     * 'schar' to 'short int' instead of to 'char'.  (Needed by pre-ANSI
     * systems that use unsigned characters without a way to force them
     * to be signed.)  So, the type has been changed back to 'xchar' for
     * 3.6.x.
     *
     * TODO:  change to 'char' (and move out of this block of booleans,
     * and get rid of these comments...) once 3.6.0 savefile compatibility
     * eventually ends.
     */
#ifndef SKIP_BOOLEAN
    /* This is the normal configuration; assigning a character constant
       for a normal letter to an 'xchar' variable should always work even
       if 'char' is unsigned since character constants are actually 'int'
       and letters are within the range where signedness shouldn't matter */
    xchar   sortloot; /* 'n'=none, 'l'=loot (pickup), 'f'=full ('l'+invent) */
#else
    /* With SKIP_BOOLEAN, we have no idea what underlying type is being
       used, other than it isn't 'xchar' (although its size might match
       that) or a bitfield (because it must be directly addressable);
       it's probably either 'char' for compactness or 'int' for access,
       but we don't know which and it might be something else anyway;
       flip a coin here and guess 'char' for compactness */
    char    sortloot; /* 'n'=none, 'l'=loot (pickup), 'f'=full ('l'+invent) */
#endif
    boolean sortpack;        /* Sorted inventory */
    boolean sparkle;         /* Show "resisting" special FX (Scott Bigham) */
    boolean standout;        /* Use standout for --More-- */
    boolean time;            /* Display elapsed 'time' */
    boolean tombstone;       /* Print tombstone */
    boolean verbose;         /* Max battle info */
    int end_top, end_around; /* Describe desired score list */
    unsigned moonphase;
    unsigned long suppress_alert;
#define NEW_MOON 0
#define FULL_MOON 4
    unsigned paranoia_bits; /* Alternate confirmation prompting */
#define PARANOID_CONFIRM    0x0001
#define PARANOID_QUIT       0x0002
#define PARANOID_DIE        0x0004
#define PARANOID_BONES      0x0008
#define PARANOID_HIT        0x0010
#define PARANOID_PRAY       0x0020
#define PARANOID_REMOVE     0x0040
#define PARANOID_BREAKWAND  0x0080
#define PARANOID_WERECHANGE 0x0100
#define PARANOID_EATING     0x0200
    int pickup_burden; /* Maximum burden before prompt */
    int pile_limit;    /* Controls feedback when walking over objects */
    char inv_order[MAXOCLASSES];
    char pickup_types[MAXOCLASSES];
#define NUM_DISCLOSURE_OPTIONS 6 /* I,a,v,g,c,o (decl.c) */
#define DISCLOSE_PROMPT_DEFAULT_YES 'y'
#define DISCLOSE_PROMPT_DEFAULT_NO 'n'
#define DISCLOSE_PROMPT_DEFAULT_SPECIAL '?' /* V, default a */
#define DISCLOSE_YES_WITHOUT_PROMPT '+'
#define DISCLOSE_NO_WITHOUT_PROMPT '-'
#define DISCLOSE_SPECIAL_WITHOUT_PROMPT '#' /* V, use a */
    char end_disclose[NUM_DISCLOSURE_OPTIONS + 1]; /* Disclose various
                                                      info upon exit */
    char menu_style;    /* User interface style setting */
    boolean made_fruit; /* Don't easily let the user overflow the number of
                           fruits */

    /* KMH, role patch -- Variables used during startup.
     *
     * If the user wishes to select a role, race, gender, and/or alignment
     * during startup, the choices should be recorded here.  This
     * might be specified through command-line options, environmental
     * variables, a popup dialog box, menus, etc.
     *
     * These values are each an index into an array.  They are not
     * characters or letters, because that limits us to 26 roles.
     * They are not booleans, because someday someone may need a neuter
     * gender.  Negative values are used to indicate that the user
     * hasn't yet specified that particular value.  If you determine
     * that the user wants a random choice, then you should set an
     * appropriate random value; if you just left the negative value,
     * the user would be asked again!
     *
     * These variables are stored here because the u structure is
     * cleared during character initialization, and because the
     * flags structure is restored for saved games.  Thus, we can
     * use the same parameters to build the role entry for both
     * new and restored games.
     *
     * These variables should not be referred to after the character
     * is initialized or restored (specifically, after role_init()
     * is called).
     */
    int initrole;  /* Starting role      (index into roles[])   */
    int initrace;  /* Starting race      (index into races[])   */
    int initgend;  /* Starting gender    (index into genders[]) */
    int initalign; /* Starting alignment (index into aligns[])  */
    int randomall; /* Randomly assign everything not specified */
    int pantheon;  /* Deity selection for priest character */
    /* Items which were in iflags in 3.4.x to preserve savefile compatibility
     */
    boolean lootabc;   /* Use "a/b/c" rather than "o/i/b" when looting */
    boolean showrace;  /* Show hero glyph by race rather than by role */
    boolean travelcmd; /* Allow travel command */
    int runmode;       /* Update screen display during run moves */
};

/*
 * System-specific flags that are saved with the game if SYSFLAGS is defined.
 */

#if defined(AMIFLUSH) || defined(AMII_GRAPHICS) || defined(OPT_DISPMAP)
#define SYSFLAGS
#else
#if defined(MFLOPPY) || defined(MAC)
#define SYSFLAGS
#endif
#endif

#ifdef SYSFLAGS
struct sysflag {
    char sysflagsid[10];
#ifdef AMIFLUSH
    boolean altmeta;  /* Use ALT keys as META */
    boolean amiflush; /* Kill typeahead */
#endif
#ifdef AMII_GRAPHICS
    int numcols;
    unsigned short
        amii_dripens[20]; /* DrawInfo Pens currently there are 13 in v39 */
    AMII_COLOR_TYPE amii_curmap[AMII_MAXCOLORS]; /* Colormap */
#endif
#ifdef OPT_DISPMAP
    boolean fast_map; /* Use optimized, less flexible map display */
#endif
#ifdef MFLOPPY
    boolean asksavedisk;
#endif
#ifdef MAC
    boolean page_wait; /* Put up a --More-- after a page of messages */
#endif
};
#endif

/*
 * Flags that are set each time the game is started.
 * These are not saved with the game.
 *
 */

/* Values for iflags.getpos_coords */
#define GPCOORDS_NONE    'n'
#define GPCOORDS_MAP     'm'
#define GPCOORDS_COMPASS 'c'
#define GPCOORDS_COMFULL 'f'
#define GPCOORDS_SCREEN  's'

enum getloc_filters {
    GFILTER_NONE = 0,
    GFILTER_VIEW,
    GFILTER_AREA,

    NUM_GFILTER
};

struct debug_flags {
    boolean test;
#ifdef TTY_GRAPHICS
    boolean ttystatus;
#endif
#ifdef WIN32
    boolean immediateflips;
#endif
};

struct instance_flags {
    /* Stuff that really isn't option or platform related. They are
     * set and cleared during the game to control the internal
     * behaviour of various NetHack functions and probably warrant
     * a structure of their own elsewhere some day.
     */
    boolean debug_fuzzer;  /* Fuzz testing */
    boolean defer_plname;  /* X11 hack: askname() might not set plname */
    boolean herecmd_menu;  /* Use menu when mouseclick on yourself */
    boolean invis_goldsym; /* Gold symbol is ' '? */
    int at_midnight;       /* Only valid during end of game disclosure */
    int at_night;          /* Also only valid during end of game disclosure */
    int failing_untrap;    /* Move_into_trap() -> spoteffects() -> dotrap() */
    int in_lava_effects;   /* Hack for Boots_off() */
    int last_msg;          /* Indicator of last message player saw */
    int override_ID;       /* True to force full identification of objects */
    int parse_config_file_src;  /* Hack for parse_config_line() */
    int purge_monsters;    /* # of dead monsters still on fmon list */
    int suppress_price;    /* Controls doname() for unpaid objects */
    int terrainmode; /* For getpos()'s autodescribe when #terrain is active */
#define TER_MAP    0x01
#define TER_TRP    0x02
#define TER_OBJ    0x04
#define TER_MON    0x08
#define TER_DETECT 0x10    /* Detect_foo magic rather than #terrain */
    boolean getloc_travelmode;
    int getloc_filter;     /* GFILTER_foo */
    boolean getloc_usemenu;
    boolean getloc_moveskip;
    coord travelcc;        /* Coordinates for travel_cache */
    boolean trav_debug;    /* Display travel path (#if DEBUG only) */
    boolean window_inited; /* True if init_nhwindows() completed */
    boolean vision_inited; /* True if vision is ready */
    boolean sanity_check;  /* Run sanity checks */
    boolean mon_polycontrol; /* Debug: control monster polymorphs */
    boolean in_dumplog;    /* Doing the dumplog right now? */
    boolean in_parse;      /* Is a command being parsed? */
     /* Suppress terminate during options parsing, for --showpaths */
    boolean initoptions_noterminate;

    /* Stuff that is related to options and/or user or platform preferences
     */
    unsigned msg_history; /* Hint: # of top lines to save */
    int getpos_coords;    /* Show coordinates when getting cursor position */
    int menu_headings;    /* ATR for menu headings */
    int *opt_booldup;     /* For duplication of boolean opts in config file */
    int *opt_compdup;     /* For duplication of compound opts in conf file */
#ifdef ALTMETA
    boolean altmeta;      /* Alt-c sends ESC c rather than M-c */
#endif
    boolean autodescribe;     /* Autodescribe mode in getpos() */
    boolean cbreak;           /* In cbreak mode, rogue format */
    boolean deferred_X;       /* Deferred entry into explore mode */
    boolean echo;             /* 1 to echo characters */
    boolean force_invmenu;    /* Always menu when handling inventory */
    /* FIXME: goldX belongs in flags, but putting it in iflags avoids
       breaking 3.6.[01] save files */
    boolean goldX;            /* For BUCX filtering, whether gold is X or U */
    boolean hilite_pile;      /* Mark piles of objects with a hilite */
    boolean implicit_uncursed; /* Maybe omit "uncursed" status in inventory */
    boolean mention_walls;    /* Give feedback when bumping walls */
    boolean menu_head_objsym; /* Show obj symbol in menu headings */
    boolean menu_overlay;     /* Draw menus over the map */
    boolean menu_requested;   /* Flag for overloaded use of 'm' prefix
                               * on some non-move commands */
    boolean menu_tab_sep;     /* Use tabs to separate option menu fields */
    boolean news;             /* Print news */
    boolean num_pad;          /* Use numbers for movement commands */
    boolean perm_invent;      /* Keep full inventories up until dismissed */
    boolean renameallowed;    /* Can change hero name during role selection */
    boolean renameinprogress; /* We are changing hero name */
    boolean status_updates;   /* Allow updates to bottom status lines;
                               * disable to avoid excessive noise when using
                               * a screen reader (use ^X to review status) */
    boolean toptenwin;        /* Ending list in window instead of stdout */
    boolean use_background_glyph; /* Use background glyph when appropriate */
    boolean use_menu_color;   /* Use color in menus; only if wc_color */
#ifdef STATUS_HILITES
    long hilite_delta;     /* Number of moves to leave a temp hilite lit */
    long unhilite_deadline; /* Time when oldest temp hilite should be unlit */
#endif
    boolean zerocomp;         /* Write zero-compressed save files */
    boolean rlecomp;          /* Alternative to zerocomp; run-length encoding
                               * compression of levels when writing savefile */
    uchar num_pad_mode;
    boolean cursesgraphics;     /* Use portable curses extended characters */
#if 0   /* XXXgraphics superseded by symbol sets */
    boolean  DECgraphics;       /* Use DEC VT-xxx extended character set */
    boolean  IBMgraphics;       /* Use IBM extended character set */
#ifdef MAC_GRAPHICS_ENV
    boolean  MACgraphics;       /* Use Macintosh extended character set, as
                                   as defined in the special font HackFont */
#endif
#endif
    uchar bouldersym; /* Symbol for boulder display */
    char prevmsg_window; /* Type of old message window to use */
    boolean extmenu;     /* Extended commands use menu interface */
#ifdef MFLOPPY
    boolean checkspace; /* Check disk space before writing files */
                        /* (in iflags to allow restore after moving
                         * to >2GB partition) */
#endif
#ifdef MICRO
    boolean BIOS; /* Use IBM or ST BIOS calls when appropriate */
#endif
#if defined(MICRO) || defined(WIN32)
    boolean rawio; /* Whether can use rawio (IOCTL call) */
#endif
#ifdef MAC_GRAPHICS_ENV
    boolean MACgraphics; /* Use Macintosh extended character set, as
                            as defined in the special font HackFont */
    unsigned use_stone;  /* Use the stone ppats */
#endif
#if defined(MSDOS) || defined(WIN32)
    boolean hassound;     /* Has a sound card */
    boolean usesound;     /* Use the sound card */
    boolean usepcspeaker; /* Use the pc speaker */
    boolean tile_view;
    boolean over_view;
    boolean traditional_view;
#endif
#ifdef MSDOS
    boolean hasvga; /* Has a vga adapter */
    boolean usevga; /* Use the vga adapter */
    boolean hasvesa; /* Has a VESA-capable VGA adapter */
    boolean usevesa; /* Use the VESA-capable VGA adapter */
    boolean grmode; /* Currently in graphics mode */
#endif
#ifdef LAN_FEATURES
    boolean lan_mail;         /* Mail is initialized */
    boolean lan_mail_fetched; /* Mail is awaiting display */
#endif
#ifdef TTY_TILES_ESCCODES
    boolean vt_tiledata;     /* Output console codes for tile support in TTY */
#endif
    boolean clicklook;       /* Allow right-clicking for look */
    boolean cmdassist;       /* Provide detailed assistance for some comnds */
    boolean time_botl;       /* Context.botl for 'time' (moves) only */
    boolean wizweight;       /* Display weight of everything in wizard mode */
    /*
     * Window capability support.
     */
    boolean wc_color;         /* Use color graphics                  */
    boolean wc_hilite_pet;    /* Hilight pets                        */
    boolean wc_ascii_map;     /* Show map using traditional ascii    */
    boolean wc_tiled_map;     /* Show map using tiles                */
    boolean wc_preload_tiles; /* Preload tiles into memory           */
    int wc_tile_width;        /* Tile width                          */
    int wc_tile_height;       /* Tile height                         */
    char *wc_tile_file;       /* Name of tile file;overrides default */
    boolean wc_inverse;       /* Use inverse video for some things   */
    int wc_align_status;      /*  Status win at top|bot|right|left   */
    int wc_align_message;     /* Message win at top|bot|right|left   */
    int wc_vary_msgcount;     /* Show more old messages at a time    */
    char *wc_foregrnd_menu; /* Points to foregrnd color name for menu win   */
    char *wc_backgrnd_menu; /* Points to backgrnd color name for menu win   */
    char *wc_foregrnd_message; /* Points to foregrnd color name for msg win */
    char *wc_backgrnd_message; /* Points to backgrnd color name for msg win */
    char *wc_foregrnd_status; /* Points to foregrnd color name for status   */
    char *wc_backgrnd_status; /* Points to backgrnd color name for status   */
    char *wc_foregrnd_text; /* Points to foregrnd color name for text win   */
    char *wc_backgrnd_text; /* Points to backgrnd color name for text win   */
    char *wc_font_map;      /* Points to font name for the map win */
    char *wc_font_message;  /* Points to font name for message win */
    char *wc_font_status;   /* Points to font name for status win  */
    char *wc_font_menu;     /* Points to font name for menu win    */
    char *wc_font_text;     /* Points to font name for text win    */
    int wc_fontsiz_map;     /* Font size for the map win           */
    int wc_fontsiz_message; /* Font size for the message window    */
    int wc_fontsiz_status;  /* Font size for the status window     */
    int wc_fontsiz_menu;    /* Font size for the menu window       */
    int wc_fontsiz_text;    /* Font size for text windows          */
    int wc_scroll_amount;   /* Scroll this amount at scroll_margin */
    int wc_scroll_margin;   /* Scroll map when this far from the edge */
    int wc_map_mode;        /* Specify map viewing options, mostly
                             * for backward compatibility */
    int wc_player_selection;    /* Method of choosing character */
    boolean wc_splash_screen;   /* Display an opening splash screen or not */
    boolean wc_popup_dialog;    /* Put queries in pop up dialogs instead of
                                 * in the message window */
    boolean wc_eight_bit_input; /* Allow eight bit input               */
    boolean wc2_fullscreen;     /* Run fullscreen */
    boolean wc2_softkeyboard;   /* Use software keyboard */
    boolean wc2_wraptext;       /* Wrap text */
    boolean wc2_selectsaved;    /* Display a menu of user's saved games */
    boolean wc2_darkgray;    /* Try to use dark-gray color for black glyphs */
    boolean wc2_hitpointbar;  /* Show graphical bar representing hit points */
    boolean wc2_guicolor;       /* Allow colours in gui (outside map) */
    int wc_mouse_support;       /* Allow mouse support */
    int wc2_term_cols;		/* Terminal width, in characters */
    int wc2_term_rows;		/* Terminal height, in characters */
    int wc2_statuslines;        /* Default = 2, curses can handle 3 */
    int wc2_windowborders;	/* Display borders on NetHack windows */
    int wc2_petattr;            /* Text attributes for pet */
#ifdef WIN32
#define MAX_ALTKEYHANDLER 25
    char altkeyhandler[MAX_ALTKEYHANDLER];
#endif
    /* Copies of values in struct u, used during detection when the
       originals are temporarily cleared; kept here rather than
       locally so that they can be restored during a hangup save */
    Bitfield(save_uswallow, 1);
    Bitfield(save_uinwater, 1);
    Bitfield(save_uburied, 1);
    /* Item types used to acomplish "special achievements"; find the target
       object and you'll be flagged as having achieved something... */
    short mines_prize_type;     /* Luckstone */
    short soko_prize_type1;     /* Bag of holding or    */
    short soko_prize_type2;     /* Amulet of reflection */
    struct debug_flags debug;
    boolean windowtype_locked;  /* Windowtype can't change from configfile */
    boolean windowtype_deferred; /* Pick a windowport and store it in
                                    chosen_windowport[], but do not switch to
                                    it in the midst of options processing */
    genericptr_t returning_missile; /* 'struct obj *'; Mjollnir or aklys */
    boolean obsolete;  /* Obsolete options can point at this, it isn't used */
};

/*
 * Old deprecated names
 */
#ifdef TTY_GRAPHICS
#define eight_bit_tty wc_eight_bit_input
#endif
#define use_color wc_color
#define hilite_pet wc_hilite_pet
#define use_inverse wc_inverse
#ifdef MAC_GRAPHICS_ENV
#define large_font obsolete
#endif
#ifdef MAC
#define popup_dialog wc_popup_dialog
#endif
#define preload_tiles wc_preload_tiles

/* Flags — now per-env via macro indirection. All 12
 * struct-field `flags` collisions were renamed (rmflags, lflags, dflags,
 * linfo_flags, mflags, init_flags, cmd_flags, ls_flags, wflags, tflags),
 * so the `flags` token is now unambiguous and can be redirected through
 * current_nle_ctx like iflags/sysflags.
 *
 * Util binaries (makedefs, recover) don't link nle.h and need the
 * traditional `struct flag flags` global, so we gate on
 * NLE_PER_ENV_FLAGS (set by CMakeLists.txt for the nethack target). */
#ifdef NLE_PER_ENV_FLAGS
#define flags (*current_nle_ctx->flags_ptr)
#else
extern NEARDATA struct flag flags;
#endif
#ifdef SYSFLAGS
/* Sysflags — migrated to nle_ctx_t (per-env). No `.sysflags`
 * struct-field collisions. */
#define sysflags (*current_nle_ctx->sysflags_ptr)
#endif
/* Iflags — instance flags. Migrated to nle_ctx_t (per-env). No
 * `.iflags` struct-field collisions in headers or src/. */
#define iflags (*current_nle_ctx->iflags_ptr)

/* Last_msg values
 * Usage:
 *  pline("some message");
 *    pline: vsprintf + putstr + iflags.last_msg = PLNMSG_UNKNOWN;
 *  iflags.last_msg = PLNMSG_some_message;
 * and subsequent code can adjust the next message if it is affected
 * by some_message.  The next message will clear iflags.last_msg.
 */
enum plnmsg_types {
    PLNMSG_UNKNOWN = 0,         /* Arbitrary */
    PLNMSG_ONE_ITEM_HERE,       /* "you see <single item> here" */
    PLNMSG_TOWER_OF_FLAME,      /* Scroll of fire */
    PLNMSG_CAUGHT_IN_EXPLOSION, /* Explode() feedback */
    PLNMSG_OBJ_GLOWS,           /* "the <obj> glows <color>" */
    PLNMSG_OBJNAM_ONLY,         /* Xname/doname only, for #tip */
    PLNMSG_OK_DONT_DIE          /* Overriding death in explore/wizard mode */
};

/* Runmode options */
enum runmode_types {
    RUN_TPORT = 0, /* Don't update display until movement stops */
    RUN_LEAP,      /* Update display every 7 steps */
    RUN_STEP,      /* Update display every single step */
    RUN_CRAWL      /* Walk w/ extra delay after each update */
};

/* Paranoid confirmation prompting */
/* Any yes confirmations also require explicit no (or ESC) to reject */
#define ParanoidConfirm ((flags.paranoia_bits & PARANOID_CONFIRM) != 0)
/* Quit: yes vs y for "Really quit?" and "Enter explore mode?" */
#define ParanoidQuit ((flags.paranoia_bits & PARANOID_QUIT) != 0)
/* Die: yes vs y for "Die?" (dying in explore mode or wizard mode) */
#define ParanoidDie ((flags.paranoia_bits & PARANOID_DIE) != 0)
/* Hit: yes vs y for "Save bones?" in wizard mode */
#define ParanoidBones ((flags.paranoia_bits & PARANOID_BONES) != 0)
/* Hit: yes vs y for "Really attack <the peaceful monster>?" */
#define ParanoidHit ((flags.paranoia_bits & PARANOID_HIT) != 0)
/* Pray: ask "Really pray?" (accepts y answer, doesn't require yes),
   taking over for the old prayconfirm boolean option */
#define ParanoidPray ((flags.paranoia_bits & PARANOID_PRAY) != 0)
/* Remove: remove ('R') and takeoff ('T') commands prompt for an inventory
   item even when only one accessory or piece of armor is currently worn */
#define ParanoidRemove ((flags.paranoia_bits & PARANOID_REMOVE) != 0)
/* Breakwand: Applying a wand */
#define ParanoidBreakwand ((flags.paranoia_bits & PARANOID_BREAKWAND) != 0)
/* Werechange: accepting randomly timed werecreature change to transform
   from human to creature or vice versa while having polymorph control */
#define ParanoidWerechange ((flags.paranoia_bits & PARANOID_WERECHANGE) != 0)
/* Continue eating: prompt given _after_first_bite_ when eating something
   while satiated */
#define ParanoidEating ((flags.paranoia_bits & PARANOID_EATING) != 0)

/* Command parsing, mainly dealing with number_pad handling;
   not saved and restored */

#ifdef NHSTDC
/* Forward declaration sufficient to declare pointers */
struct ext_func_tab; /* From func_tab.h */
#endif

/* Special key functions */
enum nh_keyfunc {
    NHKF_ESC = 0,
    NHKF_DOAGAIN,

    NHKF_REQMENU,

    /* Run ... clicklook need to be in a continuous block */
    NHKF_RUN,
    NHKF_RUN2,
    NHKF_RUSH,
    NHKF_FIGHT,
    NHKF_FIGHT2,
    NHKF_NOPICKUP,
    NHKF_RUN_NOPICKUP,
    NHKF_DOINV,
    NHKF_TRAVEL,
    NHKF_CLICKLOOK,

    NHKF_REDRAW,
    NHKF_REDRAW2,
    NHKF_GETDIR_SELF,
    NHKF_GETDIR_SELF2,
    NHKF_GETDIR_HELP,
    NHKF_COUNT,
    NHKF_GETPOS_SELF,
    NHKF_GETPOS_PICK,
    NHKF_GETPOS_PICK_Q,  /* Quick */
    NHKF_GETPOS_PICK_O,  /* Once */
    NHKF_GETPOS_PICK_V,  /* Verbose */
    NHKF_GETPOS_SHOWVALID,
    NHKF_GETPOS_AUTODESC,
    NHKF_GETPOS_MON_NEXT,
    NHKF_GETPOS_MON_PREV,
    NHKF_GETPOS_OBJ_NEXT,
    NHKF_GETPOS_OBJ_PREV,
    NHKF_GETPOS_DOOR_NEXT,
    NHKF_GETPOS_DOOR_PREV,
    NHKF_GETPOS_UNEX_NEXT,
    NHKF_GETPOS_UNEX_PREV,
    NHKF_GETPOS_INTERESTING_NEXT,
    NHKF_GETPOS_INTERESTING_PREV,
    NHKF_GETPOS_VALID_NEXT,
    NHKF_GETPOS_VALID_PREV,
    NHKF_GETPOS_HELP,
    NHKF_GETPOS_MENU,
    NHKF_GETPOS_LIMITVIEW,
    NHKF_GETPOS_MOVESKIP,

    NUM_NHKF
};

enum gloctypes {
    GLOC_MONS = 0,
    GLOC_OBJS,
    GLOC_DOOR,
    GLOC_EXPLORE,
    GLOC_INTERESTING,
    GLOC_VALID,

    NUM_GLOCS
};

/* Commands[] is used to directly access cmdlist[] instead of looping
   through it to find the entry for a given input character;
   move_X is the character used for moving one step in direction X;
   alphadirchars corresponds to old sdir,
   dirchars corresponds to ``iflags.num_pad ? ndir : sdir'';
   pcHack_compat and phone_layout only matter when num_pad is on,
   swap_yz only matters when it's off */
struct cmd {
    unsigned serialno;     /* Incremented after each update */
    boolean num_pad;       /* Same as iflags.num_pad except during updates */
    boolean pcHack_compat; /* For numpad:  affects 5, M-5, and M-0 */
    boolean phone_layout;  /* Inverted keypad:  1,2,3 above, 7,8,9 below */
    boolean swap_yz;       /* QWERTZ keyboards; use z to move NW, y to zap */
    char move_W, move_NW, move_N, move_NE, move_E, move_SE, move_S, move_SW;
    const char *dirchars;      /* Current movement/direction characters */
    const char *alphadirchars; /* Same as dirchars if !numpad */
    const struct ext_func_tab *commands[256]; /* Indexed by input character */
    char spkeys[NUM_NHKF];
};

/* Cmd — migrated to nle_ctx_t (per-env command bindings). */
#define Cmd (*current_nle_ctx->s5_cmd_p)

#endif /* FLAG_H */
