/* test_multi_env.c — multi-env stress test for the per-env NLE refactor.
 *
 * Build (from vendor/nle/):
 *   gcc -O2 -fopenmp -DNLE_ALLOW_SEEDING=1 test_multi_env.c \
 *       -Iinclude -Lsrc/build -lnethack -Wl,-rpath,src/build \
 *       -o test_multi_env -lm
 *
 * Run:
 *   NETHACKDIR=$PWD/src/build/dat ./test_multi_env [N_ENVS] [STEPS] [THREADS]
 *   # defaults: 64 envs, 500 steps, 4 threads
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <omp.h>

#include "nleobs.h"

typedef struct nle_ctx_t nle_ctx_t;
extern nle_ctx_t *nle_start(nle_obs *, FILE *, nle_seeds_init_t *, nle_settings *);
extern nle_ctx_t *nle_step(nle_ctx_t *, nle_obs *);
extern void nle_end(nle_ctx_t *);

static void touch(const char *path) {
    int fd = open(path, O_CREAT | O_WRONLY, 0644);
    if (fd >= 0) close(fd);
}

static void make_vardir(const char *hackdir, char *out, size_t cap) {
    char tmpl[] = "/tmp/nle-test-XXXXXX";
    char *d = mkdtemp(tmpl);
    assert(d);
    snprintf(out, cap, "%s", d);
    char src[512], dst[512];
    snprintf(src, sizeof(src), "%s/nhdat", hackdir);
    snprintf(dst, sizeof(dst), "%s/nhdat", d);
    assert(symlink(src, dst) == 0);
    const char *files[] = {"perm", "record", "logfile", "xlogfile"};
    for (int i = 0; i < 4; i++) {
        snprintf(dst, sizeof(dst), "%s/%s", d, files[i]);
        touch(dst);
    }
    snprintf(dst, sizeof(dst), "%s/save", d);
    mkdir(dst, 0755);
}

static void rm_vardir(const char *dir) {
    char path[512];
    const char *files[] = {"nhdat", "perm", "record", "logfile", "xlogfile", "paniclog"};
    for (int i = 0; i < 6; i++) {
        snprintf(path, sizeof(path), "%s/%s", dir, files[i]);
        unlink(path);
    }
    snprintf(path, sizeof(path), "%s/save", dir);
    rmdir(path);
    rmdir(dir);
}

typedef struct {
    nle_ctx_t    *ctx;
    nle_obs       obs;
    nle_settings  settings;
    char          vardir[256];
    short         glyphs[21 * 79];
    unsigned char chars[21 * 79];
    unsigned char colors[21 * 79];
    long          blstats[NLE_BLSTATS_SIZE];
    unsigned char message[NLE_MESSAGE_SIZE];
    int           misc[NLE_MISC_SIZE];
    int           internal[NLE_INTERNAL_SIZE];
    unsigned long seed_a, seed_b;
    long          total_score;
    int           episodes;
} TestEnv;

static void env_bind_obs(TestEnv *e) {
    e->obs.glyphs   = e->glyphs;
    e->obs.chars     = e->chars;
    e->obs.colors    = e->colors;
    e->obs.blstats   = e->blstats;
    e->obs.message   = e->message;
    e->obs.misc      = e->misc;
    e->obs.internal  = e->internal;
}

static void env_init(TestEnv *e, int idx, const char *hackdir) {
    memset(e, 0, sizeof(*e));
    make_vardir(hackdir, e->vardir, sizeof(e->vardir));

    strncpy(e->settings.hackdir, e->vardir, sizeof(e->settings.hackdir) - 1);
    strncpy(e->settings.options,
            "name:Agent-mon-hum-neu-mal,autopickup,color,"
            "disclose:+i +a +v +g +c +o,nobones,!status_updates",
            sizeof(e->settings.options) - 1);
    e->settings.spawn_monsters = 1;
    env_bind_obs(e);

    e->seed_a = 0xCAFEBEEF + (unsigned long)idx;
    e->seed_b = (0xCAFEBEEF ^ 0x9E3779B97F4A7C15UL) + (unsigned long)idx;
}

static void env_start(TestEnv *e) {
    e->seed_a = e->seed_a * 6364136223846793005UL + 1442695040888963407UL;
    e->seed_b = e->seed_b * 6364136223846793005UL + 1442695040888963407UL;
    nle_seeds_init_t seeds;
    memset(&seeds, 0, sizeof(seeds));
    seeds.seeds[0] = e->seed_a;
    seeds.seeds[1] = e->seed_b;
    seeds.reseed = 0;
    e->obs.action = 0;
    e->obs.done = 0;
    e->ctx = nle_start(&e->obs, NULL, &seeds, &e->settings);
    assert(e->ctx);
}

static void env_step(TestEnv *e, int action) {
    e->obs.action = action;
    e->ctx = nle_step(e->ctx, &e->obs);
}

static void env_end(TestEnv *e) {
    if (e->ctx) {
        nle_end(e->ctx);
        e->ctx = NULL;
    }
}

static void env_close(TestEnv *e) {
    env_end(e);
    rm_vardir(e->vardir);
}

int main(int argc, char **argv) {
    int n_envs    = argc > 1 ? atoi(argv[1]) : 64;
    int n_steps   = argc > 2 ? atoi(argv[2]) : 500;
    int n_threads = argc > 3 ? atoi(argv[3]) : 4;

    const char *hackdir = getenv("NETHACKDIR");
    if (!hackdir) hackdir = "src/build/dat";

    char abs_hackdir[512];
    if (hackdir[0] != '/') {
        char cwd[256];
        assert(getcwd(cwd, sizeof(cwd)));
        snprintf(abs_hackdir, sizeof(abs_hackdir), "%s/%s", cwd, hackdir);
        hackdir = abs_hackdir;
    }

    printf("test_multi_env: N=%d steps=%d threads=%d\n", n_envs, n_steps, n_threads);
    printf("sizeof(nle_settings) = %zu bytes (%.1f KB)\n",
           sizeof(nle_settings), sizeof(nle_settings) / 1024.0);

    omp_set_num_threads(n_threads);

    TestEnv *envs = calloc(n_envs, sizeof(TestEnv));
    assert(envs);

    for (int i = 0; i < n_envs; i++)
        env_init(&envs[i], i, hackdir);

    printf("Initializing %d envs...\n", n_envs);
    double t0 = omp_get_wtime();

    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < n_envs; i++)
        env_start(&envs[i]);

    double t1 = omp_get_wtime();
    printf("Init: %.2fs (%.0f envs/sec)\n", t1 - t0, n_envs / (t1 - t0));

    int actions[] = {'k','j','h','l','y','u','b','n','.','s'};
    int n_actions = sizeof(actions) / sizeof(actions[0]);

    printf("Running %d steps per env...\n", n_steps);
    double t2 = omp_get_wtime();
    long total_steps = 0;
    long total_resets = 0;

    #pragma omp parallel for schedule(dynamic) reduction(+:total_steps,total_resets)
    for (int i = 0; i < n_envs; i++) {
        TestEnv *e = &envs[i];
        unsigned int rng = (unsigned int)(i * 7 + 1);
        for (int s = 0; s < n_steps; s++) {
            rng = rng * 1103515245 + 12345;
            int act = actions[rng % n_actions];
            env_step(e, act);
            total_steps++;
            if (e->obs.done) {
                e->total_score += e->blstats[NLE_BL_SCORE];
                e->episodes++;
                total_resets++;
                env_end(e);
                env_bind_obs(e);
                env_start(e);
            }
        }
    }

    double t3 = omp_get_wtime();
    double elapsed = t3 - t2;
    printf("Done: %ld steps in %.2fs = %.0f SPS (%ld resets)\n",
           total_steps, elapsed, total_steps / elapsed, total_resets);

    long total_score = 0;
    int total_episodes = 0;
    for (int i = 0; i < n_envs; i++) {
        total_score += envs[i].total_score;
        total_episodes += envs[i].episodes;
    }
    if (total_episodes > 0)
        printf("Episodes: %d, avg score: %.1f\n", total_episodes,
               (double)total_score / total_episodes);

    printf("Cleaning up...\n");
    for (int i = 0; i < n_envs; i++)
        env_close(&envs[i]);
    free(envs);

    printf("PASS\n");
    return 0;
}
