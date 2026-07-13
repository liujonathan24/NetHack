/* nleweb.c — thin browser-facing C API over the NLE engine, so the JS driver
 * never has to know the nle_obs struct layout. Exposes obs allocation, buffer
 * accessors, and start/step/goto wrappers. Compiled into the WASM module. */
#include "nleobs.h"
#include <stdlib.h>
#include <string.h>

typedef struct nle_ctx_t nle_ctx_t;
extern nle_ctx_t *nle_start(nle_obs *, void *, nle_seeds_init_t *, nle_settings *);
extern nle_ctx_t *nle_step(nle_ctx_t *, nle_obs *);
extern void nle_end(nle_ctx_t *);
extern int nle_goto_abs(nle_ctx_t *, int, int);
extern int nle_hero_on_stair(nle_ctx_t *);
extern int nle_num_dungeons(nle_ctx_t *);
extern int nle_dungeon_info(nle_ctx_t *, int, char *, int, int *, int *);

#define ROWNO 21
#define COLNO 80
#define GRID (ROWNO * (COLNO - 1))
#define TTY (NLE_TERM_LI * NLE_TERM_CO)

nle_obs *nleweb_new_obs(void)
{
    nle_obs *o = (nle_obs *)calloc(1, sizeof(nle_obs));
    o->glyphs = calloc(GRID, sizeof(short));
    o->chars = calloc(GRID, 1);
    o->colors = calloc(GRID, 1);
    o->specials = calloc(GRID, 1);
    o->blstats = calloc(NLE_BLSTATS_SIZE, sizeof(long));
    o->message = calloc(NLE_MESSAGE_SIZE, 1);
    o->program_state = calloc(NLE_PROGRAM_STATE_SIZE, sizeof(int));
    o->internal = calloc(NLE_INTERNAL_SIZE, sizeof(int));
    o->inv_glyphs = calloc(NLE_INVENTORY_SIZE, sizeof(short));
    o->inv_strs = calloc(NLE_INVENTORY_SIZE * NLE_INVENTORY_STR_LENGTH, 1);
    o->inv_letters = calloc(NLE_INVENTORY_SIZE, 1);
    o->inv_oclasses = calloc(NLE_INVENTORY_SIZE, 1);
    o->screen_descriptions = calloc(GRID * NLE_SCREEN_DESCRIPTION_LENGTH, 1);
    o->tty_chars = calloc(TTY, 1);
    o->tty_colors = calloc(TTY, 1);
    o->tty_cursor = calloc(2, 1);
    o->misc = calloc(NLE_MISC_SIZE, sizeof(int));
    return o;
}

/* buffer accessors (return raw pointers the JS side reads via HEAP views) */
unsigned char *nleweb_tty_chars(nle_obs *o) { return o->tty_chars; }
signed char *nleweb_tty_colors(nle_obs *o) { return o->tty_colors; }
unsigned char *nleweb_chars(nle_obs *o) { return o->chars; }
unsigned char *nleweb_colors(nle_obs *o) { return o->colors; }
short *nleweb_glyphs(nle_obs *o) { return o->glyphs; }
long *nleweb_blstats(nle_obs *o) { return o->blstats; }
unsigned char *nleweb_message(nle_obs *o) { return o->message; }
int nleweb_done(nle_obs *o) { return o->done; }
int nleweb_in_game(nle_obs *o) { return (int)o->in_normal_game; }

/* dims for the JS side */
int nleweb_rows(void) { return ROWNO; }
int nleweb_cols(void) { return COLNO - 1; }
int nleweb_tty_rows(void) { return NLE_TERM_LI; }
int nleweb_tty_cols(void) { return NLE_TERM_CO; }
int nleweb_blstats_size(void) { return NLE_BLSTATS_SIZE; }

/* Fiber swaps unwind through Asyncify, and the value returned across the JS
 * boundary from such a call is lost (comes back 0). The assignment INSIDE wasm
 * is correct though, so we stash the live ctx in a static and expose it via a
 * plain (non-suspending) accessor for the JS driver to read after start/step. */
static nle_ctx_t *g_nle;

nle_ctx_t *nleweb_start(nle_obs *o, unsigned long seed, const char *opts)
{
    static nle_settings st;
    memset(&st, 0, sizeof st);
    strcpy(st.hackdir, "/nethackdir");
    strcpy(st.datadir, "/nethackdir");
    strncpy(st.options, opts, sizeof(st.options) - 1);
    st.spawn_monsters = 1;
    static nle_seeds_init_t sd;
    memset(&sd, 0, sizeof sd);
    sd.seeds[0] = seed;
    sd.seeds[1] = seed;
    sd.reseed = 0;
    g_nle = nle_start(o, NULL, &sd, &st);
    return g_nle;
}

nle_ctx_t *nleweb_step(nle_obs *o, int action)
{
    o->action = action;
    g_nle = nle_step(g_nle, o);
    return g_nle;
}

nle_ctx_t *nleweb_ctx(void) { return g_nle; } /* plain accessor, no unwind */

/* curriculum: schedule a jump to (dnum,dlevel); processed on the next step. */
int nleweb_goto_abs(int dnum, int dlevel) { return nle_goto_abs(g_nle, dnum, dlevel); }
int nleweb_hero_on_stair(void) { return nle_hero_on_stair(g_nle); }
int nleweb_num_dungeons(void) { return nle_num_dungeons(g_nle); }
int nleweb_dungeon_info(int idx, char *name, int cap, int *depth_start, int *num)
{
    return nle_dungeon_info(g_nle, idx, name, cap, depth_start, num);
}
