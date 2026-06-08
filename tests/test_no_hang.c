/* test_no_hang.c — proves NLE makes forward progress over time under scale.
 *
 * Drives N envs across T worker threads (round-robin, one step per env per
 * pass) plus a dedicated monitor thread that polls the sentinel snapshot once
 * per second. If any live env's heartbeat fails to advance for STALL seconds,
 * the test prints the offending env and exits 1 (graceful failure, no abort).
 * If every env keeps advancing for the whole DURATION, it exits 0.
 *
 * Tiers:
 *   (default)  N=64   T=8   DURATION=30s   STALL=5s
 *   --soak     N=1024 T=16  DURATION=180s  STALL=10s  (override secs: --soak <secs>)
 *
 * Links libnethack (uses the sentinel symbols exported from it). Do NOT also
 * compile nle_sentinel.c — that would create a second, separate registry.
 *
 * Build: see tests/run_tests.sh
 * Run:   NETHACKDIR=$PWD/src/build/dat ./tests/test_no_hang [--soak [secs]]
 */
#define _GNU_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <omp.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nleobs.h"
#include "nle_sentinel.h"

typedef struct nle_ctx_t nle_ctx_t;
extern nle_ctx_t *nle_start(nle_obs *, FILE *, nle_seeds_init_t *, nle_settings *);
extern nle_ctx_t *nle_step(nle_ctx_t *, nle_obs *);
extern void nle_end(nle_ctx_t *);

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
} Env;

static void touch(const char *p) { int fd = open(p, O_CREAT | O_WRONLY, 0644); if (fd >= 0) close(fd); }

static void make_vardir(const char *hackdir, char *out, size_t cap) {
    char tmpl[] = "/tmp/nle-soak-XXXXXX";
    char *d = mkdtemp(tmpl); assert(d);
    snprintf(out, cap, "%s", d);
    char src[512], dst[512];
    snprintf(src, sizeof(src), "%s/nhdat", hackdir);
    snprintf(dst, sizeof(dst), "%s/nhdat", d);
    assert(symlink(src, dst) == 0);
    const char *files[] = {"perm", "record", "logfile", "xlogfile"};
    for (int i = 0; i < 4; i++) { snprintf(dst, sizeof(dst), "%s/%s", d, files[i]); touch(dst); }
    snprintf(dst, sizeof(dst), "%s/save", d); mkdir(dst, 0755);
}

static void env_bind(Env *e) {
    e->obs.glyphs = e->glyphs; e->obs.chars = e->chars; e->obs.colors = e->colors;
    e->obs.blstats = e->blstats; e->obs.message = e->message;
    e->obs.misc = e->misc; e->obs.internal = e->internal;
}

static void env_init(Env *e, int idx, const char *hackdir) {
    memset(e, 0, sizeof(*e));
    make_vardir(hackdir, e->vardir, sizeof(e->vardir));
    strncpy(e->settings.hackdir, e->vardir, sizeof(e->settings.hackdir) - 1);
    strncpy(e->settings.options,
            "name:Agent-mon-hum-neu-mal,autopickup,color,"
            "disclose:+i +a +v +g +c +o,nobones,!status_updates",
            sizeof(e->settings.options) - 1);
    e->settings.spawn_monsters = 1;
    env_bind(e);
    e->seed_a = 0xCAFEBEEF + (unsigned long)idx;
    e->seed_b = (0xCAFEBEEF ^ 0x9E3779B97F4A7C15UL) + (unsigned long)idx;
}

static void env_start(Env *e) {
    e->seed_a = e->seed_a * 6364136223846793005UL + 1442695040888963407UL;
    e->seed_b = e->seed_b * 6364136223846793005UL + 1442695040888963407UL;
    nle_seeds_init_t seeds; memset(&seeds, 0, sizeof(seeds));
    seeds.seeds[0] = e->seed_a; seeds.seeds[1] = e->seed_b; seeds.reseed = 0;
    e->obs.action = 0; e->obs.done = 0;
    e->ctx = nle_start(&e->obs, NULL, &seeds, &e->settings);
    assert(e->ctx);
}

static atomic_int g_stop = 0;
static atomic_int g_failed = 0;

int main(int argc, char **argv) {
    int n_envs = 64, n_threads = 8, duration = 30, stall = 5;
    if (argc > 1 && strcmp(argv[1], "--soak") == 0) {
        n_envs = 1024; n_threads = 16; duration = 180; stall = 10;
        if (argc > 2) duration = atoi(argv[2]);
    }

    const char *hackdir = getenv("NETHACKDIR");
    if (!hackdir) hackdir = "src/build/dat";
    char abs_hackdir[512];
    if (hackdir[0] != '/') {
        char cwd[256]; assert(getcwd(cwd, sizeof(cwd)));
        snprintf(abs_hackdir, sizeof(abs_hackdir), "%s/%s", cwd, hackdir);
        hackdir = abs_hackdir;
    }

    printf("test_no_hang: N=%d threads=%d duration=%ds stall_threshold=%ds\n",
           n_envs, n_threads, duration, stall);

    nle_sentinel_global_init();
    omp_set_dynamic(0);

    Env *envs = calloc(n_envs, sizeof(Env));
    assert(envs);
    for (int i = 0; i < n_envs; i++) env_init(&envs[i], i, hackdir);
    #pragma omp parallel for schedule(dynamic) num_threads(n_threads)
    for (int i = 0; i < n_envs; i++) env_start(&envs[i]);

    int actions[] = {'k','j','h','l','y','u','b','n','.','s'};
    int n_actions = (int)(sizeof(actions) / sizeof(actions[0]));

    double t_start = omp_get_wtime();

    #pragma omp parallel num_threads(n_threads + 1)
    {
        int tid = omp_get_thread_num();
        if (tid == n_threads) {
            /* dedicated monitor thread: poll snapshots once per second */
            nle_sentinel_stat *cur = calloc(n_envs + 16, sizeof(nle_sentinel_stat));
            uint64_t *prev_hb = calloc(NLE_SENTINEL_CAP, sizeof(uint64_t));
            int *stalled = calloc(NLE_SENTINEL_CAP, sizeof(int));
            while (omp_get_wtime() - t_start < duration && !atomic_load(&g_failed)) {
                sleep(1);
                int n = nle_sentinel_snapshot(cur, n_envs + 16);
                for (int i = 0; i < n; i++) {
                    int id = cur[i].env_id;
                    if (cur[i].heartbeat == prev_hb[id]) {
                        if (++stalled[id] >= stall) {
                            fprintf(stderr,
                                "FAIL: env id=%d seed=0x%lx stalled %ds (heartbeat frozen at %llu) "
                                "step=%llu action=%d dlvl=%d\n",
                                id, cur[i].seed, stalled[id],
                                (unsigned long long)cur[i].heartbeat,
                                (unsigned long long)cur[i].step, cur[i].last_action, cur[i].dlvl);
                            atomic_store(&g_failed, 1);
                            break;
                        }
                    } else {
                        stalled[id] = 0;
                        prev_hb[id] = cur[i].heartbeat;
                    }
                }
            }
            atomic_store(&g_stop, 1);
            free(cur); free(prev_hb); free(stalled);
        } else {
            /* worker: round-robin over a strided subset, one step per env per pass */
            unsigned int rng = (unsigned int)(tid * 7 + 1);
            while (!atomic_load(&g_stop) && !atomic_load(&g_failed)) {
                for (int i = tid; i < n_envs; i += n_threads) {
                    rng = rng * 1103515245 + 12345;
                    envs[i].obs.action = actions[rng % n_actions];
                    envs[i].ctx = nle_step(envs[i].ctx, &envs[i].obs);
                    if (envs[i].obs.done) {
                        nle_end(envs[i].ctx);
                        env_bind(&envs[i]);
                        env_start(&envs[i]);
                    }
                }
            }
        }
    }

    atomic_store(&g_stop, 1);

    /* summary */
    nle_sentinel_stat *snap = calloc(n_envs + 16, sizeof(nle_sentinel_stat));
    int n = nle_sentinel_snapshot(snap, n_envs + 16);
    uint64_t total = 0, mn = (uint64_t)-1, mx = 0;
    for (int i = 0; i < n; i++) {
        total += snap[i].step;
        if (snap[i].step < mn) mn = snap[i].step;
        if (snap[i].step > mx) mx = snap[i].step;
    }
    double elapsed = omp_get_wtime() - t_start;
    printf("envs_live=%d total_steps=%llu min_steps=%llu max_steps=%llu steps/sec=%.0f elapsed=%.1fs\n",
           n, (unsigned long long)total, (unsigned long long)(n ? mn : 0),
           (unsigned long long)mx, total / (elapsed > 0 ? elapsed : 1), elapsed);
    free(snap);

    if (atomic_load(&g_failed)) { printf("test_no_hang: FAIL\n"); return 1; }
    printf("test_no_hang: PASS\n");
    return 0;
}
