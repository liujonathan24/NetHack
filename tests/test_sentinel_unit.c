/* Unit test for the sentinel registry. Compiles nle_sentinel.c directly,
 * so it needs no libnethack and no NetHack data files. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "nle_sentinel.h"

int main(void) {
    /* register two envs */
    void *a = nle_sentinel_register(1111);
    void *b = nle_sentinel_register(2222);
    assert(a && b && a != b);

    /* beats advance step + heartbeat */
    nle_sentinel_beat(a, 'k', 1);
    nle_sentinel_beat(a, 'j', 2);
    nle_sentinel_beat(b, 'h', 1);
    assert(nle_sentinel_total_heartbeat() == 3);

    /* snapshot reflects state */
    nle_sentinel_stat stats[8];
    int n = nle_sentinel_snapshot(stats, 8);
    assert(n == 2);
    int seen_a = 0, seen_b = 0;
    for (int i = 0; i < n; i++) {
        if (stats[i].seed == 1111) { seen_a = 1; assert(stats[i].step == 2); assert(stats[i].dlvl == 2); assert(stats[i].last_action == 'j'); }
        if (stats[i].seed == 2222) { seen_b = 1; assert(stats[i].step == 1); }
    }
    assert(seen_a && seen_b);

    /* NULL slot is a no-op (no crash) */
    nle_sentinel_beat(NULL, 0, 0);

    /* unregister frees the slot */
    nle_sentinel_unregister(a);
    n = nle_sentinel_snapshot(stats, 8);
    assert(n == 1 && stats[0].seed == 2222);

    printf("test_sentinel_unit: PASS\n");
    return 0;
}
