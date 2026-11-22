#ifndef UI_H
#define UI_H

#include "config.h"
#include "style.h"

/**
 * @file ui.h
 * @brief Satu-satunya API untuk layar, menu, input, log, dan konfirmasi.
 *
 * Semua fungsi di sini bekerja pada dua mode:
 *  - interaktif: stdout adalah terminal; layar penuh, warna, panah, raw input;
 *  - pipa: stdout dialihkan (test/CI); teks polos satu baris per prompt.
 *
 * Aturan pemakaian untuk modul P2-P5:
 *  - satu frame = ui_screen_begin() -> gambar isi -> ui_screen_end();
 *  - jangan memakai printf untuk layar; hasil aksi lewat ui_log()/set_message();
 *  - jangan memanggil API legacy (yang bertanda @deprecated);
 *  - angka acak memakai plat_rand(), bukan rand().
 *
 * Panduan lengkap beserta contoh: docs/UI-API.md.
 */

/**
 * @brief Callback penggambar layar untuk ui_menu_pick/ui_pick_row.
 * @param ctx data pemanggil, diteruskan apa adanya;
 * @param sel indeks yang sedang disorot, atau -1 di mode pipa (tanpa sorotan).
 */
typedef void (*UiDrawFn)(void *ctx, int sel);

/**
 * @brief Callback gaya satu sel peta untuk ui_map_draw.
 * @param ctx data pemanggil, diteruskan apa adanya;
 * @param x kolom peta (bukan kolom layar);
 * @param y baris peta (bukan baris layar);
 * @param cell karakter sel, atau spasi di luar batas peta.
 * @return token gaya dari style.h untuk sel tersebut.
 */
typedef Style (*UiCellStyleFn)(void *ctx, int x, int y, char cell);

/* ------------------------------------------------------------------ */
/* Inisialisasi dan pesan sistem                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Siapkan lapisan UI: deteksi terminal, gaya warna, grid, dan mode input.
 * @note Dipanggil sekali oleh main.c sebelum layar pertama; modul tidak perlu.
 */
void ui_init(void);

/**
 * @brief Kembalikan terminal ke mode normal saat program selesai.
 * @note Sudah dipasang otomatis lewat atexit() oleh ui_init().
 */
void ui_shutdown(void);

/**
 * @brief Tanya apakah sesi berjalan interaktif.
 * @return 1 bila stdout adalah terminal (layar penuh aman), 0 bila mode pipa.
 */
int ui_interactive(void);

/**
 * @brief Simpan pesan sistem yang tampil pada frame berikutnya.
 * @param fmt format printf, mis. "Gagal membuka %s";
 * @param ... argumen format.
 * @note Pesan digambar oleh ui_screen_end()/ui_footer_message().
 */
void set_message(const char *fmt, ...);

/**
 * @brief Ambil pesan sistem terakhir tanpa mengubahnya.
 * @return pointer ke buffer pesan internal, string kosong bila belum ada.
 */
const char *last_message(void);

/* ------------------------------------------------------------------ */
/* Log aksi                                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Tambah satu baris log berwarna ST_TEXT.
 * @param fmt format printf;
 * @param ... argumen format.
 * @note Mode pipa langsung mencetak baris tersebut ke stdout.
 */
void ui_log(const char *fmt, ...);

/**
 * @brief Tambah satu baris log dengan gaya tertentu.
 * @param st token gaya dari style.h;
 * @param fmt format printf;
 * @param ... argumen format.
 */
void ui_log_style(Style st, const char *fmt, ...);

/**
 * @brief Kosongkan seluruh log, mis. saat masuk layar/pertarungan baru.
 */
void ui_log_clear(void);

/**
 * @brief Tambah satu baris log terstruktur (aktor, aksi, target, nilai).
 * @param actor nama pelaku, mis. "Farras";
 * @param action kata kerja, mis. "menyerang";
 * @param target nama sasaran atau alasan, boleh NULL;
 * @param value angka/hasil utama, mis. "13 damage", boleh NULL;
 * @param actorSt gaya nama pelaku;
 * @param targetSt gaya sasaran;
 * @param valueSt gaya nilai hasil.
 */
void ui_log_event(const char *actor, const char *action, const char *target,
                  const char *value, Style actorSt, Style targetSt, Style valueSt);

/**
 * @brief Jumlah baris log yang tersimpan (maksimum 24).
 * @return banyak baris, 0 bila log kosong.
 */
int ui_log_count(void);

/**
 * @brief Ambil teks satu baris log yang sudah digabung.
 * @param i indeks baris 0..ui_log_count()-1.
 * @return string internal; string kosong bila indeks di luar jangkauan.
 */
const char *ui_log_text(int i);

/**
 * @brief Gaya segmen pertama satu baris log.
 * @param i indeks baris log.
 * @return token gaya; ST_TEXT bila baris tidak ada.
 */
Style ui_log_style_at(int i);

/**
 * @brief Jumlah segmen bergaya pada satu baris log.
 * @param i indeks baris log.
 * @return banyak segmen, 0 bila baris tidak ada.
 */
int ui_log_part_count(int i);

/**
 * @brief Teks satu segmen pada satu baris log.
 * @param i indeks baris;
 * @param part indeks segmen 0..ui_log_part_count(i)-1.
 * @return string internal; string kosong bila indeks tidak valid.
 */
const char *ui_log_part_text(int i, int part);

/**
 * @brief Gaya satu segmen pada satu baris log.
 * @param i indeks baris;
 * @param part indeks segmen.
 * @return token gaya; ST_TEXT bila segmen tidak ada.
 */
Style ui_log_part_style(int i, int part);

/**
 * @brief Gambar satu baris log lengkap dengan gaya tiap segmennya.
 * @param row baris layar;
 * @param col kolom layar;
 * @param maxW lebar maksimum yang boleh dipakai (dipotong bila perlu);
 * @param i indeks baris log.
 */
void ui_log_draw(int row, int col, int maxW, int i);

/* ------------------------------------------------------------------ */
/* Kerangka frame                                                      */
/* ------------------------------------------------------------------ */

/**
 * @brief Mulai frame: kosongkan grid, tulis judul "GACHA QUEST · <screen>" dan garis.
 * @param screen nama layar, mis. "DUNGEON".
 * @note Pasangan mentah dari ui_screen_begin(); jangan lupa ui_flush() di akhir.
 */
void ui_frame(const char *screen);

/**
 * @brief Kirim isi grid ke terminal (satu-satunya titik output layar).
 * @note Mode pipa mencetak baris polos tanpa escape.
 */
void ui_flush(void);

/** @brief Baris info di bawah garis judul (lihat ui_screen_begin). */
#define UI_INFO_ROW 2
/** @brief Baris pertama untuk isi layar. */
#define UI_CONTENT_ROW 4

/**
 * @brief Mulai layar standar: frame + judul + baris info.
 * @param title judul layar, mis. "DUNGEON";
 * @param info satu baris keterangan, boleh NULL untuk mengosongkan.
 * @note Lanjutkan dengan isi layar mulai UI_CONTENT_ROW, akhiri ui_screen_end().
 */
void ui_screen_begin(const char *title, const char *info);

/**
 * @brief Tutup layar standar: footer + pesan sistem + flush.
 * @param hint teks tambahan footer, boleh NULL untuk hint bawaan menu.
 * @note Panggil sekali di akhir callback gambar; jangan flush di tengah isi.
 */
void ui_screen_end(const char *hint);

/**
 * @brief Panel daftar satu kolom dengan sorotan dan scroll otomatis.
 * @param row,col sudut kiri atas panel;
 * @param w lebar panel termasuk border;
 * @param h tinggi panel termasuk border (isi yang tampil = h-2);
 * @param title judul panel di border atas;
 * @param items array string yang ditampilkan;
 * @param n jumlah item;
 * @param sel indeks terpilih 0..n-1; jendela digeser agar sel terlihat.
 * @note `sel` adalah indeks, bukan nomor menu.
 */
void ui_list_panel(int row, int col, int w, int h, const char *title,
                   const char *const *items, int n, int sel);

/**
 * @brief Panel berisi beberapa entri log terakhir.
 * @param row,col sudut kiri atas panel;
 * @param w lebar panel termasuk border;
 * @param h tinggi panel termasuk border;
 * @param title judul panel;
 * @param lines jumlah entri terakhir yang ingin ditampilkan (dibatasi h-2).
 */
void ui_log_panel(int row, int col, int w, int h, const char *title, int lines);

/**
 * @brief Gambar jendela kamera dari grid peta karakter (dungeon).
 * @param row,col sudut kiri atas di layar;
 * @param w,h ukuran jendela kamera (dungeon: 25x15);
 * @param cells grid row-major, mis. (const char *)area;
 * @param stride panjang satu baris grid, mis. AREA_MAX;
 * @param mapW,mapH batas peta yang valid;
 * @param camX,camY sel peta yang tampil di kiri atas;
 * @param style callback gaya per sel, NULL berarti ST_TEXT;
 * @param ctx diteruskan apa adanya ke style.
 * @note Sel di luar batas peta digambar kosong. Helper ini hanya menggambar;
 *       pembangkitan peta dan arti karakter tetap milik modul pemanggil.
 */
void ui_map_draw(int row, int col, int w, int h,
                 const char *cells, int stride, int mapW, int mapH,
                 int camX, int camY, UiCellStyleFn style, void *ctx);

/* ------------------------------------------------------------------ */
/* Widget dasar                                                        */
/* ------------------------------------------------------------------ */

/**
 * @brief Gambar panel berborder dengan judul di border atas.
 * @param row,col sudut kiri atas;
 * @param w lebar termasuk border;
 * @param h tinggi termasuk border.
 */
void ui_panel(int row, int col, int w, int h, const char *title);

/**
 * @brief Bar HP berpadding satu sel; dipakai ui_status_panel_at.
 * @param row,col posisi;
 * @param w lebar total bar;
 * @param hp nilai HP sekarang;
 * @param maxHp nilai HP maksimum.
 * @note Modul lain sebaiknya memakai ui_hp_bar_n agar lebarnya tepat.
 */
void ui_hp_bar(int row, int col, int w, int hp, int maxHp);

/**
 * @brief Bar HP dengan lebar tepat w sel dan warna mengikuti ambang.
 * @param row,col posisi;
 * @param w lebar tepat;
 * @param hp nilai HP sekarang;
 * @param maxHp nilai HP maksimum.
 */
void ui_hp_bar_n(int row, int col, int w, int hp, int maxHp);

/**
 * @brief Bar umum untuk progres apa pun (EXP, langkah, dsb.).
 * @param row,col posisi;
 * @param w lebar tepat;
 * @param cur nilai sekarang;
 * @param max nilai maksimum (0 dianggap kosong);
 * @param fill token gaya untuk bagian terisi.
 */
void ui_bar(int row, int col, int w, int cur, int max, Style fill);

/**
 * @brief Gambar menu vertikal bernomor, satu item per baris.
 * @param row,col posisi item pertama;
 * @param items teks menu;
 * @param nums nomor yang ditampilkan;
 * @param n jumlah item;
 * @param sel indeks tersorot; -1 berarti tanpa sorotan.
 */
void ui_menu_v(int row, int col, const char *const *items, const int *nums, int n, int sel);

/**
 * @brief Gambar menu horizontal bernomor dalam satu baris.
 * @param row,col posisi awal;
 * @param items teks menu;
 * @param nums nomor yang ditampilkan;
 * @param n jumlah item;
 * @param sel indeks tersorot; -1 berarti tanpa sorotan.
 */
void ui_menu_h(int row, int col, const char *const *items, const int *nums, int n, int sel);

/**
 * @brief Gambar footer standar satu baris di bawah konten terakhir.
 * @param extra teks tambahan spesifik layar, boleh NULL.
 * @note Biasanya tidak dipanggil langsung; ui_screen_end sudah menanganinya.
 */
void ui_footer(const char *extra);

/**
 * @brief Gambar footer pengganti tanpa hint menu (mis. layar peta).
 * @param text teks footer; dipotong pada lebar terminal.
 */
void ui_footer_line(const char *text);

/**
 * @brief Gambar pesan sistem pada baris tetap.
 * @param row baris layar untuk pesan.
 * @note Hindari baris tetap; pakai ui_screen_end/ui_footer_message.
 */
void ui_message(int row);

/**
 * @brief Gambar pesan sistem satu baris kosong di bawah footer.
 * @note Tidak melakukan apa pun bila pesan kosong atau mode pipa.
 */
void ui_footer_message(void);

/**
 * @brief Panel status pemain: nama, level, bar HP, gold, dan potion.
 * @param p pemain yang ditampilkan (dari player.h);
 * @param row,col sudut kiri atas panel;
 * @param w lebar panel termasuk border.
 */
void ui_status_panel_at(const Player *p, int row, int col, int w);

/* ------------------------------------------------------------------ */
/* Loop menu dan pemilihan                                             */
/* ------------------------------------------------------------------ */

/**
 * @brief Loop menu siap pakai: panah/WASD, angka, Enter, Esc.
 * @param items teks menu;
 * @param nums nomor yang dikembalikan; NULL berarti minNum+i;
 * @param n jumlah item;
 * @param minNum,maxNum rentang nomor yang sah untuk input angka;
 * @param draw callback yang menggambar seluruh layar untuk indeks sel;
 * @param ctx diteruskan ke draw.
 * @return nomor pilihan, 0 bila Esc/kembali (saat ada nomor 0 atau nums NULL),
 *         -1 saat EOF.
 * @note Di mode pipa, draw dipanggil sekali dengan sel=-1 lalu pengguna
 *       mengetik nomor menu.
 */
int ui_menu_pick(const char *const *items, const int *nums, int n,
                 int minNum, int maxNum, UiDrawFn draw, void *ctx);

/**
 * @brief Konfirmasi Ya/Tidak inline di bagian bawah layar.
 * @param question pertanyaan, dibungkus otomatis sampai tiga baris.
 * @return 1 untuk Ya, 0 untuk Tidak/Esc/EOF (mode pipa selalu 0).
 */
int ui_confirm_box(const char *question);

/**
 * @brief Pilih satu baris dari daftar/tabel lewat loop panah.
 * @param count jumlah baris yang bisa dipilih;
 * @param draw callback penggambar layar; menerima indeks tersorot;
 * @param ctx diteruskan ke draw.
 * @return indeks 0..count-1, atau -1 bila Esc/EOF.
 * @note Mode pipa: pengguna mengetik nomor baris 1-based (0 = batal).
 */
int ui_pick_row(int count, UiDrawFn draw, void *ctx);

/* ------------------------------------------------------------------ */
/* Input baris                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Tetapkan judul header untuk prompt-prompt input berikutnya.
 * @param name nama layar, mis. "DUNGEON".
 */
void ui_input_screen(const char *name);

/**
 * @brief Gambar kotak Input dengan pertanyaan dan mulai buffer kosong.
 * @param fmt format pertanyaan;
 * @param ... argumen format.
 * @note Di mode pipa hanya mencetak pertanyaan tanpa baris baru.
 */
void ui_line_begin(const char *fmt, ...);

/**
 * @brief Selesai memakai kotak Input dan kembalikan kursor.
 * @note Panggil setelah ui_line_read(), juga pada jalur batal/keluar.
 */
void ui_line_end(void);

/**
 * @brief Gambar ulang kotak Input tanpa mengubah catatan validasi.
 */
void ui_line_prompt(void);

/**
 * @brief Gambar ulang kotak Input dengan catatan bahaya (validasi gagal).
 * @param fmt format catatan;
 * @param ... argumen format.
 */
void ui_line_error(const char *fmt, ...);

/**
 * @brief Gambar ulang kotak Input dengan catatan sukses.
 * @param fmt format catatan;
 * @param ... argumen format.
 */
void ui_line_note(const char *fmt, ...);

/**
 * @brief Baca satu baris di dalam kotak Input (mode interaktif).
 * @param buf buffer tujuan;
 * @param max ukuran buffer termasuk terminator;
 * @return 1 saat Enter, 0 saat EOF, -1 saat Esc.
 * @note Mode pipa memakai fgets dan mengembalikan 0 bila input habis.
 */
int ui_line_read(char *buf, int max);

/**
 * @brief Hitung lebar tampil string, menghitung glyph lebar sebagai dua kolom.
 * @param s string UTF-8.
 * @return jumlah kolom yang dipakai.
 */
int lebar_tampil(const char *s);

/**
 * @brief Bungkus teks menjadi beberapa baris sesuai lebar kolom.
 * @param text teks sumber;
 * @param width lebar maksimum satu baris;
 * @param rows buffer hasil, tiap baris maksimal 191 karakter;
 * @param maxRows jumlah baris yang muat di buffer.
 * @return jumlah baris hasil (0 bila tidak ada yang bisa ditulis).
 */
int wrap_teks(const char *text, int width, char rows[][192], int maxRows);

/**
 * @brief Gambar ulang kotak Input dengan pertanyaan dan isi buffer tertentu.
 * @param question pertanyaan;
 * @param buffer isi input yang ditampilkan.
 * @note Dipakai oleh helper input; modul biasanya tidak perlu memanggilnya.
 */
void render_input_box(const char *question, const char *buffer);

/* ------------------------------------------------------------------ */
/* API legacy (jangan dipakai kode baru)                               */
/* ------------------------------------------------------------------ */

/** @deprecated Token ANSI lama; pakai Style/style.h. */
#define ANSI_RESET "\033[0m"
/** @deprecated Token ANSI lama; pakai Style/style.h. */
#define ANSI_CYAN "\033[36m"
/** @deprecated Token ANSI lama; pakai Style/style.h. */
#define ANSI_RED "\033[31m"
/** @deprecated Token ANSI lama; pakai Style/style.h. */
#define ANSI_GREEN "\033[32m"
/** @deprecated Token ANSI lama; pakai Style/style.h. */
#define ANSI_YELLOW "\033[33m"
/** @deprecated Token ANSI lama; pakai Style/style.h. */
#define ANSI_MAGENTA "\033[35m"
/** @deprecated Token ANSI lama; pakai Style/style.h. */
#define ANSI_DIM "\033[2m"

/**
 * @brief Apakah terminal mendukung UTF-8.
 * @return 1 bila karakter Unicode aman dipakai, 0 untuk mode ASCII/mono.
 */
int ui_utf8_enabled(void);

/** @deprecated Layar lama; pakai ui_frame/ui_screen_begin. */
void clear_screen(void);
/** @deprecated Layar lama; pakai ui_panel. */
void draw_box_top(const char *title);
/** @deprecated Layar lama; pakai ui_panel. */
void draw_box_bottom(void);
/** @deprecated Layar lama; pakai ui_screen_end/ui_log. */
void draw_box_line(const char *fmt, ...);
/** @deprecated Layar lama; pakai ui_hp_bar_n. */
void format_hp_bar(char *buf, int bufSize, int hp, int maxHp);
/** @deprecated Layar lama; pakai ui_screen_begin + ui_status_panel_at. */
void draw_header(const char *title, const Player *p);
/** @deprecated Layar lama; pakai ui_hp_bar_n. */
void draw_hp_bar(int hp, int maxHp);
/** @deprecated Layar lama; pakai ui_menu_v. */
void draw_menu(int number, const char *label);
/**
 * @brief Kode warna ANSI untuk tingkat rarity item.
 * @param rarity 1..4.
 * @return string ANSI; pakai style_rarity() untuk gaya semantik.
 * @deprecated Pakai style_rarity().
 */
const char *rarity_color(int rarity);
/**
 * @brief Kode warna ANSI untuk HP berdasarkan ambang.
 * @param hp nilai HP sekarang;
 * @param maxHp nilai maksimum.
 * @return string ANSI; pakai style_hp() untuk gaya semantik.
 * @deprecated Pakai style_hp().
 */
const char *hp_color(int hp, int maxHp);

#endif
