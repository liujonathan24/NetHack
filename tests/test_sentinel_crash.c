/* A child process installs the sentinel, registers an env, beats once, then
 * dereferences NULL. The parent verifies the child died of SIGSEGV AND that an
 * attributed breadcrumb file was written naming the env's seed. */
#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "nle_sentinel.h"

int main(void) {
    char dir[] = "/tmp/nle-crash-XXXXXX";
    assert(mkdtemp(dir));
    setenv("NLE_CRASH_DIR", dir, 1);

    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        nle_sentinel_global_init();
        void *s = nle_sentinel_register(0xABCDEF);
        nle_sentinel_beat(s, 'k', 7);
        volatile int *p = (volatile int *)0;
        *p = 1;            /* SIGSEGV */
        _exit(0);          /* unreachable */
    }

    int status = 0;
    waitpid(pid, &status, 0);
    assert(WIFSIGNALED(status));
    assert(WTERMSIG(status) == SIGSEGV); /* re-raised, exit semantics preserved */

    /* find the breadcrumb and check it names the seed (hex ABCDEF) */
    char path[512], buf[8192];
    snprintf(path, sizeof(path), "%s/nle_crash_%d.txt", dir, pid);
    FILE *f = fopen(path, "r");
    assert(f && "breadcrumb file not written");
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    fclose(f);
    assert(strstr(buf, "abcdef") || strstr(buf, "ABCDEF")); /* seed attribution */
    assert(strstr(buf, "SIGSEGV"));

    printf("test_sentinel_crash: PASS\n");
    return 0;
}
