#include <signal.h>
#include <stdio.h>

#include "sig.h"

static volatile sig_atomic_t shutdownFlag;
static volatile sig_atomic_t cleanupFlag;

#ifdef _WIN32
int sig_wake_fd(void)
{
    return -1;
}
#else
#include <unistd.h>
static int wakePipe[2] = { -1, -1 };

int sig_wake_fd(void)
{
    return wakePipe[0];
}
#endif

static void onSignal(int sig)
{
    (void)sig;
    if (shutdownFlag) return;
    shutdownFlag = 1;
#ifndef _WIN32
    if (wakePipe[1] >= 0) {
        const char byte = 'x';
        (void)write(wakePipe[1], &byte, 1);
    }
#endif
}

void sig_install(void)
{
    shutdownFlag = 0;
    cleanupFlag = 0;
    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);
#ifndef _WIN32
    if (pipe(wakePipe) != 0) {
        wakePipe[0] = -1;
        wakePipe[1] = -1;
    }
#endif
}

int sig_requested(void)
{
    return shutdownFlag != 0;
}

void sig_cleanup_done(void)
{
    cleanupFlag = 1;
}
