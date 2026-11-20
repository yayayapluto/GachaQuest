#include <stdio.h>
#include <string.h>

#include "term.h"
#include "caps.h"
#include "sig.h"

static int rawOn;
static int altOn;
static int termReady;

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
static int savedCursorVisible = 1;
#else
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <sys/ioctl.h>
static struct termios savedTio;
static int hasSavedTio;
#endif

#ifndef _WIN32
int term_saved_tio(struct termios *out)
{
    if (out == NULL || !hasSavedTio) return 0;
    *out = savedTio;
    return 1;
}
#endif

void term_init(void)
{
    if (termReady) return;
    termReady = 1;
#ifndef _WIN32
    if (caps_interactive() && tcgetattr(0, &savedTio) == 0) hasSavedTio = 1;
#endif
    term_raw();
    if (caps_interactive()) term_alt(1);
}

static void applyRaw(int on)
{
#ifndef _WIN32
    struct termios tio;
    if (!hasSavedTio) return;
    if (tcgetattr(0, &tio) != 0) return;
    if (on) {
        tio.c_lflag &= ~(ICANON | ECHO | ISIG);
        tio.c_cc[VMIN] = 1;
        tio.c_cc[VTIME] = 0;
    } else {
        tio = savedTio;
    }
    (void)tcsetattr(0, TCSANOW, &tio);
#else
    (void)on;
#endif
}

void term_raw(void)
{
    applyRaw(1);
    rawOn = 1;
}

void term_cooked(void)
{
    applyRaw(0);
    rawOn = 0;
}

static int readByte(void)
{
#ifdef _WIN32
    return _getch();
#else
    unsigned char c;
    ssize_t n = read(0, &c, 1);
    return (n == 1) ? (int)c : (n == 0 ? EOF : -1);
#endif
}

static int waitInput(void)
{
#ifdef _WIN32
    return 1;
#else
    struct pollfd fds[2];
    int nfds = 1;
    int rc;
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    if (sig_wake_fd() >= 0) {
        fds[1].fd = sig_wake_fd();
        fds[1].events = POLLIN;
        nfds = 2;
    }
    rc = poll(fds, (nfds_t)nfds, -1);
    if (rc < 0) return 0;
    if (nfds == 2 && (fds[1].revents & POLLIN)) {
        char tmp[16];
        (void)read(fds[1].fd, tmp, sizeof(tmp));
        if (sig_requested()) return -1;
    }
    return 1;
#endif
}

TermKey term_getkey(char *ch)
{
    int c;

    if (ch != NULL) *ch = '\0';
    if (!caps_interactive()) {
        c = getchar();
        if (c == EOF) return KEY_EOF;
        if (ch != NULL) *ch = (char)c;
        if (c == '\n' || c == '\r') return KEY_ENTER;
        if (c == 27) return KEY_ESC;
        return KEY_CHAR;
    }
    if (!waitInput()) return KEY_NONE;
    c = readByte();
#ifdef _WIN32
    if (c == 0 || c == 224) {
        int ext = readByte();
        switch (ext) {
            case 72: return KEY_UP;
            case 80: return KEY_DOWN;
            case 75: return KEY_LEFT;
            case 77: return KEY_RIGHT;
            default: return KEY_ESC;
        }
    }
#endif
    if (c < 0) return sig_requested() ? KEY_ESC : KEY_NONE;
    if (c == EOF) return KEY_EOF;
    if (c == 27) {
        int a = readByte();
        if (a == '[') {
            int b = readByte();
            switch (b) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
                case 'r': return KEY_RESIZE;
                default: return KEY_ESC;
            }
        }
        return KEY_ESC;
    }
    if (c == '\n' || c == '\r') return KEY_ENTER;
    if (ch != NULL) *ch = (char)c;
    return KEY_CHAR;
}

void term_cursor(int on)
{
    if (!caps_interactive()) return;
#ifdef _WIN32
    {
        CONSOLE_CURSOR_INFO ci;
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleCursorInfo(h, &ci);
        ci.bVisible = on ? TRUE : FALSE;
        SetConsoleCursorInfo(h, &ci);
        savedCursorVisible = on;
    }
#else
    printf(on ? "\033[?25h" : "\033[?25l");
    fflush(stdout);
#endif
}

void term_alt(int on)
{
    if (!caps_interactive()) return;
    if (on == altOn) return;
    altOn = on;
    printf(on ? "\033[?1049h" : "\033[?1049l");
    fflush(stdout);
}

int term_size_query(void)
{
    int before = caps_cols() * 1000 + caps_rows();
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        caps_set_size(info.srWindow.Right - info.srWindow.Left + 1,
                      info.srWindow.Bottom - info.srWindow.Top + 1);
    }
#else
    {
        struct winsize ws;
        if (ioctl(1, TIOCGWINSZ, &ws) == 0) caps_set_size(ws.ws_col, ws.ws_row);
    }
#endif
    return (caps_cols() * 1000 + caps_rows()) != before;
}

void term_poll_size(void)
{
    (void)term_size_query();
}

void term_shutdown(void)
{
    if (!termReady) return;
    term_raw();
    term_cooked();
    term_alt(0);
    term_cursor(1);
#ifndef _WIN32
    if (hasSavedTio) {
        (void)tcsetattr(0, TCSANOW, &savedTio);
        hasSavedTio = 0;
    }
#else
    (void)savedCursorVisible;
#endif
    printf("\033[0m");
    fflush(stdout);
    rawOn = 0;
    termReady = 0;
}
