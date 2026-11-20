#include <signal.h>
#include <stdio.h>

#include "crash.h"
#include "log.h"
#include "term.h"

static void onCrash(int sig)
{
    term_shutdown();
    log_crash("crash: sinyal %d", sig);
    signal(sig, SIG_DFL);
    raise(sig);
}

void crash_install(void)
{
    signal(SIGSEGV, onCrash);
    signal(SIGABRT, onCrash);
    signal(SIGFPE, onCrash);
}
