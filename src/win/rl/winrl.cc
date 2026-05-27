/* Copyright (c) Facebook, Inc. and its affiliates. */
#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "libc_allocator.h"

extern "C" {
#include "hack.h"
#include "nle.h" /* Current_nle_ctx */
}

extern "C" {
#include "wintty.h"
}

extern "C" {
#include "nleobs.h"
}

/* Include/global.h defines `#define free(p) nle_arena_free(...)`
 * which would otherwise rewrite the std::free calls inside this file and the
 * libc_allocator.h template instantiations into arena frees — the exact
 * thing we are trying to avoid. Undef it here so the rest of this TU sees
 * libc free. Libnethack C code that #include's hack.h still gets the
 * arena-aware free. */
#undef free

#define USE_DEBUG_API 0

#if USE_DEBUG_API
#define DEBUG_API(x)    \
    do {                \
        std::cerr << x; \
    } while (0)
#else
#define DEBUG_API(x)
#endif

/*
 * We had to change xwaitforspace() in getline.c to tell the agent in a
 * --More-- situation that enter/return (ironically not necessarily space)
 * is required to continue.
 */
/* xwaitingforspace — migrated to nle_ctx_t. */
#define xwaitingforspace (current_nle_ctx->xwaitingforspace_v)

/* Some hack.h macros. Can be undefined here. */
#undef Invisible
#undef Warning
#undef index
#undef msleep
#undef rindex
#undef wizard
#undef yn

extern unsigned long nle_seeds[];

extern "C" {
extern void *nle_yield(boolean);
extern nle_obs *nle_get_obs();
}

/* Initial value of glyph_ buffer. Cf. display.c. */
const int nul_glyph = cmap_to_glyph(S_stone);

namespace nethack_rl
{
/* Route per-env STL containers off the shared NLE bump arena
 * and onto libc malloc / free. See libc_allocator.h for the rationale.
 *
 * All STL types that own heap memory and live (transitively) under
 * nle_ctx_t->s_win_proc_calls or nle_ctx_t->s_netHackRL_instance use these
 * libc-backed aliases. The global `new` override in nle_arena_cpp.cc would
 * otherwise put their nodes in the arena where another env's libnethack
 * activity can zero them out from underneath us. */
using LibcString =
    std::basic_string<char, std::char_traits<char>, LibcAllocator<char> >;

template <class T>
using LibcVector = std::vector<T, LibcAllocator<T> >;

template <class T>
using LibcDeque = std::deque<T, LibcAllocator<T> >;

/* Store const char* literals (not LibcString). All 38 ScopedStack call sites
 * pass string literals — no need to construct an std::basic_string per push
 * (a measurable hot-path cost; many of the literals exceed libstdc++ SSO and
 * hit libc malloc each call). Nothing ever reads the contents — the deque is
 * pure scope-tracking. */
using WinProcDeque = LibcDeque<const char *>;

/* Helper: build a LibcString from a C string without relying on a converting
 * constructor that might be ambiguous with the per-allocator overload set. */
static inline LibcString
make_libc_string(const char *s)
{
    return LibcString(s ? s : "", LibcAllocator<char>());
}

/* Per-env via nle_ctx_t->s_win_proc_calls. The `win_proc_calls`
 * symbol is a free function below that returns a reference to the current
 * env's deque, allocated lazily on first use. Previously this was
 * `thread_local std::deque<std::string>`, which crashed when ScopedStack
 * was pushed on the init thread and popped on the OMP step-worker thread
 * after a coroutine resume on the worker.
 *
 * Deque object and its node storage now come from libc, not
 * the arena. We allocate a raw buffer with std::malloc and placement-new
 * the deque into it so the deque control block ALSO lives outside the
 * arena (default `new WinProcDeque()` would route through the arena
 * operator-new override). The matching teardown in destroy_for_ctx /
 * rl_exit_nhwindows runs the dtor explicitly then std::free's the buffer. */
static WinProcDeque &
win_proc_calls()
{
    static thread_local WinProcDeque fallback_deque;
    if (!current_nle_ctx) return fallback_deque;
    auto *d = static_cast<WinProcDeque *>(current_nle_ctx->s_win_proc_calls);
    if (!d) {
        void *mem = std::malloc(sizeof(WinProcDeque));
        if (!mem) std::abort();
        d = new (mem) WinProcDeque();
        current_nle_ctx->s_win_proc_calls = d;
    }
    return *d;
}
#define in_yn_function (current_nle_ctx->s_in_yn_function)
#define in_getlin      (current_nle_ctx->s_in_getlin)

// Glyphs provide instructions for windows to render the game (see display.h).
// At the start of the game, descriptions and properties of the object classes
// are shuffled (see o_init.c) while the glyphs pointing to these classes are
// not. This means glyph observations would always identify a 'wand of
// wishing', regardless of whether it is 'metal', 'balsa', &c.
//
// In this function, we map a glyph to correspond to its shuffled equivalent,
// following the logic used by tiles that also need to generate images from
// glyphs (c.f. o_init.c).  In practice this means:
//   BEFORE: looking up objclass on a glyph gives CORRECT name INCORRECT descr
//   AFTER: looking up objclass on a glyph gives INCORRECT name CORRECT descr
int
shuffled_glyph(int glyph)
{
    if glyph_is_normal_object (glyph) {
        return GLYPH_OBJ_OFF + objects[glyph_to_obj(glyph)].oc_descr_idx;
    }
    return glyph;
}

class ScopedStack
{
  public:
    ScopedStack(WinProcDeque &deque, const char *s) : deque_(deque)
    {
        deque_.push_back(s);
    }

    ~ScopedStack()
    {
        deque_.pop_back();
    }

  private:
    WinProcDeque &deque_;
};

class NetHackRL
{
  public:
    NetHackRL(int &argc, char **argv);

    static void rl_init_nhwindows(int *argc, char **argv);
    static void rl_player_selection();
    static void rl_askname();
    static void rl_get_nh_event();
    static void rl_exit_nhwindows(const char *);
    static void rl_suspend_nhwindows(const char *);
    static void rl_resume_nhwindows();
    static winid rl_create_nhwindow(int type);
    static void rl_clear_nhwindow(winid wid);
    static void rl_display_nhwindow(winid wid, BOOLEAN_P block);
    static void rl_destroy_nhwindow(winid wid);
    static void rl_curs(winid wid, int x, int y);
    static void rl_putstr(winid wid, int attr, const char *text);
    static void rl_display_file(const char *filename, BOOLEAN_P must_exist);
    static void rl_start_menu(winid wid);
    static void rl_add_menu(winid wid, int glyph, const ANY_P *identifier,
                            CHAR_P ch, CHAR_P gch, int attr, const char *str,
                            BOOLEAN_P presel);
    static void rl_end_menu(winid wid, const char *prompt);
    static int rl_select_menu(winid wid, int how, MENU_ITEM_P **menu_list);
    static void rl_update_inventory();
    static void rl_mark_synch();
    static void rl_wait_synch();

    static void rl_cliparound(int x, int y);
    static void rl_print_glyph(winid wid, XCHAR_P x, XCHAR_P y, int glyph,
                               int bkglyph);
    static void rl_raw_print(const char *str);
    static void rl_raw_print_bold(const char *str);
    static int rl_nhgetch();
    static int rl_nh_poskey(int *x, int *y, int *mod);
    static void rl_nhbell();
    static int rl_doprev_message();
    static char rl_yn_function(const char *question, const char *choices,
                               CHAR_P def);
    static void rl_getlin(const char *prompt, char *line);
    static int rl_get_ext_cmd();
    static void rl_number_pad(int);
    static void rl_delay_output();
    static void rl_start_screen();
    static void rl_end_screen();

    static char *rl_getmsghistory(BOOLEAN_P init);
    static void rl_putmsghistory(const char *msg, BOOLEAN_P is_restoring);

    static void rl_outrip(winid wid, int how, time_t when);
    static void rl_status_init();

    static void rl_status_update(int fldidx, genericptr_t ptr, int chg,
                                 int percent, int color,
                                 unsigned long *colormasks);

  private:
    struct rl_menu_item {
        int glyph;            /* Character glyph */
        anything identifier;  /* User identifier */
        long count;           /* User count */
        LibcString str;       /* Description string (libc-backed) */
        int attr;             /* String attribute */
        boolean selected;     /* TRUE if selected by user */
        char selector;        /* Keyboard accelerator */
        char gselector;       /* Group accelerator */
    };

    struct rl_window {
        int type;
        LibcVector<rl_menu_item> menu_items;
        /* Replaced std::vector<std::string> strings with a
         * single last_msg string.  The vector's _M_finish pointer lived in
         * the arena (operator new → arena alloc), so nle_fr_restore would
         * overwrite it with stale (pre-snapshot) content including a non-zero
         * _M_finish, making strings.size() > 0 at the next clear and causing
         * glibc to detect a double-free of an already-tcache'd _M_p.
         * A single string is sufficient because fill_obs only reads the LAST
         * pushed message (back()) for the yn_function case.
         * Also use a libc-backed string so its heap buffer is
         * never zeroed by another env's libnethack activity. */
        LibcString last_msg;
    };

    struct rl_inventory_item {
        int glyph;
        /* Libc-backed strings instead of std::string. */
        LibcString str;
        char letter;
        char object_class;
        LibcString object_class_name;
    };

    /* Per-env (not per-thread). The previous incarnation was
     * `static thread_local std::unique_ptr<NetHackRL> instance`, which
     * meant: every OMP thread had its own NetHackRL initialized only on
     * the thread that called nle_start. PufferLib's cpu_vec_step uses
     * `#pragma omp parallel for`, so worker threads saw a null instance
     * and segfaulted in `instance_get()->getch_method()`.
     *
     * Now the NetHackRL singleton lives in nle_ctx_t->s_netHackRL_instance.
     * `instance` is an inline accessor that resolves to the current env's
     * NetHackRL via current_nle_ctx (which is __thread but set by
     * nle_swap_in before each step). */
    static inline NetHackRL* instance_get() {
        return current_nle_ctx
                   ? static_cast<NetHackRL*>(current_nle_ctx->s_netHackRL_instance)
                   : nullptr;
    }
    static inline void instance_set(NetHackRL* p) {
        if (current_nle_ctx) current_nle_ctx->s_netHackRL_instance = p;
    }
  public:
    /* Allocate the NetHackRL instance through libc malloc and
     * placement-new so the NetHackRL object itself does NOT live in the
     * arena. Without this, `new NetHackRL(...)` routes through the
     * libnethack operator-new override and the instance bytes (including
     * the heap pointers inside windows_, inventory_, status_, ...) sit in
     * the arena and are vulnerable to another env's libnethack writes. */
    static NetHackRL *
    create_libc(int &argc, char **argv)
    {
        void *mem = std::malloc(sizeof(NetHackRL));
        if (!mem) std::abort();
        return new (mem) NetHackRL(argc, argv);
    }

    static void
    destroy_libc(NetHackRL *p) noexcept
    {
        if (!p) return;
        p->~NetHackRL();
        std::free(p);
    }

    /* Called from nle_end (C). */
    static void destroy_for_ctx(nle_ctx_t *nle) {
        if (!nle) return;
        if (nle->s_netHackRL_instance) {
            destroy_libc(static_cast<NetHackRL*>(nle->s_netHackRL_instance));
            nle->s_netHackRL_instance = nullptr;
        }
        if (nle->s_win_proc_calls) {
            auto *d = static_cast<WinProcDeque *>(nle->s_win_proc_calls);
            d->~WinProcDeque();
            std::free(d);
            nle->s_win_proc_calls = nullptr;
        }
    }
  private:

    /* Libc-backed vector of libc-allocated rl_window objects.
     * The custom deleter runs the rl_window dtor (so inner libc strings /
     * vectors free their nodes) then std::free's the buffer, so the
     * rl_window itself never visits the arena either. */
    struct LibcRlWindowDeleter {
        void
        operator()(rl_window *p) const noexcept
        {
            if (!p) return;
            p->~rl_window();
            std::free(p);
        }
    };
    using LibcRlWindowPtr = std::unique_ptr<rl_window, LibcRlWindowDeleter>;
    LibcVector<LibcRlWindowPtr> windows_;

    static LibcRlWindowPtr
    make_libc_rl_window(int type)
    {
        void *mem = std::malloc(sizeof(rl_window));
        if (!mem) std::abort();
        return LibcRlWindowPtr(new (mem) rl_window{ type, {}, {} });
    }

    std::array<int16_t, (COLNO - 1) * ROWNO> glyphs_;

    /* Output of mapglyph */
    std::array<uint8_t, (COLNO - 1) * ROWNO> chars_;
    std::array<uint8_t, (COLNO - 1) * ROWNO> colors_;
    std::array<uint8_t, (COLNO - 1) * ROWNO> specials_;

    std::array<char, (COLNO - 1) * ROWNO * NLE_SCREEN_DESCRIPTION_LENGTH>
        screen_descriptions_;

    void store_glyph(XCHAR_P x, XCHAR_P y, int glyph);
    void store_mapped_glyph(int ch, int color, int special, XCHAR_P x,
                            XCHAR_P y);
    void store_screen_description(XCHAR_P x, XCHAR_P y, int glyph);

    void fill_obs(nle_obs *);
    int getch_method();

    std::array<LibcString, MAXBLSTATS> status_;
    long condition_bits_;

    void update_blstats();
    long blstats_[NLE_BLSTATS_SIZE];

    void player_selection_method();
    void status_update_method(int fldidx, genericptr_t ptr, int, int percent,
                              int color, unsigned long *colormasks);

    void putstr_method(winid wid, int attr, const char *str);

    LibcVector<rl_inventory_item> inventory_;

    void start_menu_method(winid wid);
    void add_menu_method(winid wid, int glyph, const anything *identifier,
                         char ch, char gch, int attr, const char *str,
                         bool preselected);
    void update_inventory_method();

    winid create_nhwindow_method(int type);
    void clear_nhwindow_method(winid wid);
    void display_nhwindow_method(winid wid, BOOLEAN_P block);
    void destroy_nhwindow_method(winid wid);
};

NetHackRL::NetHackRL(int &argc, char **argv) : glyphs_(), blstats_{}
{
    // create base window
    // (done in tty_init_nhwindows before this NetHackRL object got created).
    assert(BASE_WINDOW == 0);
    windows_.emplace_back(make_libc_rl_window(NHW_BASE));
    glyphs_.fill(nul_glyph);
}

void
NetHackRL::player_selection_method()
{
    windows_[BASE_WINDOW]->last_msg.clear();
}

void
NetHackRL::fill_obs(nle_obs *obs)
{
    if (obs->program_state) {
        obs->program_state[0] = current_nle_ctx->program_state.gameover;
        obs->program_state[1] = current_nle_ctx->program_state.panicking;
        obs->program_state[2] = current_nle_ctx->program_state.exiting;
        obs->program_state[3] = current_nle_ctx->program_state.in_moveloop;
        obs->program_state[4] = current_nle_ctx->program_state.in_impossible;
        obs->program_state[5] = current_nle_ctx->program_state.something_worth_saving;
        // TODO: Consider adding something_worth_saving.
        // Also consider adding ttyDisplay->inmore ...
    }
    if (obs->internal) {
        // From do.c. sstairs is a potential "special" staircase.
        boolean stairs_down =
            ((u.ux == xdnstair && u.uy == ydnstair)
             || (u.ux == sstairs.sx && u.uy == sstairs.sy && !sstairs.up));

        obs->internal[0] = deepest_lev_reached(false);
        obs->internal[1] = in_yn_function;
        obs->internal[2] = in_getlin;
        obs->internal[3] = xwaitingforspace;
        obs->internal[4] = stairs_down;
        obs->internal[5] = 0; /* Used to be core seed */
        obs->internal[6] = 0; /* Used to be disp seed */
        obs->internal[7] = u.uhunger;
        obs->internal[8] =
            u.urexp; /* Score (careful! check botl_score() and end.c) */
    }
    if (obs->misc) {
        obs->misc[0] = in_yn_function;
        obs->misc[1] = in_getlin;
        obs->misc[2] = xwaitingforspace;
    }

    if ((!current_nle_ctx->program_state.something_worth_saving && !current_nle_ctx->program_state.in_moveloop)
        || !iflags.window_inited) {
        // Game not yet started (!something_worth_saving && !in_moveloop -- we
        // need both as something_worth_saving also becomes false in
        // really_done(), but we still want to see the "Do you want..."
        // questions) or windows have already been destroyed. Return zero
        // observations.
        obs->in_normal_game = false;
        if (obs->glyphs)
            std::fill_n(obs->glyphs, glyphs_.size(), nul_glyph);
        if (obs->chars)
            std::memset(obs->chars, 0, chars_.size()); /* Or fill with ' '? */
        if (obs->colors)
            std::memset(obs->colors, 0, colors_.size());
        if (obs->specials)
            std::memset(obs->specials, 0, specials_.size());
        if (obs->message)
            std::memset(obs->message, 0, NLE_MESSAGE_SIZE);
        if (obs->blstats)
            std::memset(obs->blstats, 0, sizeof(long) * NLE_BLSTATS_SIZE);
        if (obs->screen_descriptions)
            std::memset(obs->screen_descriptions, 0,
                        screen_descriptions_.size());
        return;
    }
    obs->in_normal_game = true;

    if (obs->glyphs) {
        std::memcpy(obs->glyphs, glyphs_.data(),
                    sizeof(int16_t) * glyphs_.size());
    }
    if (obs->chars) {
        std::memcpy(obs->chars, chars_.data(), chars_.size());
    }
    if (obs->colors) {
        std::memcpy(obs->colors, colors_.data(), colors_.size());
    }
    if (obs->specials) {
        std::memcpy(obs->specials, specials_.data(), specials_.size());
    }
    if (obs->message) {
        // TODO: This doesn't show anything in situations where there's too
        // many items at one tile, which will get displayed in a new window.

        if (in_yn_function) {
            // Special case. See tty_putstr: yn_function doesn't add to
            // toplines until after that frame is over. Use last string on
            // NHW_MESSAGE instead.
            const char *msg = "";
            if (WIN_MESSAGE != WIN_ERR &&
                (size_t)WIN_MESSAGE < windows_.size() &&
                windows_[WIN_MESSAGE]) {
                msg = windows_[WIN_MESSAGE]->last_msg.c_str();
            }
            std::strncpy((char *) &obs->message[0], msg, NLE_MESSAGE_SIZE);
        } else if (ttyDisplay->toplin) {
            // Copy toplines[], see topl.c.
            std::strncpy((char *) &obs->message[0], toplines,
                         NLE_MESSAGE_SIZE);
        } else {
            std::memset(obs->message, 0, NLE_MESSAGE_SIZE);
        }
    }
    if (obs->blstats) {
        /* Exp_039: refresh ALL blstats fields every step, not just X/Y/TIME.
         * Pre-exp_039, blstats_ was populated lazily by status_update_method
         * via the bot() -> bot_via_windowport -> rl_status_update -> BL_FLUSH
         * path. exp_039 disabled status_updates for ~15-30% SPS, but that
         * also disabled bot() and therefore the update_blstats() pump — so
         * the agent silently received HP=HPMAX=DEPTH=AC=...=0 for the entire
         * iter-9 stability matrix run. Fix: call update_blstats()
         * unconditionally here so the agent always gets fresh stats
         * regardless of whether iflags.status_updates is on or off. */
        update_blstats();
        std::memcpy(obs->blstats, &blstats_[0], sizeof(blstats_));
    }
    if (obs->inv_glyphs) {
        /* This iterates over the inventory_ vector list once per inv
           observation instead of only once. I guess that's fine. */
        int i = 0;
        for (const rl_inventory_item &item : inventory_) {
            obs->inv_glyphs[i++] = item.glyph;
        }
        for (; i < NLE_INVENTORY_SIZE; ++i) {
            obs->inv_glyphs[i] = NO_GLYPH;
        }
    }
    if (obs->inv_strs) {
        int i = 0;
        for (const rl_inventory_item &item : inventory_) {
            int j = 0;
            for (int size = min(item.str.size(), NLE_INVENTORY_STR_LENGTH);
                 j < size; ++j) {
                obs->inv_strs[i++] = item.str[j];
            }
            for (; j < NLE_INVENTORY_STR_LENGTH; ++j) {
                obs->inv_strs[i++] = 0;
            }
        }
        for (; i < NLE_INVENTORY_SIZE * NLE_INVENTORY_STR_LENGTH; ++i) {
            obs->inv_strs[i] = 0;
        }
    }
    if (obs->inv_letters) {
        int i = 0;
        for (const rl_inventory_item &item : inventory_) {
            obs->inv_letters[i++] = item.letter;
        }
        for (; i < NLE_INVENTORY_SIZE; ++i) {
            obs->inv_letters[i] = 0;
        }
    }
    if (obs->inv_oclasses) {
        int i = 0;
        for (const rl_inventory_item &item : inventory_) {
            obs->inv_oclasses[i++] = item.object_class;
        }
        for (; i < NLE_INVENTORY_SIZE; ++i) {
            obs->inv_oclasses[i] = MAXOCLASSES;
        }
    }
    if (obs->screen_descriptions) {
        memcpy(obs->screen_descriptions, &screen_descriptions_,
               screen_descriptions_.size());
    }
}

int
NetHackRL::getch_method()
{
    fill_obs(nle_get_obs());
    int i = ((nle_obs *) nle_yield(TRUE))->action;

    /* NOT calling tty_nhgetch() but instead getting the input from
       the context switch. No stdin required. The following code is from
       tty_nhgetch. */
    if (WIN_MESSAGE != WIN_ERR && wins[WIN_MESSAGE])
        wins[WIN_MESSAGE]->wflags &= ~WIN_STOP;
    if (!i)
        i = '\033'; /* Map NUL to ESC since nethack doesn't expect NUL */
    else if (i == EOF)
        i = '\033'; /* Same for EOF */
    if (ttyDisplay && ttyDisplay->toplin == 1)
        ttyDisplay->toplin = 2;
    DEBUG_API("getch_method: action=" << i << ", xwaitingforspace="
                                      << xwaitingforspace << std::endl);
    return i;
}

void
NetHackRL::update_inventory_method()
{
    /* We cannot simply call display_inventory() as window.doc suggests,
       since we want to also use the tty window proc and we don't want the
       inventory to pop up whenever it changed. Instead, we keep our inventory
       list up to date via the following code adopted from display_pickinv
       in invent.c */

    struct obj *otmp;
    inventory_.clear();

    for (otmp = invent; otmp; otmp = otmp->nobj) {
        inventory_.emplace_back(rl_inventory_item{
            shuffled_glyph(obj_to_glyph(otmp, rn2_on_display_rng)),
            make_libc_string(doname(otmp)), otmp->invlet, otmp->oclass,
            make_libc_string(let_to_name(otmp->oclass, false, false)) });
    }
}

void
NetHackRL::store_glyph(XCHAR_P x, XCHAR_P y, int glyph)
{
    // 1 <= x < cols, 0 <= y < rows (!)
    size_t i = (x - 1) % (COLNO - 1);
    size_t j = y % ROWNO;
    size_t offset = j * (COLNO - 1) + i;

    // TODO: Glyphs might be taken from gbuf[y][x].glyph.
    glyphs_[offset] = shuffled_glyph(glyph);
}

void
NetHackRL::store_mapped_glyph(int ch, int color, int special, XCHAR_P x,
                              XCHAR_P y)
{
    // 1 <= x < cols, 0 <= y < rows (!)
    size_t i = (x - 1) % (COLNO - 1);
    size_t j = y % ROWNO;
    size_t offset = j * (COLNO - 1) + i;

    chars_[offset] = ch;
    colors_[offset] = color;
    specials_[offset] = special;
}

void
NetHackRL::store_screen_description(XCHAR_P x, XCHAR_P y, int glyph)
{
    // 1 <= x < cols, 0 <= y < rows (!)
    size_t i = (x - 1) % (COLNO - 1);
    size_t j = y % ROWNO;
    size_t offset = j * (COLNO - 1) + i;
    size_t start = offset * NLE_SCREEN_DESCRIPTION_LENGTH;

    // see code in src/do_name.c:538 auto_describe
    coord cc;
    int sym = 0;
    char tmpbuf[BUFSZ];
    const char *firstmatch = "unknown";

    cc.x = x;
    cc.y = y;

    if (do_screen_description(cc, TRUE, sym, tmpbuf, &firstmatch,
                              (struct permonst **) 0)) {
        strncpy((char *) &screen_descriptions_ + start, firstmatch,
                NLE_SCREEN_DESCRIPTION_LENGTH);
    } else {
        strncpy((char *) &screen_descriptions_ + start, "",
                NLE_SCREEN_DESCRIPTION_LENGTH);
    }
}

void
NetHackRL::update_blstats()
{
    int hitpoints;

    /* See botl.c. */
    int i = Upolyd ? u.mh : u.uhp;
    if (i < 0)
        i = 0;

    hitpoints = min(i, 9999);

    int max_hitpoints;
    i = Upolyd ? u.mhmax : u.uhpmax;
    max_hitpoints = min(i, 9999);

    /* Cf. botl.c. */
    blstats_[NLE_BL_X] = u.ux - 1;     /* X coordinate, 1 <= ux <= cols */
    blstats_[NLE_BL_Y] = u.uy;         /* Y coordinate, 0 <= uy < rows */
    blstats_[NLE_BL_STR25] = ACURRSTR; /* Strength 3..25 */
    blstats_[NLE_BL_STR125] = ACURR(A_STR);        /* Strength 3..125   */
    blstats_[NLE_BL_DEX] = ACURR(A_DEX);           /* Dexterity         */
    blstats_[NLE_BL_CON] = ACURR(A_CON);           /* Constitution      */
    blstats_[NLE_BL_INT] = ACURR(A_INT);           /* Intelligence      */
    blstats_[NLE_BL_WIS] = ACURR(A_WIS);           /* Wisdom            */
    blstats_[NLE_BL_CHA] = ACURR(A_CHA);           /* Charisma          */
    blstats_[NLE_BL_SCORE] = botl_score();         /* Score             */
    blstats_[NLE_BL_HP] = hitpoints;               /* Hitpoints         */
    blstats_[NLE_BL_HPMAX] = max_hitpoints;        /* Max_hitpoints     */
    blstats_[NLE_BL_DEPTH] = depth(&u.uz);         /* Depth             */
    blstats_[NLE_BL_GOLD] = money_cnt(invent);     /* Gold              */
    blstats_[NLE_BL_ENE] = min(u.uen, 9999);       /* Energy            */
    blstats_[NLE_BL_ENEMAX] = min(u.uenmax, 9999); /* Max_energy        */
    blstats_[NLE_BL_AC] = u.uac;                   /* Armor_class       */
    blstats_[NLE_BL_HD] = Upolyd ? (int) mons[u.umonnum].mlevel
                                 : 0;       /* Monster level, hit-dice */
    blstats_[NLE_BL_XP] = u.ulevel;         /* Experience level  */
    blstats_[NLE_BL_EXP] = u.uexp;          /* Experience points */
    blstats_[NLE_BL_TIME] = moves;          /* Time              */
    blstats_[NLE_BL_HUNGER] = u.uhs;        /* Hunger state      */
    blstats_[NLE_BL_CAP] = near_capacity(); /* Carrying capacity */
    blstats_[NLE_BL_DNUM] = u.uz.dnum;      /* Dungeon number */
    blstats_[NLE_BL_DLEVEL] = u.uz.dlevel;  /* Level number */
    blstats_[NLE_BL_CONDITION] = condition_bits_; /* Condition bit mask */
    blstats_[NLE_BL_ALIGN] = u.ualign.type;       /* Character alignment */
}

void
NetHackRL::status_update_method(int fldidx, genericptr_t ptr, int,
                                int percent, int color,
                                unsigned long *colormasks)
{
    if ((fldidx < BL_RESET) || (fldidx >= MAXBLSTATS))
        return;

    // Needs to be kept in sync with the switch statement in rl_status_update.
    if (fldidx == BL_FLUSH || fldidx == BL_RESET) {
        update_blstats();
        return;
    } else if (fldidx == BL_CONDITION) {
        long *condptr = (long *) ptr;
        condition_bits_ = *condptr;
        blstats_[NLE_BL_CONDITION] = condition_bits_;
        return;
    }

    /* Exp_039: status_[] is write-only in this build — no caller reads it.
     * Per perf-record, the make_libc_string allocation + decode_mixed call
     * showed up at ~5-7% combined user CPU (sprintf machinery upstream in
     * bot/eval_notify_windowport_field + the per-field std::basic_string
     * alloc here). Skip the allocation entirely; if a future caller needs
     * the formatted string, restore from the git history of this hunk.
     * blstats_[] (the actual agent-facing data) is still populated via
     * update_blstats() on the BL_FLUSH/BL_RESET branch above. */
    (void) ptr;
    (void) percent;
    (void) color;
    (void) colormasks;
}

void
NetHackRL::putstr_method(winid wid, int attr, const char *str)
{
    DEBUG_API("About to set strings on " << wid << std::endl);
    windows_[wid]->last_msg = make_libc_string(str);
}

winid
NetHackRL::create_nhwindow_method(int type)
{
    std::string window_type;
    switch (type) {
    case NHW_MAP:
        window_type = "map";
        break;
    case NHW_MESSAGE:
        window_type = "message";
        break;
    case NHW_STATUS:
        window_type = "status";
        break;
    case NHW_MENU:
        window_type = "menu";
        break;
    case NHW_TEXT:
        window_type = "text";
        break;
    }

    DEBUG_API("rl_create_nhwindow(type=" << window_type << ")");
    ScopedStack s(win_proc_calls(), "create_nhwindow");

    winid wid = tty_create_nhwindow(type);
    DEBUG_API(": wid == " << wid << std::endl);

    /* Only GROW the vector, never shrink.
     * The original `windows_.resize(wid + 1)` would shrink the vector
     * when wid < windows_.size()-1 (e.g., after WIN_INVEN is destroyed and
     * slot 4 is reused while slot 5 is still live).  Shrinking calls the
     * unique_ptr destructors for all slots above `wid`, freeing those
     * rl_window objects without a corresponding tty_destroy_nhwindow — the
     * freed rl_window's strings vector destructs its elements, and one of
     * those string data buffers may already be in the glibc tcache (freed
     * by a prior rl_clear_nhwindow), triggering a double-free abort.
     * Fix: only extend the vector; never implicitly delete live windows. */
    if ((size_t)(wid + 1) > windows_.size())
        windows_.resize(wid + 1);
    assert(!windows_[wid]);

    DEBUG_API("ABOUT TO RESET " << wid << std::endl;);

    windows_[wid] = make_libc_rl_window(type);
    return wid;
}

void
NetHackRL::clear_nhwindow_method(winid wid)
{
    /* Bounds-check wid before indexing windows_; a stale
     * process-shared wid from a not-yet-migrated global would otherwise
     * cause OOB vector access or a double-free. */
    if (wid < 0 || (size_t) wid >= windows_.size() || !windows_[wid]) {
        return; /* Silently skip the bad wid */
    }
    auto &rl_win = windows_[wid];
    rl_win->menu_items.clear();
    rl_win->last_msg.clear();

    if (wid == WIN_MAP) {
        glyphs_.fill(nul_glyph);
        chars_.fill(' ');
        colors_.fill(0);
        specials_.fill(0);
        if (nle_get_obs()->screen_descriptions) {
            screen_descriptions_.fill(0);
        }
    }

    DEBUG_API("rl_clear_nhwindow(wid=" << wid << ")" << std::endl);
    /* Exp_039: tty_clear_nhwindow emits home()/cl_end()/clear_screen() etc.
     * which all go to nle_putchar -> outbuf. The agent reads the in-memory
     * window state (windows_[wid]->menu_items, last_msg, glyphs_/chars_/
     * colors_) which is already cleared above. The TTY-side rendering
     * here is dead work. Per perf-record: this was ~2.4% of user CPU at
     * N=1024 (clear_nhwindow_method -> tty_clear_nhwindow -> nle_putchar). */
#if 0
    tty_clear_nhwindow(wid);
#endif
}

void
NetHackRL::display_nhwindow_method(winid wid, BOOLEAN_P block)
{
    DEBUG_API("rl_display_nhwindow(wid=" << wid << ", block=" << block << ")"
                                         << std::endl);

    tty_display_nhwindow(wid, block);
}

void
NetHackRL::destroy_nhwindow_method(winid wid)
{
    DEBUG_API("rl_destroy_nhwindow(wid=" << wid << ")" << std::endl);
    windows_[wid].reset(nullptr);
    tty_destroy_nhwindow(wid);
}

void
NetHackRL::start_menu_method(winid wid)
{
    DEBUG_API("rl_start_menu(wid=" << wid << ")" << std::endl);
    tty_start_menu(wid);
    windows_[wid]->menu_items.clear();
}

void
NetHackRL::add_menu_method(
    winid wid,                  /* Window to use, must be of type NHW_MENU */
    int glyph,                  /* Glyph to display with item (not used) */
    const anything *identifier, /* What to return if selected */
    char ch,                    /* Keyboard accelerator (0 = pick our own) */
    char gch,                   /* Group accelerator (0 = no group) */
    int attr,                   /* Attribute for string (like putstr()) */
    const char *str,            /* Menu string */
    bool preselected            /* Item is marked as selected */
)
{
    DEBUG_API("rl_add_menu" << std::endl);
    tty_add_menu(wid, glyph, identifier, ch, gch, attr, str, preselected);

    /* We just add the menu item here. One problem with this method is that
       we won't see any updates happening during tty_select_menu. We could
       try to inspect tty's own menu items instead? */

    windows_[wid]->menu_items.emplace_back(rl_menu_item{
        glyph, *identifier, -1L, make_libc_string(str), attr, preselected, ch,
        gch });
}

void
NetHackRL::rl_init_nhwindows(int *argc, char **argv)
{
    DEBUG_API("rl_init_nhwindows" << std::endl);
    ScopedStack s(win_proc_calls(), "init_nhwindows");
    tty_init_nhwindows(argc, argv);
    /* Allocate via libc, not the arena. */
    instance_set(create_libc(*argc, argv));
}

void
NetHackRL::rl_player_selection()
{
    DEBUG_API("rl_player_selection" << std::endl);
    ScopedStack s(win_proc_calls(), "player_selection");
    tty_player_selection();
    instance_get()->player_selection_method();
}

void
NetHackRL::rl_askname()
{
    DEBUG_API("rl_askname" << std::endl);
    ScopedStack s(win_proc_calls(), "askname");
    tty_askname();
}

void
NetHackRL::rl_get_nh_event()
{
    DEBUG_API("rl_get_nh_event" << std::endl);
    ScopedStack s(win_proc_calls(), "get_nh_event");
    tty_get_nh_event();
}

void
NetHackRL::rl_exit_nhwindows(const char *c)
{
    DEBUG_API("rl_exit_nhwindows" << std::endl);
    ScopedStack s(win_proc_calls(), "exit_nhwindows");
    if (current_nle_ctx && current_nle_ctx->s_netHackRL_instance) {
        destroy_libc(static_cast<NetHackRL*>(current_nle_ctx->s_netHackRL_instance));
        current_nle_ctx->s_netHackRL_instance = nullptr;
    }
    tty_exit_nhwindows(c);
}

void
NetHackRL::rl_suspend_nhwindows(const char *c)
{
    DEBUG_API("rl_suspend_nhwindows" << std::endl);
    ScopedStack s(win_proc_calls(), "suspend_nhwindows");
    tty_suspend_nhwindows(c);
}

void
NetHackRL::rl_resume_nhwindows()
{
    DEBUG_API("rl_resume_nhwindows" << std::endl);
    ScopedStack s(win_proc_calls(), "resume_nhwindows");
    tty_resume_nhwindows();
}

winid
NetHackRL::rl_create_nhwindow(int type)
{
    // win_proc_calls code happens in method.
    return instance_get()->create_nhwindow_method(type);
}

void
NetHackRL::rl_clear_nhwindow(winid wid)
{
    ScopedStack s(win_proc_calls(), "clear_nhwindow");
    instance_get()->clear_nhwindow_method(wid);
}

/* Display_nhwindow(window, boolean blocking)
                -- Display the window on the screen.  If there is data
                   pending for output in that window, it should be sent.
                   If blocking is TRUE, display_nhwindow() will not
                   return until the data has been displayed on the screen,
                   and acknowledged by the user where appropriate.
                -- All calls are blocking in the tty window-port.
                -- Calling display_nhwindow(WIN_MESSAGE,???) will do a
                   --more--, if necessary, in the tty window-port. */
void
NetHackRL::rl_display_nhwindow(winid wid, BOOLEAN_P block)
{
    ScopedStack s(win_proc_calls(), "display_nhwindow");
    instance_get()->display_nhwindow_method(wid, block);
}

void
NetHackRL::rl_destroy_nhwindow(winid wid)
{
    ScopedStack s(win_proc_calls(), "destroy_nhwindow");
    instance_get()->destroy_nhwindow_method(wid);
}

void
NetHackRL::rl_curs(winid wid, int x, int y)
{
    DEBUG_API("rl_curs(wid=" << wid << ", x=" << x << ", y=" << y << ")"
                             << std::endl);
    ScopedStack s(win_proc_calls(), "curs");
    DEBUG_API("rl_curs for window id " << wid << std::endl);
    tty_curs(wid, x, y);
}

void
NetHackRL::rl_putstr(winid wid, int attr, const char *text)
{
    DEBUG_API("rl_putstr(wid=" << wid << ", attr=" << attr
                               << ", text=" << text << ")" << std::endl);
    ScopedStack s(win_proc_calls(), "putstr");
    instance_get()->putstr_method(wid, attr, text);
    tty_putstr(wid, attr, text);
}

void
NetHackRL::rl_display_file(const char *filename, BOOLEAN_P must_exist)
{
    DEBUG_API("rl_display_file" << std::endl);
    ScopedStack s(win_proc_calls(), "display_file");
    tty_display_file(filename, must_exist);
}

void
NetHackRL::rl_start_menu(winid wid)
{
    ScopedStack s(win_proc_calls(), "start_menu");
    instance_get()->start_menu_method(wid);
}

void
NetHackRL::rl_add_menu(winid wid, int glyph, const ANY_P *identifier,
                       CHAR_P ch, CHAR_P gch, int attr, const char *str,
                       BOOLEAN_P presel)
{
    ScopedStack s(win_proc_calls(), "add_menu");
    instance_get()->add_menu_method(wid, glyph, identifier, ch, gch, attr, str,
                              presel);
}

void
NetHackRL::rl_end_menu(winid wid, const char *prompt)
{
    DEBUG_API("rl_end_menu" << std::endl);
    ScopedStack s(win_proc_calls(), "end_menu");
    tty_end_menu(wid, prompt);
}

int
NetHackRL::rl_select_menu(winid wid, int how, MENU_ITEM_P **menu_list)
{
    DEBUG_API("rl_select_menu");
    ScopedStack s(win_proc_calls(), "select_menu");
    int response = tty_select_menu(wid, how, menu_list);
    DEBUG_API(" : " << response << std::endl);
    return response;
}

void
NetHackRL::rl_update_inventory()
{
    DEBUG_API("rl_update_inventory" << std::endl);
    ScopedStack s(win_proc_calls(), "update_inventory");
    instance_get()->update_inventory_method();
}

void
NetHackRL::rl_mark_synch()
{
    DEBUG_API("rl_mark_synch" << std::endl);
    ScopedStack s(win_proc_calls(), "mark_synch");
    tty_mark_synch();
}

void
NetHackRL::rl_wait_synch()
{
    DEBUG_API("rl_wait_synch" << std::endl);
    ScopedStack s(win_proc_calls(), "wait_synch");
    tty_wait_synch();
}

void
NetHackRL::rl_cliparound(int x, int y)
{
#ifdef CLIPPING
    tty_cliparound(x, y);
#endif
}

/* Print_glyph(window, x, y, glyph, bkglyph)
                -- Print the glyph at (x,y) on the given window.  Glyphs are
                   integers at the interface, mapped to whatever the window-
                   port wants (symbol, font, color, attributes, ...there's
                   a 1-1 map between glyphs and distinct things on the map).
                -- bkglyph is a background glyph for potential use by some
                   graphical or tiled environments to allow the depiction
                   to fall against a background consistent with the grid
                   around x,y. If bkglyph is NO_GLYPH, then the parameter
                   should be ignored (do nothing with it). */
void
NetHackRL::rl_print_glyph(winid wid, XCHAR_P x, XCHAR_P y, int glyph,
                          int bkglyph)
{
    int ch;
    int color;
    unsigned special;

    (void) mapglyph(glyph, &ch, &color, &special, x, y, 0);
#if USE_DEBUG_API
    DEBUG_API("rl_print_glyph(wid=" << wid << ", x=" << x << ", y=" << y
                                    << ", glyph=(ch='" << (char) ch
                                    << "', color=" << color
                                    << ", special=" << special);
    int bch;
    int bcolor;
    unsigned bspecial;
    (void) mapglyph(bkglyph, &bch, &bcolor, &bspecial, x, y, 0);
    DEBUG_API("), bkglyph=(ch='" << (char) bch << "', color=" << bcolor
                                 << ", special=" << bspecial << ")"
                                 << std::endl);
#endif

    // No win_proc_calls entry here.
    if (wid == WIN_MAP) {
        instance_get()->store_glyph(x, y, glyph);
        if (glyph != nul_glyph && color == CLR_BLACK) {
            /* This will be 'bright black' (or blue) on tty so we change it to
             * make NLE's colors and tty_colors stay compatible. */
            color = iflags.wc2_darkgray ? 8 : CLR_BLUE;
        }
        instance_get()->store_mapped_glyph(ch, color, special, x, y);
        if (nle_get_obs()->screen_descriptions) {
            instance_get()->store_screen_description(x, y, glyph);
        }
    } else {
        DEBUG_API("Window id is " << wid << ". This shouldn't happen."
                                  << std::endl);
    }

    tty_print_glyph(wid, x, y, glyph, bkglyph);
}
void
NetHackRL::rl_raw_print(const char *str)
{
    DEBUG_API("rl_raw_print" << std::endl);
    ScopedStack s(win_proc_calls(), "raw_print");
    /* Not calling tty_raw_print(str); here or below as that
       uses puts/fputs. */
    xputs(str);
    putchar('\n');
    fflush(stdout);
}

void
NetHackRL::rl_raw_print_bold(const char *str)
{
    DEBUG_API("rl_raw_print_bold" << std::endl);
    ScopedStack s(win_proc_calls(), "raw_bold_print");
    /* Not calling tty_raw_print_bold(str);, so above. */
    xputs(str);
    putchar('\n');
    fflush(stdout);
}

int
NetHackRL::rl_nhgetch()
{
    DEBUG_API("rl_nhgetch" << std::endl);
    ScopedStack s(win_proc_calls(), "nhgetch");
    int i = instance_get()->getch_method();
    return i;
}

int
NetHackRL::rl_nh_poskey(int *x, int *y, int *mod)
{
    nhUse(x);
    nhUse(y);
    nhUse(mod);

    ScopedStack s(win_proc_calls(), "nh_poskey");
    int action = rl_nhgetch();
    DEBUG_API("rl_nh_poskey: " << action << std::endl);
    return action;
    // Not calling nh_poskey, but no extra logic necessary here.
}

void
NetHackRL::rl_nhbell()
{
    DEBUG_API("rl_nhbell" << std::endl);
    ScopedStack s(win_proc_calls(), "nhbell");
    return tty_nhbell();
}

int
NetHackRL::rl_doprev_message()
{
    DEBUG_API("rl_doprev_message" << std::endl);
    ScopedStack s(win_proc_calls(), "doprev_message");
    int result = tty_doprev_message();
    return result;
}

char
NetHackRL::rl_yn_function(const char *question_, const char *choices,
                          CHAR_P def)
{
    DEBUG_API("rl_yn_function" << std::endl);
    ScopedStack s(win_proc_calls(), "yn_function");
    in_yn_function = true;
    char result = tty_yn_function(question_, choices, def);
    in_yn_function = false;
    return result;
}

void
NetHackRL::rl_getlin(const char *prompt, char *line)
{
    DEBUG_API("rl_getlin" << std::endl);
    ScopedStack s(win_proc_calls(), "getlin");
    in_getlin = true;
    tty_getlin(prompt, line);
    in_getlin = false;
}

int
NetHackRL::rl_get_ext_cmd()
{
    DEBUG_API("rl_get_ext_cmd" << std::endl);
    ScopedStack s(win_proc_calls(), "get_ext_cmd");
    return tty_get_ext_cmd();
}

void
NetHackRL::rl_number_pad(int i)
{
    DEBUG_API("rl_number_pad" << std::endl);
    ScopedStack s(win_proc_calls(), "number_pad");
    tty_number_pad(i);
}

void
NetHackRL::rl_delay_output()
{
    DEBUG_API("rl_delay_output" << std::endl);
    // No call to tty_delay_output() as we don't actually want delays.
}

void
NetHackRL::rl_start_screen()
{
    DEBUG_API("rl_start_screen" << std::endl);
    ScopedStack s(win_proc_calls(), "start_screen");
    tty_start_screen();
}

void
NetHackRL::rl_end_screen()
{
    DEBUG_API("rl_end_screen" << std::endl);
    ScopedStack s(win_proc_calls(), "end_screen");
    tty_end_screen();

    if (instance_get()) {
        // The only way instance can still be around is in an error situation.
        // Unfortunately, ZQM doesn't close properly when destructed via
        // global objects. So we do it here.
        if (current_nle_ctx) {
            destroy_libc(static_cast<NetHackRL*>(current_nle_ctx->s_netHackRL_instance));
            current_nle_ctx->s_netHackRL_instance = nullptr;
        }
    }
}

void
NetHackRL::rl_outrip(winid wid, int how, time_t when)
{
    DEBUG_API("rl_outrip" << std::endl);
    genl_outrip(wid, how, when);
}

char *
NetHackRL::rl_getmsghistory(BOOLEAN_P init)
{
    DEBUG_API("rl_getmsghistory" << std::endl);
    return tty_getmsghistory(init);
}

void
NetHackRL::rl_putmsghistory(const char *msg, BOOLEAN_P is_restoring)
{
    DEBUG_API("rl_putmsghistory" << std::endl);
    tty_putmsghistory(msg, is_restoring);
}

void
NetHackRL::rl_status_init()
{
    DEBUG_API("rl_status_init" << std::endl);
    ScopedStack s(win_proc_calls(), "status_init");
    tty_status_init();
}

void
NetHackRL::rl_status_update(int fldidx, genericptr_t ptr, int chg,
                            int percent, int color, unsigned long *colormasks)
{
    DEBUG_API("rl_status_update" << std::endl);

    ScopedStack s(win_proc_calls(), "status_update");
    instance_get()->status_update_method(fldidx, ptr, chg, percent, color,
                                   colormasks);
    /* Exp_039: tty_status_update() formats the status bar (sprintf-heavy)
     * into a TTY buffer that the RL agent never reads — the agent gets
     * its stats via update_blstats / fill_obs straight from u/youmonst.
     * Per perf-record: this path was ~15% of user CPU under N=128 puffer
     * training (printf_positional, __vfprintf, __strchrnul, _IO_default_xsputn).
     * Skip it; nothing downstream consumes the formatted output. */
#if 0 && defined(STATUS_HILITES)
    tty_status_update(fldidx, ptr, chg, percent, color, colormasks);
#endif
}

static void
rl_update_positionbar(char *chrs)
{
    DEBUG_API("rl_update_positionbar" << std::endl);
#ifdef POSITIONBAR
    tty_update_positionbar(chrs);
#endif
}

} // namespace nethack_rl

/* C++ defaults `const` at namespace scope to internal linkage; the
 * `extern` qualifier forces external linkage so windows.c can find it. */
extern const struct window_procs rl_procs;
extern const struct window_procs rl_procs = {
    "rl",
    (WC_COLOR | WC_HILITE_PET | WC_INVERSE | WC_EIGHT_BIT_IN
     | WC_PERM_INVENT),
    (0
#if defined(SELECTSAVED)
     | WC2_SELECTSAVED
#endif
#if defined(STATUS_HILITES)
     | WC2_HILITE_STATUS | WC2_HITPOINTBAR | WC2_FLUSH_STATUS
     | WC2_RESET_STATUS
#endif
     | WC2_DARKGRAY | WC2_SUPPRESS_HIST | WC2_STATUSLINES),
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1 }, /* Color availability */
    nethack_rl::NetHackRL::rl_init_nhwindows,
    nethack_rl::NetHackRL::rl_player_selection,
    nethack_rl::NetHackRL::rl_askname,
    nethack_rl::NetHackRL::rl_get_nh_event,
    nethack_rl::NetHackRL::rl_exit_nhwindows,
    nethack_rl::NetHackRL::rl_suspend_nhwindows,
    nethack_rl::NetHackRL::rl_resume_nhwindows,
    nethack_rl::NetHackRL::rl_create_nhwindow,
    nethack_rl::NetHackRL::rl_clear_nhwindow,
    nethack_rl::NetHackRL::rl_display_nhwindow,
    nethack_rl::NetHackRL::rl_destroy_nhwindow,
    nethack_rl::NetHackRL::rl_curs,
    nethack_rl::NetHackRL::rl_putstr,
    genl_putmixed,
    nethack_rl::NetHackRL::rl_display_file,
    nethack_rl::NetHackRL::rl_start_menu,
    nethack_rl::NetHackRL::rl_add_menu,
    nethack_rl::NetHackRL::rl_end_menu,
    nethack_rl::NetHackRL::rl_select_menu,
    genl_message_menu, /* No need for X-specific handling */
    nethack_rl::NetHackRL::rl_update_inventory,
    nethack_rl::NetHackRL::rl_mark_synch,
    nethack_rl::NetHackRL::rl_wait_synch,
#ifdef CLIPPING
    nethack_rl::NetHackRL::rl_cliparound,
#endif
#ifdef POSITIONBAR
    nethack_rl::rl_update_positionbar,
#endif
    nethack_rl::NetHackRL::rl_print_glyph,
    // NetHackRL::rl_print_glyph_compose,
    nethack_rl::NetHackRL::rl_raw_print,
    nethack_rl::NetHackRL::rl_raw_print_bold,
    nethack_rl::NetHackRL::rl_nhgetch,
    nethack_rl::NetHackRL::rl_nh_poskey,
    nethack_rl::NetHackRL::rl_nhbell,
    nethack_rl::NetHackRL::rl_doprev_message,
    nethack_rl::NetHackRL::rl_yn_function,
    nethack_rl::NetHackRL::rl_getlin,
    nethack_rl::NetHackRL::rl_get_ext_cmd,
    nethack_rl::NetHackRL::rl_number_pad,
    nethack_rl::NetHackRL::rl_delay_output,
#ifdef CHANGE_COLOR /* Only a Mac option currently */
    donull,
    donull,
    donull,
    donull,
#endif
    /* Other defs that really should go away (they're tty specific) */
    nethack_rl::NetHackRL::rl_start_screen,
    nethack_rl::NetHackRL::rl_end_screen,
#ifdef GRAPHIC_TOMBSTONE
    nethack_rl::NetHackRL::rl_outrip,
#else
    genl_outrip,
#endif
    tty_preference_update,
    nethack_rl::NetHackRL::rl_getmsghistory,
    nethack_rl::NetHackRL::rl_putmsghistory,
    nethack_rl::NetHackRL::rl_status_init,
    genl_status_finish,
    tty_status_enablefield,
    nethack_rl::NetHackRL::rl_status_update,
    genl_can_suspend_yes,
};

extern "C" void nle_winrl_destroy_for_ctx(nle_ctx_t *nle) {
    nethack_rl::NetHackRL::destroy_for_ctx(nle);
}
