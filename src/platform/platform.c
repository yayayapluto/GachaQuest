#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define make_data_dir() _mkdir("data")
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define make_data_dir() mkdir("data", 0755)
#endif

#include "platform.h"
#include "atomicfile.h"
#include "log.h"
#include "lock.h"
#include "sig.h"
#include "crash.h"
#include "autosave.h"

static int shutdownDone;

void platform_init(void)
{
    make_data_dir();
    file_cleanup_tmp("data");
    log_init();
    if (!lock_acquire()) {
        printf("Gacha Quest sudah berjalan di instance lain. Tutup instance tersebut dulu.\n");
        exit(1);
    }
    log_event("sesi dimulai");
    sig_install();
    crash_install();
    autosave_start();
    shutdownDone = 0;
}

int platform_previous_running(void)
{
    return lock_previous_running();
}

int platform_shutdown_requested(void)
{
    return sig_requested();
}

void platform_shutdown(void)
{
    if (shutdownDone) return;
    shutdownDone = 1;
    autosave_stop();
    sig_cleanup_done();
    log_event("sesi ditutup");
    lock_mark_clean();
    log_close();
}

void platform_sleep_ms(int ms)
{
    if (ms <= 0) return;
#ifdef _WIN32
    Sleep((DWORD)ms);
#else
    {
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (long)(ms % 1000) * 1000000L;
        nanosleep(&ts, NULL);
    }
#endif
}
