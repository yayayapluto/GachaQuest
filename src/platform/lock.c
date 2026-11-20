#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <process.h>
#define platform_pid() ((int)_getpid())
#else
#include <unistd.h>
#define platform_pid() ((int)getpid())
#endif

#include "lock.h"

#define LOCK_PATH "data/session.lock"

static int previousRunning;
static int held;

static int readRunning(const char *path)
{
    char buf[128];
    FILE *f = fopen(path, "r");
    size_t n;
    if (f == NULL) return 0;
    n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    fclose(f);
    return strncmp(buf, "RUNNING", 7) == 0;
}

int lock_acquire(void)
{
    FILE *f;

    previousRunning = readRunning(LOCK_PATH);
    f = fopen(LOCK_PATH, "wb");
    if (f == NULL) return 0;
    fprintf(f, "RUNNING pid=%d start=%ld\n", platform_pid(), (long)time(NULL));
    fclose(f);
    held = 1;
    return 1;
}

int lock_is_contention(void)
{
    return previousRunning || readRunning(LOCK_PATH);
}

int lock_previous_running(void)
{
    return previousRunning;
}

void lock_mark_clean(void)
{
    FILE *f;
    if (!held) return;
    f = fopen(LOCK_PATH, "wb");
    if (f == NULL) return;
    fprintf(f, "CLEAN\n");
    fclose(f);
    held = 0;
}
