#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ui.h"
#include "player.h"
#include "term.h"
#include "caps.h"
#include "render.h"
#include "widgets.h"

#define MAX_LOG 32
#define MAX_PARTS 3

typedef struct {
    char text[192];
    char partText[MAX_PARTS][192];
    Style partStyle[MAX_PARTS];
    int parts;
    Style style;
} LogEntry;

static char messageText[256];
static char inputTitle[80];
static char inputQuestion[192];
static char inputNote[192];
static Style inputNoteStyle = ST_TEXT;
static LogEntry logs[MAX_LOG];
static int logCount;

void ui_init(void)
{
    caps_detect();
    style_init();
    render_init();
    term_init();
    widget_footer_reset();
    messageText[0] = '\0';
    inputTitle[0] = '\0';
    inputQuestion[0] = '\0';
    inputNote[0] = '\0';
}

void ui_shutdown(void)
{
    term_shutdown();
}

int ui_interactive(void)
{
    return caps_interactive();
}

void set_message(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(messageText, sizeof(messageText), fmt, ap);
    va_end(ap);
}

const char *last_message(void)
{
    return messageText;
}

void ui_log_clear(void)
{
    logCount = 0;
}

static void pushLog(Style st, const char *fmt, va_list ap)
{
    LogEntry *e;
    int i;
    if (logCount >= MAX_LOG) {
        for (i = 0; i < MAX_LOG - 1; ++i) logs[i] = logs[i + 1];
        --logCount;
    }
    e = &logs[logCount++];
    vsnprintf(e->text, sizeof(e->text), fmt, ap);
    e->style = st;
    e->parts = 1;
    memcpy(e->partText[0], e->text, sizeof(e->text));
    e->partStyle[0] = st;
    if (!caps_interactive()) printf("%s\n", e->text);
}

void ui_log(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    pushLog(ST_TEXT, fmt, ap);
    va_end(ap);
}

void ui_log_style(Style st, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    pushLog(st, fmt, ap);
    va_end(ap);
}

void ui_log_event(const char *actor, const char *action, const char *target,
                  const char *value, Style actorSt, Style targetSt, Style valueSt)
{
    LogEntry *e;
    if (logCount >= MAX_LOG) ui_log_clear();
    e = &logs[logCount++];
    snprintf(e->text, sizeof(e->text), "%s %s%s%s%s",
             actor != NULL ? actor : "", action != NULL ? action : "",
             target != NULL ? " " : "", target != NULL ? target : "",
             value != NULL ? value : "");
    e->style = ST_TEXT;
    e->parts = 0;
    if (actor != NULL && e->parts < MAX_PARTS) {
        snprintf(e->partText[e->parts], sizeof(e->partText[0]), "%s", actor);
        e->partStyle[e->parts++] = actorSt;
    }
    if (target != NULL && e->parts < MAX_PARTS) {
        snprintf(e->partText[e->parts], sizeof(e->partText[0]), "%s", target);
        e->partStyle[e->parts++] = targetSt;
    }
    if (value != NULL && e->parts < MAX_PARTS) {
        snprintf(e->partText[e->parts], sizeof(e->partText[0]), "%s", value);
        e->partStyle[e->parts++] = valueSt;
    }
    if (!caps_interactive()) printf("%s\n", e->text);
}

int ui_log_count(void)
{
    return logCount;
}

const char *ui_log_text(int i)
{
    if (i < 0 || i >= logCount) return "";
    return logs[i].text;
}

Style ui_log_style_at(int i)
{
    if (i < 0 || i >= logCount) return ST_TEXT;
    return logs[i].style;
}

const char *ui_log_part_text(int i, int part)
{
    if (i < 0 || i >= logCount || part < 0 || part >= logs[i].parts) return "";
    return logs[i].partText[part];
}

Style ui_log_part_style(int i, int part)
{
    if (i < 0 || i >= logCount || part < 0 || part >= logs[i].parts) return ST_TEXT;
    return logs[i].partStyle[part];
}

void ui_log_draw(int row, int col, int maxW, int i)
{
    render_text_clipped(row, col, maxW, ui_log_style_at(i), "%s", ui_log_text(i));
}

void ui_frame(const char *screen)
{
    const Glyphs *g = caps_glyphs();
    int w = caps_cols();
    widget_footer_reset();
    render_clear();
    render_text(0, 1, ST_BORDER, "%s", g->tl);
    {
        int c;
        for (c = 2; c < w - 1; ++c) render_char(0, c, ST_BORDER, '-');
    }
    render_text(0, w - 1, ST_BORDER, "%s", g->tr);
    if (screen != NULL) render_text_clipped(0, 3, w - 6, ST_TITLE, " %s ", screen);
}

void ui_flush(void)
{
    render_flush();
}

void ui_panel(int row, int col, int w, int h, const char *title)
{
    widget_panel(row, col, w, h, title);
}

void ui_hp_bar(int row, int col, int w, int hp, int maxHp)
{
    widget_hp_bar(row, col, w, hp, maxHp);
}

void ui_hp_bar_n(int row, int col, int w, int hp, int maxHp)
{
    widget_hp_bar_n(row, col, w, hp, maxHp);
}

void ui_bar(int row, int col, int w, int cur, int max, Style fill)
{
    widget_bar(row, col, w, cur, max, fill);
}

void ui_menu_v(int row, int col, const char *const *items, const int *nums, int n, int sel)
{
    widget_menu_v(row, col, items, nums, n, sel);
}

void ui_menu_h(int row, int col, const char *const *items, const int *nums, int n, int sel)
{
    widget_menu_h(row, col, items, nums, n, sel);
}

void ui_footer(const char *extra)
{
    widget_footer(extra);
}

void ui_footer_line(const char *text)
{
    widget_footer_line(text);
}

void ui_message(int row)
{
    widget_msg(row, 2, caps_cols() - 4, messageText);
}

void ui_status_panel_at(const Player *p, int row, int col, int w)
{
    if (p == NULL) return;
    widget_panel(row, col, w, 9, "STATUS");
    render_text(row + 1, col + 2, ST_TEXT, "Nama  : %s", p->name);
    render_text(row + 2, col + 2, ST_TEXT, "Level : %d   EXP %d", p->level, p->exp);
    render_text(row + 3, col + 2, ST_GOLD, "Gold  : %d", p->gold);
    render_text(row + 4, col + 2, ST_TEXT, "ATK %d  DEF %d", totalAtk(p), totalDef(p));
    render_text(row + 5, col + 2, ST_SUCCESS, "HP    : %d/%d", p->hp, p->maxHp);
    ui_hp_bar_n(row + 6, col + 2, w - 4, p->hp, p->maxHp);
    render_text(row + 7, col + 2, ST_DIM, "Potion %d  Poin %d", p->potion, p->score);
}

int ui_menu_pick(const char *const *items, const int *nums, int n,
                 int minNum, int maxNum, UiDrawFn draw, void *ctx)
{
    int sel = 0;
    int result = -1;

    (void)minNum;
    (void)maxNum;
    if (n <= 0) return -1;
    for (;;) {
        TermKey key;
        char ch = '\0';
        if (draw != NULL) draw(ctx, sel);
        else {
            ui_frame("PILIH");
            ui_menu_v(3, 4, items, nums, n, sel);
            ui_footer("");
            ui_flush();
        }
        key = term_getkey(&ch);
        if (key == KEY_UP) { --sel; if (sel < 0) sel = n - 1; }
        else if (key == KEY_DOWN) { ++sel; if (sel >= n) sel = 0; }
        else if (key == KEY_ENTER) {
            if (nums != NULL) result = nums[sel];
            else result = sel + 1;
            break;
        } else if (key == KEY_ESC || key == KEY_EOF) {
            result = -1;
            break;
        } else if (key == KEY_RESIZE) {
            term_poll_size();
        } else if (key == KEY_CHAR && (ch == 'q' || ch == 'Q')) {
            result = -1;
            break;
        }
    }
    return result;
}

int ui_confirm_box(const char *question)
{
    for (;;) {
        TermKey key;
        char ch = '\0';
        ui_frame("KONFIRMASI");
        widget_panel(5, 5, 40, 7, "Tanya");
        render_text_clipped(7, 7, 36, ST_TEXT, "%s", question != NULL ? question : "");
        render_text(9, 7, ST_DIM, "Y = ya    N atau Esc = tidak");
        ui_flush();
        key = term_getkey(&ch);
        if (key == KEY_CHAR && (ch == 'y' || ch == 'Y')) return 1;
        if (key == KEY_CHAR && (ch == 'n' || ch == 'N')) return 0;
        if (key == KEY_ENTER) return 1;
        if (key == KEY_ESC || key == KEY_EOF) return 0;
    }
}

void ui_input_screen(const char *name)
{
    snprintf(inputTitle, sizeof(inputTitle), "%s", name != NULL ? name : "INPUT");
    inputNote[0] = '\0';
}

void render_input_box(const char *question, const char *buffer)
{
    int w = caps_cols();
    ui_frame(inputTitle);
    widget_panel(4, 3, w - 6, 10, inputTitle);
    render_text_clipped(6, 5, w - 10, ST_TEXT, "%s", question != NULL ? question : "");
    render_text(8, 5, ST_TEXT, "> %s", buffer != NULL ? buffer : "");
    if (inputNote[0] != '\0') {
        render_text_clipped(11, 5, w - 10, inputNoteStyle, "%s", inputNote);
    }
    ui_footer("Enter: kirim | Esc: batal");
    ui_flush();
}

void ui_line_begin(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(inputQuestion, sizeof(inputQuestion), fmt, ap);
    va_end(ap);
    inputNote[0] = '\0';
    render_input_box(inputQuestion, "");
}

void ui_line_end(void)
{
    render_input_box(inputQuestion, "");
}

void ui_line_prompt(void)
{
    render_input_box(inputQuestion, "");
}

void ui_line_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(inputNote, sizeof(inputNote), fmt, ap);
    va_end(ap);
    inputNoteStyle = ST_DANGER;
    render_input_box(inputQuestion, "");
}

void ui_line_note(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(inputNote, sizeof(inputNote), fmt, ap);
    va_end(ap);
    inputNoteStyle = ST_SUCCESS;
    render_input_box(inputQuestion, "");
}

int ui_line_read(char *buf, int max)
{
    char line[256];
    int len;

    if (buf == NULL || max <= 0) return 0;
    term_cooked();
    if (fgets(line, (int)sizeof(line), stdin) == NULL) {
        term_raw();
        buf[0] = '\0';
        return 0;
    }
    term_raw();
    len = (int)strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) line[--len] = '\0';
    if (len > 0 && line[0] == 27) { buf[0] = '\0'; return -1; }
    if (len >= max) len = max - 1;
    memcpy(buf, line, (size_t)len);
    buf[len] = '\0';
    return 1;
}

int lebar_tampil(const char *s)
{
    return ui_visible_len(s);
}

int wrap_teks(const char *text, int width, char rows[][192], int maxRows)
{
    int used = 0;
    const char *p = text;

    if (text == NULL || width <= 0 || maxRows <= 0) return 0;
    while (*p != '\0' && used < maxRows) {
        int n = 0;
        while (*p != '\0' && n < width && *p != '\n') {
            rows[used][n++] = *p++;
        }
        rows[used][n] = '\0';
        ++used;
        if (*p == '\n') ++p;
    }
    return used;
}

int ui_pick_row(int count, UiDrawFn draw, void *ctx)
{
    int sel = 0;
    if (count <= 0) return -1;
    for (;;) {
        TermKey key;
        char ch = '\0';
        if (draw != NULL) draw(ctx, sel);
        key = term_getkey(&ch);
        if (key == KEY_UP) { --sel; if (sel < 0) sel = count - 1; }
        else if (key == KEY_DOWN) { ++sel; if (sel >= count) sel = 0; }
        else if (key == KEY_ENTER) return sel;
        else if (key == KEY_ESC || key == KEY_EOF || key == KEY_CHAR) {
            if (key != KEY_CHAR || ch == 'q' || ch == 'Q') return -1;
        }
    }
}

int ui_utf8_enabled(void)
{
    return caps_unicode();
}

void clear_screen(void)
{
    if (caps_interactive()) printf("\033[2J\033[H");
    else putchar('\n');
    fflush(stdout);
}

void draw_box_top(const char *title)
{
    int w = caps_cols();
    int c;
    render_char(0, 1, ST_BORDER, '+');
    for (c = 2; c < w - 1; ++c) render_char(0, c, ST_BORDER, '-');
    render_char(0, w - 1, ST_BORDER, '+');
    if (title != NULL) render_text_clipped(0, 3, w - 6, ST_TITLE, " %s ", title);
    render_flush();
}

void draw_box_bottom(void)
{
    int w = caps_cols();
    int c;
    for (c = 1; c < w - 1; ++c) render_char(2, c, ST_BORDER, '-');
    render_char(2, 1, ST_BORDER, '+');
    render_char(2, w - 1, ST_BORDER, '+');
    render_flush();
}

void draw_box_line(const char *fmt, ...)
{
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    render_text_clipped(1, 3, caps_cols() - 6, ST_TEXT, "%s", buf);
    render_flush();
}

void format_hp_bar(char *buf, int bufSize, int hp, int maxHp)
{
    int w = 20;
    int filled = 0;
    int i, n = 0;
    if (buf == NULL || bufSize <= 0) return;
    if (maxHp > 0) filled = (hp * w) / maxHp;
    if (filled < 0) filled = 0;
    if (filled > w) filled = w;
    n += snprintf(buf + n, (size_t)(bufSize - n), "[");
    for (i = 0; i < w && n < bufSize - 1; ++i) buf[n++] = (i < filled) ? '=' : '-';
    if (n < bufSize - 1) buf[n++] = ']';
    buf[n] = '\0';
}

void draw_header(const char *title, const Player *p)
{
    ui_frame(title);
    if (p != NULL) render_text(1, 2, ST_DIM, "%s  Level %d  Gold %d", p->name, p->level, p->gold);
    ui_flush();
}

void draw_hp_bar(int hp, int maxHp)
{
    char buf[64];
    format_hp_bar(buf, sizeof(buf), hp, maxHp);
    render_text(2, 2, style_hp(hp, maxHp), "%s %d/%d", buf, hp, maxHp);
    render_flush();
}

void draw_menu(int number, const char *label)
{
    static int row = 4;
    render_text(row++, 3, ST_TEXT, "%d. %s", number, label != NULL ? label : "");
}

const char *rarity_color(int rarity)
{
    switch (rarity) {
        case RARITY_RARE: return "\033[36m";
        case RARITY_EPIC: return "\033[35m";
        case RARITY_LEGENDARY: return "\033[33m";
        default: return "\033[37m";
    }
}

const char *hp_color(int hp, int maxHp)
{
    Style s = style_hp(hp, maxHp);
    return style_fg(s);
}
