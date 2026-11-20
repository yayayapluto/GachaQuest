#include <stdio.h>
#include <string.h>

#include "widgets.h"
#include "render.h"
#include "caps.h"
#include "style.h"

static int footerRow = -1;

static void horizontal(int row, int col, int w, const char *glyph)
{
    char line[256];
    int i, n = 0;
    if (w <= 0 || glyph == NULL) return;
    for (i = 0; i < w && n < (int)sizeof(line) - 8; ++i) {
        int k;
        for (k = 0; glyph[k] != '\0'; ++k) line[n++] = glyph[k];
    }
    line[n] = '\0';
    render_text(row, col, ST_BORDER, "%s", line);
}

void widget_panel(int row, int col, int w, int h, const char *title)
{
    const Glyphs *g = caps_glyphs();
    int i;
    if (w < 4 || h < 3) return;
    render_text(row, col, ST_BORDER, "%s", g->tl);
    horizontal(row, col + 1, w - 2, g->hz);
    render_text(row, col + w - 1, ST_BORDER, "%s", g->tr);
    if (title != NULL && title[0] != '\0') {
        render_text_clipped(row, col + 2, w - 4, ST_TITLE, " %s ", title);
    }
    for (i = 1; i < h - 1; ++i) {
        render_text(row + i, col, ST_BORDER, "%s", g->vr);
        render_text(row + i, col + w - 1, ST_BORDER, "%s", g->vr);
    }
    render_text(row + h - 1, col, ST_BORDER, "%s", g->bl);
    horizontal(row + h - 1, col + 1, w - 2, g->hz);
    render_text(row + h - 1, col + w - 1, ST_BORDER, "%s", g->br);
}

void widget_bar(int row, int col, int w, int cur, int max, Style fill)
{
    const Glyphs *g = caps_glyphs();
    int filled = 0;
    int i;
    if (w <= 0) return;
    if (max > 0) {
        filled = (cur * w) / max;
        if (filled < 0) filled = 0;
        if (filled > w) filled = w;
    }
    for (i = 0; i < w; ++i) {
        render_text(row, col + i, (i < filled) ? fill : ST_DIM,
                    (i < filled) ? g->blockFull : g->blockEmpty);
    }
}

void widget_hp_bar(int row, int col, int w, int hp, int maxHp)
{
    widget_bar(row, col, w, hp, maxHp, style_hp(hp, maxHp));
}

void widget_hp_bar_n(int row, int col, int w, int hp, int maxHp)
{
    widget_hp_bar(row, col, w, hp, maxHp);
}

void widget_menu_v(int row, int col, const char *const *items, const int *nums,
                   int n, int sel)
{
    const Glyphs *g = caps_glyphs();
    int i;
    for (i = 0; i < n; ++i) {
        Style st = (i == sel) ? ST_SELECTED : ST_TEXT;
        if (nums != NULL) {
            render_text(row + i, col, st, "%s %2d. %s",
                        (i == sel) ? g->marker : " ", nums[i],
                        (items != NULL && items[i] != NULL) ? items[i] : "");
        } else {
            render_text(row + i, col, st, "%s %s",
                        (i == sel) ? g->marker : " ",
                        (items != NULL && items[i] != NULL) ? items[i] : "");
        }
    }
}

void widget_menu_h(int row, int col, const char *const *items, const int *nums,
                   int n, int sel)
{
    int i;
    int x = col;
    for (i = 0; i < n; ++i) {
        Style st = (i == sel) ? ST_SELECTED : ST_TEXT;
        const char *label = (items != NULL && items[i] != NULL) ? items[i] : "";
        int len = ui_visible_len(label) + 8;
        if (nums != NULL) render_text(row, x, st, "[%d:%s]", nums[i], label);
        else render_text(row, x, st, "[%s]", label);
        x += (len > 18) ? len : 18;
    }
}

void widget_footer_reset(void)
{
    footerRow = caps_rows() - 1;
    if (footerRow >= GRID_MAX_ROWS) footerRow = GRID_MAX_ROWS - 1;
}

void widget_footer_line(const char *text)
{
    if (footerRow < 0) widget_footer_reset();
    if (footerRow < 0) return;
    render_text_clipped(footerRow, 1, caps_cols() - 2, ST_DIM, "%s", text);
    --footerRow;
}

void widget_footer(const char *extra)
{
    const Glyphs *g = caps_glyphs();
    char buf[160];
    snprintf(buf, sizeof(buf), "%s Naik/Bawah: pilih  Enter: ok  Esc: batal%s%s",
             g->arrowUD, (extra != NULL && extra[0] != '\0') ? "  " : "",
             (extra != NULL) ? extra : "");
    widget_footer_line(buf);
}

void widget_msg(int row, int col, int w, const char *msg)
{
    if (msg == NULL || msg[0] == '\0') return;
    render_text_clipped(row, col, w, ST_ACCENT, "%s", msg);
}
