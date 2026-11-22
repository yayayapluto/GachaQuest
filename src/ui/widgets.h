#ifndef WIDGETS_H
#define WIDGETS_H

#include "style.h"

/**
 * @file widgets.h
 * @brief Implementasi widget tingkat rendah; modul game memakai ui.h.
 *
 * Tidak ada aturan baru di sini: semua fungsi adalah pasangan dari ui_* di
 * ui.h. Pemanggilan langsung hanya untuk layar lama atau pengujian.
 */

/**
 * @brief Panel berborder dengan judul memotong border atas.
 * @param row,col sudut kiri atas;
 * @param w lebar termasuk border;
 * @param h tinggi termasuk border;
 * @param title judul panel (dipotong bila terlalu panjang).
 */
void widget_panel(int row, int col, int w, int h, const char *title);

/**
 * @brief Bar HP berpadding satu sel.
 * @param row,col posisi;
 * @param w lebar total;
 * @param hp HP sekarang;
 * @param maxHp HP maksimum.
 */
void widget_hp_bar(int row, int col, int w, int hp, int maxHp);

/**
 * @brief Bar HP dengan lebar tepat w sel dan warna mengikuti ambang.
 * @param row,col posisi;
 * @param w lebar tepat;
 * @param hp HP sekarang;
 * @param maxHp HP maksimum.
 */
void widget_hp_bar_n(int row, int col, int w, int hp, int maxHp);

/**
 * @brief Bar progres umum.
 * @param row,col posisi;
 * @param w lebar tepat;
 * @param cur nilai sekarang;
 * @param max nilai maksimum (0 = kosong);
 * @param fill token gaya bagian terisi.
 */
void widget_bar(int row, int col, int w, int cur, int max, Style fill);

/**
 * @brief Menu vertikal bernomor.
 * @param row,col posisi item pertama;
 * @param items teks menu;
 * @param nums nomor yang ditampilkan;
 * @param n jumlah item;
 * @param sel indeks tersorot (-1 = tanpa sorotan).
 */
void widget_menu_v(int row, int col, const char *const *items, const int *nums,
                   int n, int sel);

/**
 * @brief Menu horizontal bernomor dalam satu baris.
 * @param row,col posisi awal;
 * @param items teks menu;
 * @param nums nomor yang ditampilkan;
 * @param n jumlah item;
 * @param sel indeks tersorot (-1 = tanpa sorotan).
 */
void widget_menu_h(int row, int col, const char *const *items, const int *nums,
                   int n, int sel);

/**
 * @brief Footer standar di bawah konten terakhir.
 * @param extra teks tambahan, boleh NULL.
 */
void widget_footer(const char *extra);

/**
 * @brief Footer pengganti tanpa hint menu.
 * @param text teks footer yang dipotong sesuai lebar.
 */
void widget_footer_line(const char *text);

/**
 * @brief Lupakan baris footer agar frame berikutnya memilih ulang.
 */
void widget_footer_reset(void);

/**
 * @brief Baris footer terakhir.
 * @return baris layar, atau -1 bila footer belum digambar pada frame ini.
 */
int widget_footer_row(void);

/**
 * @brief Gambar pesan dengan marker di awal baris.
 * @param row,col posisi awal;
 * @param w lebar maksimum;
 * @param msg teks pesan; tidak melakukan apa pun bila NULL/kosong.
 */
void widget_msg(int row, int col, int w, const char *msg);

#endif
