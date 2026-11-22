#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#define platform_pid() ((int)_getpid())
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#define platform_pid() ((int)getpid())
#endif

#include "lock.h"

#define LOCK_PATH "data/session.lock"
#define GUARD_PATH "data/session.run"

static int previousRunning;
static int held;

#ifdef _WIN32
static HANDLE guardHandle = INVALID_HANDLE_VALUE;
#else
static int guardFd = -1;
#endif

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

static int writeStatus(const char *text)
{
    FILE *f = fopen(LOCK_PATH, "wb");
    if (f == NULL) return 0;
    fputs(text, f);
    fclose(f);
    return 1;
}

int lock_acquire(void)
{
    char line[128];

    previousRunning = readRunning(LOCK_PATH);
    snprintf(line, sizeof line, "RUNNING pid=%d start=%ld\n",
             platform_pid(), (long)time(NULL));
#ifdef _WIN32
    guardHandle = CreateFileA("data\\session.run", GENERIC_READ | GENERIC_WRITE, 0,
                              NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (guardHandle == INVALID_HANDLE_VALUE) return 0;
#else
    guardFd = open(GUARD_PATH, O_RDWR | O_CREAT, 0644);
    if (guardFd < 0) return 0;
    if (flock(guardFd, LOCK_EX | LOCK_NB) != 0) {
        close(guardFd);
        guardFd = -1;
        return 0;
    }
#endif
    held = 1;
    if (!writeStatus(line)) {
        lock_mark_clean();
        return 0;
    }
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
    if (!held) return;
    writeStatus("CLEAN\n");
#ifdef _WIN32
    if (guardHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(guardHandle);
        guardHandle = INVALID_HANDLE_VALUE;
    }
#else
    if (guardFd >= 0) {
        close(guardFd);
        guardFd = -1;
    }
#endif
    held = 0;
}
