#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif

#include "caps.h"
#include "render.h"

static ColorMode colorMode = COLOR_MONO;
static int unicodeOn;
static int interactive;
static int cols = UI_COLS;
static int rows = UI_ROWS;

static const Glyphs asciiGlyphs = {
    "+", "+", "+", "+", "-", "|", "#", ".", ">", "^ v"
};
static const Glyphs unicodeGlyphs = {
    "\xe2\x94\x8c", "\xe2\x94\x90", "\xe2\x94\x94", "\xe2\x94\x98",
    "\xe2\x94\x80", "\xe2\x94\x82", "\xe2\x96\x88", "\xe2\x96\x91",
    "\xe2\x96\xb6", "\xe2\x86\x95\xe2\x86\x93"
};

static int isInteractive(void)
{
#ifdef _WIN32
    return _isatty(_fileno(stdin)) && _isatty(_fileno(stdout));
#else
    return isatty(0) && isatty(1);
#endif
}

static void querySize(void)
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        cols = info.srWindow.Right - info.srWindow.Left + 1;
        rows = info.srWindow.Bottom - info.srWindow.Top + 1;
    }
#else
    {
        struct winsize ws;
        if (ioctl(1, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
            cols = ws.ws_col;
            rows = ws.ws_row;
        }
    }
#endif
    if (cols < MIN_COLS) cols = MIN_COLS;
    if (rows < MIN_ROWS) rows = MIN_ROWS;
    if (cols > GRID_MAX_COLS) cols = GRID_MAX_COLS;
    if (rows > GRID_MAX_ROWS) rows = GRID_MAX_ROWS;
}

void caps_detect(void)
{
    const char *term = getenv("TERM");

    interactive = isInteractive();
    if (!interactive || term == NULL || strcmp(term, "dumb") == 0) {
        colorMode = COLOR_MONO;
        unicodeOn = 0;
    } else {
        if (strstr(term, "truecolor") != NULL) colorMode = COLOR_TRUE;
        else if (strstr(term, "256") != NULL) colorMode = COLOR_256;
        else colorMode = COLOR_16;
        unicodeOn = 1;
    }
    querySize();
}

ColorMode caps_color(void)
{
    return colorMode;
}

int caps_unicode(void)
{
    return unicodeOn;
}

int caps_interactive(void)
{
    return interactive;
}

int caps_cols(void)
{
    return cols;
}

int caps_rows(void)
{
    return rows;
}

void caps_set_size(int c, int r)
{
    if (c >= MIN_COLS && c <= GRID_MAX_COLS) cols = c;
    if (r >= MIN_ROWS && r <= GRID_MAX_ROWS) rows = r;
}

const Glyphs *caps_glyphs(void)
{
    return unicodeOn ? &unicodeGlyphs : &asciiGlyphs;
}
