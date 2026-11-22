#ifndef RENDER_H
#define RENDER_H

#include "style.h"

/**
 * @file render.h
 * @brief Grid karakter dan flush ke terminal; dipakai UI, bukan modul game.
 *
 * Modul P2-P5 sebaiknya memakai helper di ui.h. Alat di sini hanya untuk
 * menggambar teks/sel di luar widget, mis. judul kecil atau peta khusus.
 */

/** @brief Lebar acuan tata letak tetap. */
#define UI_COLS 50
/** @brief Tinggi acuan tata letak tetap. */
#define UI_ROWS 20
/** @brief Lebar terminal minimum; di bawah ini layar menolak menggambar. */
#define MIN_COLS 40
/** @brief Tinggi terminal minimum; di bawah ini layar menolak menggambar. */
#define MIN_ROWS 18
/** @brief Batas kapasitas grid internal. */
#define GRID_MAX_ROWS 64
/** @brief Batas kapasitas grid internal. */
#define GRID_MAX_COLS 200

/** @brief Satu sel grid: codepoint dan token gaya. cp 0 berarti kosong. */
typedef struct {
    unsigned int cp;
    Style st;
} Cell;

/**
 * @brief Kosongkan grid dan siapkan buffer render.
 * @note Dipanggil oleh ui_init(); bukan untuk modul.
 */
void render_init(void);

/**
 * @brief Kosongkan seluruh grid (tanpa menyentuh terminal).
 */
void render_clear(void);

/**
 * @brief Tulis teks berformat ke grid pada posisi baris/kolom.
 * @param row baris grid (0-based);
 * @param col kolom grid (0-based);
 * @param s token gaya dari style.h;
 * @param fmt format printf;
 * @param ... argumen format.
 * @note Tanpa clipping; bagian di luar grid diabaikan.
 */
void render_text(int row, int col, Style s, const char *fmt, ...);

/**
 * @brief Tulis teks ke grid dengan batas lebar kolom.
 * @param row baris grid;
 * @param col kolom awal;
 * @param maxW maksimum kolom yang boleh dipakai;
 * @param s token gaya;
 * @param fmt format printf;
 * @param ... argumen format.
 */
void render_text_clipped(int row, int col, int maxW, Style s, const char *fmt, ...);

/**
 * @brief Tulis teks dengan dua gaya; karakter '~' mengganti gaya base/alt.
 * @param row baris grid;
 * @param col kolom awal;
 * @param maxW maksimum kolom;
 * @param base gaya sebelum penanda '~';
 * @param alt gaya sesudah penanda '~';
 * @param text teks UTF-8.
 * @return jumlah sel yang ditulis.
 */
int render_span(int row, int col, int maxW, Style base, Style alt, const char *text);

/**
 * @brief Tulis satu karakter (codepoint) ke satu sel grid.
 * @param row baris grid;
 * @param col kolom grid;
 * @param s token gaya;
 * @param cp codepoint yang ditulis.
 */
void render_char(int row, int col, Style s, unsigned int cp);

/**
 * @brief Kirim isi grid ke terminal.
 * @note Interaktif: escape + gaya. Mode pipa: baris teks polos tanpa escape.
 */
void render_flush(void);

/**
 * @brief Hitung lebar tampil string (UTF-8, glyph lebar = 2 kolom).
 * @param s string sumber.
 * @return jumlah kolom yang dipakai.
 */
int ui_visible_len(const char *s);

/**
 * @brief Baris terendah yang sudah berisi konten.
 * @return indeks baris (0-based), -1 bila grid kosong.
 */
int render_max_row(void);

/**
 * @brief Codepoint pada satu sel grid.
 * @param row baris grid;
 * @param col kolom grid.
 * @return codepoint sel, 0 bila kosong atau di luar batas.
 */
unsigned int render_cell_cp(int row, int col);

#endif
