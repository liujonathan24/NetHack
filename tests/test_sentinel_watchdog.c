/* Child sets NLE_WATCHDOG_SECS=1, registers an env, beats a few times, then
 * stops beating (sleeps). The watchdog must detect the stall and abort the
 * process. Parent asserts the child died of SIGABRT and a breadcrumb mentioning
 * a hang/stall was written. */
#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "nle_sentinel.h"

int main(void) {
    char dir[] = "/tmp/nle-wd-XXXXXX";
    assert(mkdtemp(dir));
    setenv("NLE_CRASH_DIR", dir, 1);
    setenv("NLE_WATCHDOG_SECS", "1", 1);

    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        nle_sentinel_global_init();
        void *s = nle_sentinel_register(0x1234);
        for (int i = 0; i < 3; i++) { nle_sentinel_beat(s, 'k', 1); usleep(100000); }
        for (;;) pause(); /* stop beating -> hang */
    }

    int status = 0;
    waitpid(pid, &status, 0);
    assert(WIFSIGNALED(status));
    assert(WTERMSIG(status) == SIGABRT); /* watchdog aborted */

    char path[512], buf[8192];
    snprintf(path, sizeof(path), "%s/nle_crash_%d.txt", dir, pid);
    FILE *f = fopen(path, "r");
    assert(f && "watchdog breadcrumb not written");
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    fclose(f);
    assert(strstr(buf, "HANG") || strstr(buf, "SIGABRT"));

    printf("test_sentinel_watchdog: PASS\n");
    return 0;
}
