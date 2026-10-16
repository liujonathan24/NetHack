/* test_resume_blobs.c -- cross-environment checkpoint/resume fidelity.
 *
 * One game is played to a command boundary and made to hallucinate, so
 * every move redraws every monster through see_monsters(); its level,
 * player, RNG and display-mirror blobs are taken and a second environment
 * loads them the way the engine's resume() does (ESC, level, player, ^R,
 * RNG, mirror). From then on the second environment must give the first
 * one's observation hash at every step.
 *
 *   test_resume_blobs <datadir> [nsteps]
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
void *nle_save_level(nle_ctx_t *, long *);
int nle_load_level(nle_ctx_t *, const void *, long);
void *nle_save_player(nle_ctx_t *, long *);
int nle_load_player(nle_ctx_t *, const void *, long);
void nle_free_blob(void *);
size_t nle_rng_blob_size(void);
void nle_rng_save(void *);
void nle_rng_load(const void *);
size_t nle_mirror_blob_size(void);
void nle_mirror_save(nle_ctx_t *, void *);
void nle_mirror_load(nle_ctx_t *, const void *);

/* below the hack.h include: the anchored env's globals */
static void set_hallucination(long turns);
static long hallucination_left(void);
static int see_monsters_deferred(void);

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
    long bl[NLE_BLSTATS_SIZE];
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
lcg(uint32_t *rs, int n)
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
    h = fnv(h, e->bl, sizeof e->bl);
    h = fnv(h, e->message, sizeof e->message);
    h = fnv(h, e->tty_chars, sizeof e->tty_chars);
    h = fnv(h, e->tty_colors, sizeof e->tty_colors);
    h = fnv(h, e->tty_cursor, sizeof e->tty_cursor);
    h = fnv(h, e->inv_glyphs, sizeof e->inv_glyphs);
    h = fnv(h, e->inv_strs, sizeof e->inv_strs);
    h = fnv(h, e->inv_letters, sizeof e->inv_letters);
    h = fnv(h, e->internal, sizeof e->internal);
    h = fnv(h, e->misc, sizeof e->misc);
    return h;
}

static void
env_start(env_t *e, const char *tag)
{
    memset(e, 0, sizeof *e);
    e->obs.glyphs = e->glyphs; e->obs.chars = e->chars; e->obs.colors = e->colors;
    e->obs.specials = e->specials; e->obs.blstats = e->bl; e->obs.message = e->message;
    e->obs.program_state = e->program_state; e->obs.internal = e->internal; e->obs.misc = e->misc;
    e->obs.inv_glyphs = e->inv_glyphs; e->obs.inv_strs = e->inv_strs;
    e->obs.inv_letters = e->inv_letters; e->obs.inv_oclasses = e->inv_oclasses;
    e->obs.tty_chars = e->tty_chars; e->obs.tty_colors = e->tty_colors; e->obs.tty_cursor = e->tty_cursor;
    snprintf(e->hackdir, sizeof e->hackdir, "/tmp/nle_resume_%d/%s", (int) getpid(), tag);
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
    e->seeds.seeds[0] = 42;
    e->seeds.seeds[1] = 7;
    e->seeds.reseed = 0;
    e->rs = 12345u;
    e->nle = nle_start(&e->obs, NULL, &e->seeds, &e->st);
}

static void
env_key(env_t *e, int key)
{
    e->obs.action = key;
    nle_step(e->nle, &e->obs);
}

/* ESC until the game waits for a command (internal[5]), not inside a prompt */
static int
to_boundary(env_t *e)
{
    int n;
    for (n = 0; n < 8 && !(e->internal[5] && !e->misc[0] && !e->misc[1] && !e->misc[2]); n++)
        env_key(e, '\033');
    return n < 8;
}

static void
env_end(env_t *e)
{
    char cmd[512];
    nle_end(e->nle);
    snprintf(cmd, sizeof cmd, "rm -rf %s", e->hackdir);
    if (system(cmd) != 0)
        perror("rm");
}

int
main(int argc, char **argv)
{
    int nsteps = argc > 2 ? atoi(argv[2]) : 60;
    int t, fails = 0, first = -1;
    env_t *a = calloc(1, sizeof *a), *b = calloc(1, sizeof *b);
    uint64_t *h = calloc((size_t) nsteps, sizeof *h);
    char *keys = calloc((size_t) nsteps, 1);
    void *lvl, *plr, *rng, *mir;
    long llen = 0, plen = 0;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <datadir> [nsteps]\n", argv[0]);
        return 2;
    }
    datadir = argv[1];

    /* 1. the straight game: 50 steps, hallucination, 10 more, a boundary */
    env_start(a, "straight");
    for (t = 0; t < 50 && !a->obs.done; t++)
        env_key(a, acts[lcg(&a->rs, (int) sizeof acts - 1)]);
    if (a->obs.done || !to_boundary(a)) {
        printf("FAIL: the straight game ended or is stuck in a prompt\n");
        return 1;
    }
    set_hallucination(400);
    for (t = 0; t < 10 && !a->obs.done; t++)
        env_key(a, acts[lcg(&a->rs, (int) sizeof acts - 1)]);
    if (a->obs.done || !to_boundary(a) || hallucination_left() <= 0) {
        printf("FAIL: no boundary while hallucinating (left %ld)\n", hallucination_left());
        return 1;
    }

    /* 2. the checkpoint: level, player, RNG, mirror */
    lvl = nle_save_level(a->nle, &llen);
    plr = nle_save_player(a->nle, &plen);
    rng = malloc(nle_rng_blob_size());
    nle_rng_save(rng);
    mir = malloc(nle_mirror_blob_size());
    nle_mirror_save(a->nle, mir);
    if (!lvl || !plr) {
        printf("FAIL: blobs not written (level %ld, player %ld)\n", llen, plen);
        return 1;
    }
    for (t = 0; t < nsteps; t++) {
        keys[t] = a->obs.done ? '\033' : acts[lcg(&a->rs, (int) sizeof acts - 1)];
        env_key(a, keys[t]);
        h[t] = obs_hash(a);
    }
    printf("straight: checkpoint at T %ld while hallucinating, %d steps recorded\n", a->bl[20], nsteps);

    /* 3. the resume, as the engine does it */
    env_start(b, "resumed");
    env_key(b, '\033');
    if (nle_load_level(b->nle, lvl, llen) != 0 || nle_load_player(b->nle, plr, plen) != 0) {
        printf("FAIL: the blobs did not load\n");
        return 1;
    }
    env_key(b, 18);
    nle_rng_load(rng);
    nle_mirror_load(b->nle, mir);
    if (see_monsters_deferred()) {
        printf("FAIL: defer_see_monsters is still set after the resume\n");
        fails++;
    }
    for (t = 0; t < nsteps; t++) {
        env_key(b, keys[t]);
        if (obs_hash(b) != h[t]) {
            if (first < 0)
                first = t;
            fails++;
        }
    }
    printf("resumed: %d of %d steps differ%s%d\n", fails, nsteps, first < 0 ? "" : ", first at ", first);

    nle_free_blob(lvl);
    nle_free_blob(plr);
    free(rng);
    free(mir);
    env_end(a);
    env_end(b);
    if (fails) {
        printf("FAIL\n");
        return 1;
    }
    printf("PASS\n");
    return 0;
}

/* the game's globals of the environment the last call anchored (nh_g) */
#include "hack.h"

static void
set_hallucination(long turns)
{
    u.uprops[HALLUC].intrinsic = turns;
}

static long
hallucination_left(void)
{
    return u.uprops[HALLUC].intrinsic;
}

static int
see_monsters_deferred(void)
{
    return defer_see_monsters;
}
