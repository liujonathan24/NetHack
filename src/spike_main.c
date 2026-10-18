/* Phase-0 spike harness: drive the WASM engine (nle_start + a few nle_step) and
 * dump the tty frame, to prove the emscripten_fiber coroutine shim works and
 * NetHack runs under WebAssembly. Temporary — not part of the real build. */
#include "nleobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct nle_ctx_t nle_ctx_t;
extern nle_ctx_t *nle_start(nle_obs *, void *ttyrec, nle_seeds_init_t *, nle_settings *);
extern nle_ctx_t *nle_step(nle_ctx_t *, nle_obs *);
extern void nle_end(nle_ctx_t *);

#define ROWNO 21
#define COLNO 80
#define GRID (ROWNO * (COLNO - 1))
#define TTY (NLE_TERM_LI * NLE_TERM_CO)

static nle_obs OBS;

static void setup_obs(void)
{
    memset(&OBS, 0, sizeof OBS);
    OBS.glyphs = calloc(GRID, sizeof(short));
    OBS.chars = calloc(GRID, 1);
    OBS.colors = calloc(GRID, 1);
    OBS.specials = calloc(GRID, 1);
    OBS.blstats = calloc(NLE_BLSTATS_SIZE, sizeof(long));
    OBS.message = calloc(NLE_MESSAGE_SIZE, 1);
    OBS.program_state = calloc(NLE_PROGRAM_STATE_SIZE, sizeof(int));
    OBS.internal = calloc(NLE_INTERNAL_SIZE, sizeof(int));
    OBS.inv_glyphs = calloc(NLE_INVENTORY_SIZE, sizeof(short));
    OBS.inv_strs = calloc(NLE_INVENTORY_SIZE * NLE_INVENTORY_STR_LENGTH, 1);
    OBS.inv_letters = calloc(NLE_INVENTORY_SIZE, 1);
    OBS.inv_oclasses = calloc(NLE_INVENTORY_SIZE, 1);
    OBS.screen_descriptions = calloc(GRID * NLE_SCREEN_DESCRIPTION_LENGTH, 1);
    OBS.tty_chars = calloc(TTY, 1);
    OBS.tty_colors = calloc(TTY, 1);
    OBS.tty_cursor = calloc(2, 1);
    OBS.misc = calloc(NLE_MISC_SIZE, sizeof(int));
}

static void dump(const char *tag)
{
    printf("=== %s | in_game=%d done=%d hp=%ld/%ld depth=%ld ===\n", tag,
           OBS.in_normal_game, OBS.done, OBS.blstats[NLE_BL_HP],
           OBS.blstats[NLE_BL_HPMAX], OBS.blstats[NLE_BL_DEPTH]);
    for (int r = 0; r < NLE_TERM_LI; r++) {
        for (int c = 0; c < NLE_TERM_CO; c++) {
            unsigned char ch = OBS.tty_chars[r * NLE_TERM_CO + c];
            putchar((ch >= 32 && ch < 127) ? ch : ' ');
        }
        putchar('\n');
    }
}

int main(void)
{
    setup_obs();
    nle_settings st;
    memset(&st, 0, sizeof st);
    strcpy(st.hackdir, "/nethackdir");
    strcpy(st.datadir, "/nethackdir");
    strcpy(st.options,
           "autopickup,color,disclose:+i +a +v +g +c +o,mention_walls,nobones,"
           "nocmdassist,nolegacy,nosparkle,pickup_burden:unencumbered,"
           "pickup_types:$?!/,runmode:teleport,showexp,showscore,time,"
           "name:Agent-Val-hum-neu-fem");
    st.spawn_monsters = 1;

    nle_seeds_init_t seeds;
    memset(&seeds, 0, sizeof seeds);
    seeds.seeds[0] = 19;
    seeds.seeds[1] = 19;
    seeds.reseed = 0;

    printf("[spike] nle_start...\n");
    fflush(stdout);
    nle_ctx_t *nle = nle_start(&OBS, NULL, &seeds, &st);
    dump("after start");

    for (int i = 0; i < 8; i++) {
        OBS.action = ' '; /* space: dismiss --More-- / no-op */
        nle = nle_step(nle, &OBS);
        if (OBS.done) {
            printf("[spike] done after %d steps\n", i + 1);
            break;
        }
    }
    dump("after steps");
    nle_end(nle);
    printf("[spike] DONE\n");
    return 0;
}
