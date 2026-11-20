#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "log.h"

static FILE *logFile;

static void logWrite(const char *level, const char *fmt, va_list ap)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    FILE *f = (logFile != NULL) ? logFile : fopen("data/game.log", "a");

    if (f == NULL) return;
    if (t != NULL) {
        fprintf(f, "%04d-%02d-%02d %02d:%02d:%02d [%s] ",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec, level);
    } else {
        fprintf(f, "[%s] ", level);
    }
    vfprintf(f, fmt, ap);
    fputc('\n', f);
    fflush(f);
    if (logFile == NULL) fclose(f);
}

void log_init(void)
{
    if (logFile == NULL) logFile = fopen("data/game.log", "a");
}

void log_close(void)
{
    if (logFile != NULL) {
        fclose(logFile);
        logFile = NULL;
    }
}

void log_event(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    logWrite("INFO", fmt, ap);
    va_end(ap);
}

void log_warn(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    logWrite("WARN", fmt, ap);
    va_end(ap);
}

void log_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    logWrite("ERROR", fmt, ap);
    va_end(ap);
}

void log_crash(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    logWrite("CRASH", fmt, ap);
    va_end(ap);
}
