/* test_interleave.c -- per-environment isolation and snapshot fidelity.
 *
 * Every environment must be a pure function of its own (seed, action)
 * history. The test replays K seeded games three ways and requires
 * byte-identical observation hashes at every step:
 *
 *   1. solo:         each game alone in the process;
 *   2. interleaved:  all games stepped round-robin, with one game ended
 *                    half-way and a fresh one started in its place;
 *   3. restored:     a snapshot taken mid-game, the game played on, then
 *                    restored and played again -- the second continuation
 *                    must reproduce the first.
 *
 * Any shared mutable state left in the library (a global the collector
 * missed, an NLE-layer static, a stale display mirror) shows up here as a
 * hash mismatch. Builds against nleobs.h only.
 *
 *   test_interleave <datadir> [nseeds] [nsteps]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "nleobs.h"

typedef struct nle_ctx nle_ctx_t;
nle_ctx_t *nle_start(nle_obs *, FILE *, nle_seeds_init_t *, nle_settings *);
nle_ctx_t *nle_step(nle_ctx_t *, nle_obs *);
void nle_end(nle_ctx_t *);
void *nle_fr_snapshot(nle_ctx_t *);
void nle_fr_restore(nle_ctx_t *, void *);
void nle_fr_destroy(void *);
int nle_fr_last_restore_rc(void);

#define ROWNO 21
#define COLNO 80
#define MAPSZ (ROWNO * (COLNO - 1))

typedef struct env {
    nle_obs obs;
    nle_ctx_t *nle;
    nle_settings st;
    nle_seeds_init_t seeds;
    uint32_t rs;
    char hackdir[256];
    short glyphs[MAPSZ];
    unsigned char chars[MAPSZ], colors[MAPSZ], specials[MAPSZ];
    long blstats[NLE_BLSTATS_SIZE];
    unsigned char message[NLE_MESSAGE_SIZE];
    int program_state[NLE_PROGRAM_STATE_SIZE], internal[NLE_INTERNAL_SIZE], misc[NLE_MISC_SIZE];
    short inv_glyphs[NLE_INVENTORY_SIZE];
    unsigned char inv_strs[NLE_INVENTORY_SIZE * NLE_INVENTORY_STR_LENGTH];
    unsigned char inv_letters[NLE_INVENTORY_SIZE], inv_oclasses[NLE_INVENTORY_SIZE];
    unsigned char tty_chars[NLE_TERM_LI * NLE_TERM_CO];
    signed char tty_colors[NLE_TERM_LI * NLE_TERM_CO];
    unsigned char tty_cursor[2];
} env_t;

static const char acts[] = "hjklyubnhjklyubnhjklsss.:,\r\033 <>ihjklHJKL";
static const char *datadir;

static uint64_t
fnv(uint64_t h, const void *p, size_t n)
{
    const unsigned char *b = p;
    for (size_t i = 0; i < n; i++) {
        h ^= b[i];
        h *= 1099511628211ULL;
    }
    return h;
}

static int
rnd(uint32_t *rs, int n)
{
    *rs = *rs * 1103515245u + 12345u;
    return (*rs >> 16) % n;
}

static uint64_t
obs_hash(env_t *e)
{
    uint64_t h = 1469598103934665603ULL;
    h = fnv(h, e->glyphs, sizeof e->glyphs);
    h = fnv(h, e->chars, sizeof e->chars);
    h = fnv(h, e->colors, sizeof e->colors);
    h = fnv(h, e->specials, sizeof e->specials);
    h = fnv(h, e->blstats, sizeof e->blstats);
    h = fnv(h, e->message, sizeof e->message);
    h = fnv(h, e->tty_chars, sizeof e->tty_chars);
    h = fnv(h, e->tty_colors, sizeof e->tty_colors);
    h = fnv(h, e->inv_strs, sizeof e->inv_strs);
    h = fnv(h, e->inv_letters, sizeof e->inv_letters);
    h = fnv(h, e->internal, sizeof e->internal);
    h = fnv(h, e->misc, sizeof e->misc);
    return h;
}

static void
env_start(env_t *e, int seed)
{
    memset(e, 0, sizeof *e);
    e->obs.glyphs = e->glyphs; e->obs.chars = e->chars; e->obs.colors = e->colors;
    e->obs.specials = e->specials; e->obs.blstats = e->blstats; e->obs.message = e->message;
    e->obs.program_state = e->program_state; e->obs.internal = e->internal; e->obs.misc = e->misc;
    e->obs.inv_glyphs = e->inv_glyphs; e->obs.inv_strs = e->inv_strs;
    e->obs.inv_letters = e->inv_letters; e->obs.inv_oclasses = e->inv_oclasses;
    e->obs.tty_chars = e->tty_chars; e->obs.tty_colors = e->tty_colors; e->obs.tty_cursor = e->tty_cursor;
    /* one writable dir per seed, the same in every phase: the end-of-game
     * screen prints file paths, so the path must not vary between runs */
    snprintf(e->hackdir, sizeof e->hackdir, "/tmp/nle_interleave_%d/seed%d", (int) getpid(), seed);
    {
        char cmd[512];
        snprintf(cmd, sizeof cmd, "mkdir -p %s", e->hackdir);
        if (system(cmd) != 0) {
            perror("mkdir");
            exit(2);
        }
    }
    snprintf(e->st.hackdir, sizeof e->st.hackdir, "%.200s/", e->hackdir);
    snprintf(e->st.datadir, sizeof e->st.datadir, "%s", datadir);
    snprintf(e->st.options, sizeof e->st.options, "%s",
             "autopickup,color,disclose:+i +a +v +g +c +o,mention_walls,nobones,nocmdassist,nolegacy,"
             "nosparkle,pickup_burden:unencumbered,pickup_types:$?!/,runmode:teleport,showexp,showscore,"
             "time,name:Agent,race:human,role:Valkyrie,gender:female,align:neutral,pettype:none");
    e->st.spawn_monsters = 1;
    e->seeds.seeds[0] = (unsigned long) seed * 1000003UL;
    e->seeds.seeds[1] = (unsigned long) seed * 7UL + 1;
    e->seeds.reseed = 0;
    e->rs = 12345u + (uint32_t) seed;
    e->nle = nle_start(&e->obs, NULL, &e->seeds, &e->st);
}

/* one step with the seed's deterministic action stream; returns the hash of
 * the observation BEFORE the action (i.e. the state at this step) */
static uint64_t
env_step(env_t *e)
{
    uint64_t h = obs_hash(e);
    if (!e->obs.done) {
        e->obs.action = acts[rnd(&e->rs, (int) sizeof acts - 1)];
        nle_step(e->nle, &e->obs);
    }
    return h;
}

static void
env_end(env_t *e)
{
    char cmd[512];
    nle_end(e->nle);
    e->nle = NULL;
    snprintf(cmd, sizeof cmd, "rm -rf %s", e->hackdir);
    if (system(cmd) != 0)
        perror("rm");
}

int
main(int argc, char **argv)
{
    int nseeds = argc > 2 ? atoi(argv[2]) : 4;
    int nsteps = argc > 3 ? atoi(argv[3]) : 200;
    int i, t, fails = 0;
    uint64_t *solo;
    env_t *envs;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <datadir> [nseeds] [nsteps]\n", argv[0]);
        return 2;
    }
    datadir = argv[1];
    /* one extra seed for the game started mid-run */
    solo = calloc((size_t) (nseeds + 1) * nsteps, sizeof *solo);
    envs = calloc((size_t) nseeds + 1, sizeof *envs);

    /* 1. solo reference runs */
    for (i = 0; i <= nseeds; i++) {
        env_start(&envs[0], i + 1);
        for (t = 0; t < nsteps; t++)
            solo[i * nsteps + t] = env_step(&envs[0]);
        env_end(&envs[0]);
    }
    printf("solo: %d games x %d steps recorded\n", nseeds + 1, nsteps);

    /* 2. interleaved: all games at once, round-robin; game 0 is replaced by
     *    seed nseeds+1 half-way through */
    for (i = 0; i < nseeds; i++)
        env_start(&envs[i], i + 1);
    {
        int half = nsteps / 2, t0 = 0; /* game 0's own step counter */
        for (t = 0; t < nsteps; t++) {
            for (i = 0; i < nseeds; i++) {
                int ref = i, rt = t;
                if (i == 0) {
                    if (t == half) {
                        env_end(&envs[0]);
                        env_start(&envs[0], nseeds + 1);
                        t0 = 0;
                    }
                    if (t >= half) {
                        ref = nseeds;
                        rt = t0++;
                    }
                }
                uint64_t h = env_step(&envs[i]);
                if (h != solo[ref * nsteps + rt]) {
                    if (fails < 10)
                        printf("MISMATCH interleaved: env %d (seed %d) step %d\n", i, ref + 1, rt);
                    fails++;
                }
            }
        }
        /* the replacement game only ran nsteps - half steps; fine */
    }
    for (i = 0; i < nseeds; i++)
        env_end(&envs[i]);
    printf("interleaved: %s\n", fails ? "FAIL" : "ok");

    /* 3. snapshot / restore fidelity on seed 1 */
    {
        int at = nsteps / 3, f0 = fails;
        void *snap;
        env_start(&envs[0], 1);
        for (t = 0; t < at; t++)
            env_step(&envs[0]);
        snap = nle_fr_snapshot(envs[0].nle);
        if (!snap) {
            printf("snapshot: FAIL (NULL handle)\n");
            fails++;
        } else {
            uint32_t rs_at = envs[0].rs;
            for (t = at; t < nsteps; t++) {
                uint64_t h = env_step(&envs[0]);
                if (h != solo[0 * nsteps + t]) {
                    if (fails < 10)
                        printf("MISMATCH pre-restore: step %d\n", t);
                    fails++;
                }
            }
            nle_fr_restore(envs[0].nle, snap);
            if (nle_fr_last_restore_rc() != 0) {
                printf("restore: level-file set not restored (rc=%d)\n", nle_fr_last_restore_rc());
                fails++;
            }
            envs[0].rs = rs_at;
            /* after a restore the buffers reflect the restored state only after
             * the next step: the first observation is checked from step at+1 */
            (void) env_step(&envs[0]);
            for (t = at + 1; t < nsteps; t++) {
                uint64_t h = env_step(&envs[0]);
                if (h != solo[0 * nsteps + t]) {
                    if (fails < 10)
                        printf("MISMATCH post-restore: step %d\n", t);
                    fails++;
                }
            }
            nle_fr_destroy(snap);
        }
        env_end(&envs[0]);
        printf("snapshot/restore: %s\n", fails > f0 ? "FAIL" : "ok");
    }

    free(solo);
    free(envs);
    if (fails) {
        printf("FAIL: %d mismatches\n", fails);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
