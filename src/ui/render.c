#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "render.h"
#include "caps.h"
#include "style.h"

static Cell grid[GRID_MAX_ROWS][GRID_MAX_COLS];
static int maxRowUsed;
static char output[262144];
static size_t outLen;

static unsigned int decodeUtf8(const char **p)
{
    const unsigned char *s = (const unsigned char *)*p;
    unsigned int cp;
    int extra;

    if (s[0] < 0x80) { *p += 1; return s[0]; }
    if ((s[0] & 0xE0) == 0xC0) { cp = s[0] & 0x1F; extra = 1; }
    else if ((s[0] & 0xF0) == 0xE0) { cp = s[0] & 0x0F; extra = 2; }
    else if ((s[0] & 0xF8) == 0xF0) { cp = s[0] & 0x07; extra = 3; }
    else { *p += 1; return '?'; }
    if (s[1] == 0 || (s[1] & 0xC0) != 0x80) { *p += 1; return '?'; }
    cp = (cp << 6) | (s[1] & 0x3F);
    if (extra >= 2) {
        if (s[2] == 0 || (s[2] & 0xC0) != 0x80) { *p += 1; return '?'; }
        cp = (cp << 6) | (s[2] & 0x3F);
    }
    if (extra >= 3) {
        if (s[3] == 0 || (s[3] & 0xC0) != 0x80) { *p += 1; return '?'; }
        cp = (cp << 6) | (s[3] & 0x3F);
    }
    *p += extra + 1;
    return cp;
}

static int cpWidth(unsigned int cp)
{
    if (!caps_unicode()) return 1;
    if (cp >= 0x1100 && (cp <= 0x115F || cp == 0x2329 || cp == 0x232A ||
        (cp >= 0x2E80 && cp <= 0xA4CF) || (cp >= 0xAC00 && cp <= 0xD7A3) ||
        (cp >= 0xF900 && cp <= 0xFAFF) || (cp >= 0xFE10 && cp <= 0xFE6F) ||
        (cp >= 0xFF00 && cp <= 0xFF60) || (cp >= 0xFFE0 && cp <= 0xFFE6))) return 2;
    return 1;
}

void render_init(void)
{
    render_clear();
}

void render_clear(void)
{
    memset(grid, 0, sizeof(grid));
    maxRowUsed = 0;
}

void render_char(int row, int col, Style s, unsigned int cp)
{
    int w;
    if (row < 0 || row >= GRID_MAX_ROWS || col < 0 || col >= GRID_MAX_COLS) return;
    if (cp == 0) return;
    grid[row][col].cp = cp;
    grid[row][col].st = s;
    if (row + 1 > maxRowUsed) maxRowUsed = row + 1;
    w = cpWidth(cp);
    if (w > 1 && col + 1 < GRID_MAX_COLS) {
        grid[row][col + 1].cp = ' ';
        grid[row][col + 1].st = s;
    }
}

void render_text_clipped(int row, int col, int maxW, Style s, const char *fmt, ...)
{
    char buf[1024];
    const char *p;
    va_list ap;
    int x = col;

    if (fmt == NULL) return;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    p = buf;
    while (*p != '\0' && x < col + maxW && x < GRID_MAX_COLS) {
        unsigned int cp = decodeUtf8(&p);
        int w = cpWidth(cp);
        if (x + w > col + maxW) break;
        render_char(row, x, s, cp);
        x += w;
    }
}

void render_text(int row, int col, Style s, const char *fmt, ...)
{
    char buf[1024];
    va_list ap;

    if (fmt == NULL) return;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    render_text_clipped(row, col, GRID_MAX_COLS - col, s, "%s", buf);
}

int render_span(int row, int col, int maxW, Style base, Style alt, const char *text)
{
    Style current = base;
    const char *p = text;
    int x = col;

    if (text == NULL) return 0;
    while (*p != '\0' && x < col + maxW && x < GRID_MAX_COLS) {
        if (*p == '~') {
            current = (current == base) ? alt : base;
            ++p;
            continue;
        }
        {
            unsigned int cp = decodeUtf8(&p);
            render_char(row, x, current, cp);
            x += cpWidth(cp);
        }
    }
    return x - col;
}

int ui_visible_len(const char *s)
{
    int n = 0;
    if (s == NULL) return 0;
    while (*s != '\0') {
        if (*s == '\033') {
            while (*s != '\0' && *s != 'm') ++s;
            if (*s == 'm') ++s;
            continue;
        }
        ++n;
        ++s;
    }
    return n;
}

static void appendText(const char *s)
{
    size_t n = strlen(s);
    if (outLen + n >= sizeof(output)) n = sizeof(output) - outLen - 1;
    memcpy(output + outLen, s, n);
    outLen += n;
    output[outLen] = '\0';
}

static void appendCodepoint(unsigned int cp)
{
    char tmp[8];
    int n = 0;
    if (cp < 0x80) tmp[n++] = (char)cp;
    else if (cp < 0x800) {
        tmp[n++] = (char)(0xC0 | (cp >> 6));
        tmp[n++] = (char)(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        tmp[n++] = (char)(0xE0 | (cp >> 12));
        tmp[n++] = (char)(0x80 | ((cp >> 6) & 0x3F));
        tmp[n++] = (char)(0x80 | (cp & 0x3F));
    } else {
        tmp[n++] = (char)(0xF0 | (cp >> 18));
        tmp[n++] = (char)(0x80 | ((cp >> 12) & 0x3F));
        tmp[n++] = (char)(0x80 | ((cp >> 6) & 0x3F));
        tmp[n++] = (char)(0x80 | (cp & 0x3F));
    }
    tmp[n] = '\0';
    appendText(tmp);
}

void render_flush(void)
{
    int r, c, last;
    Style used = ST_TEXT;
    Style current;

    outLen = 0;
    output[0] = '\0';
    if (caps_interactive()) appendText("\033[H");
    for (r = 0; r < maxRowUsed; ++r) {
        last = GRID_MAX_COLS - 1;
        while (last >= 0 && grid[r][last].cp == 0) --last;
        if (last < 0) {
            appendText("\n");
            continue;
        }
        current = ST_TEXT;
        used = ST_TEXT;
        if (caps_interactive()) appendText("\033[0m");
        for (c = 0; c <= last; ++c) {
            unsigned int cp = grid[r][c].cp;
            if (caps_interactive() && grid[r][c].st != current) {
                current = grid[r][c].st;
                used = current;
                appendText("\033[0m");
                appendText(style_bg(used));
                appendText(style_fg(used));
            }
            if (cp == 0) appendText(" ");
            else appendCodepoint(cp);
        }
        if (caps_interactive()) appendText("\033[0m");
        appendText("\r\n");
    }
    if (caps_interactive()) {
        char clear[32];
        snprintf(clear, sizeof(clear), "\033[%d;%dH", GRID_MAX_ROWS, 1);
        appendText(clear);
    }
    fwrite(output, 1, outLen, stdout);
    fflush(stdout);
}

int render_max_row(void)
{
    return maxRowUsed;
}

unsigned int render_cell_cp(int row, int col)
{
    if (row < 0 || row >= GRID_MAX_ROWS || col < 0 || col >= GRID_MAX_COLS) return 0;
    return grid[row][col].cp;
}
