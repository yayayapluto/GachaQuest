#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "util.h"
#include "render.h"
#include "caps.h"
#include "term.h"
#include "platform.h"
#include "autosave.h"

int readLine(char *buf, int max)
{
    int ch;
    int n = 0;

    if (buf == NULL || max <= 0) return 0;
    while ((ch = getchar()) != EOF && ch != '\n') {
        if (ch == '\r') continue;
        if (n < max - 1) buf[n++] = (char)ch;
    }
    buf[n] = '\0';
    return n;
}

int readLineEsc(char *buf, int max)
{
    int ch;
    int n = 0;

    if (buf == NULL || max <= 0) return 0;
    while ((ch = getchar()) != EOF && ch != '\n') {
        if (ch == 27) {
            buf[0] = '\0';
            return -1;
        }
        if (ch == '\r') continue;
        if (n < max - 1) buf[n++] = (char)ch;
    }
    buf[n] = '\0';
    if (ch == EOF && n == 0) return 0;
    return 1;
}

static int parseRange(const char *s, int min, int max, int *out)
{
    char *end;
    long value;
    const char *p = s;

    while (*p != '\0' && isspace((unsigned char)*p)) ++p;
    if (*p == '\0') return 0;
    errno = 0;
    value = strtol(p, &end, 10);
    if (errno != 0 || end == p) return 0;
    while (*end != '\0' && isspace((unsigned char)*end)) ++end;
    if (*end != '\0') return 0;
    if (value < (long)min || value > (long)max) return 0;
    if (value < (long)INT_MIN || value > (long)INT_MAX) return 0;
    *out = (int)value;
    return 1;
}

int readIntTry(const char *prompt, int min, int max, int *out)
{
    char buf[128];

    if (out == NULL) return 0;
    for (;;) {
        if (prompt != NULL) printf("%s", prompt);
        fflush(stdout);
        int status = readLineEsc(buf, (int)sizeof(buf));
        if (status <= 0) return 0;
        if (parseRange(buf, min, max, out)) return 1;
        printf("Input tidak valid. Masukkan angka %d sampai %d.\n", min, max);
    }
}

int readInt(const char *prompt, int min, int max)
{
    int value = min;
    if (!readIntTry(prompt, min, max, &value)) return min;
    return value;
}

int readStringTry(const char *prompt, char *out, int max)
{
    if (prompt != NULL) printf("%s", prompt);
    fflush(stdout);
    return readLineEsc(out, max) > 0 ? 1 : 0;
}

void readString(const char *prompt, char *out, int max)
{
    for (;;) {
        if (!readStringTry(prompt, out, max)) {
            if (out != NULL && max > 0) out[0] = '\0';
            return;
        }
        if (out != NULL && out[0] != '\0') {
            if (max > MAX_NAME) out[MAX_NAME - 1] = '\0';
            return;
        }
    }
}

int confirmPrompt(const char *prompt)
{
    char buf[32];

    if (prompt != NULL) printf("%s", prompt);
    fflush(stdout);
    if (readLineEsc(buf, (int)sizeof(buf)) <= 0) return 0;
    return (buf[0] == 'y' || buf[0] == 'Y') ? 1 : 0;
}

void clearScreen(void)
{
    if (caps_interactive()) {
        printf("\033[2J\033[H");
    } else {
        putchar('\n');
    }
    fflush(stdout);
}

void printLine(void)
{
    int i;
    int width = caps_cols();
    if (width < UI_COLS) width = UI_COLS;
    if (width > 120) width = 120;
    for (i = 0; i < width; ++i) putchar('-');
    putchar('\n');
    fflush(stdout);
}

void shutdownGame(void)
{
    autosave_stop();
    term_shutdown();
    platform_shutdown();
    exit(0);
}
