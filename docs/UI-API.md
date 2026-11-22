# Cara Memakai Fungsi UI

Dokumen ini untuk pemilik modul P2-P5. Tujuannya satu: layar, input, dan
konfirmasi cukup memanggil fungsi di `src/ui/ui.h`, tanpa menghitung baris
terminal atau menulis escape ANSI sendiri.

Setiap fungsi publik di `src/ui/ui.h`, `render.h`, `style.h`, dan `widgets.h`
punya komentar Doxygen lengkap dengan parameter dan nilai kembalian. Di VS Code,
arahkan kursor ke nama fungsi (atau ketik nama lalu `Ctrl+Space`) untuk melihat
deskripsinya tanpa membuka dokumen ini.

Semua fungsi UI sudah menangani dua mode:

| Mode | Kapan | Yang terjadi |
|---|---|---|
| Interaktif | stdout adalah terminal | layar penuh, warna, panah, raw input |
| Pipa | stdout dialihkan (test skenario/CI) | teks polos satu baris per prompt, tanpa escape |

Karena itu modul tidak perlu bercabang sendiri untuk mode pipa, kecuali saat
menggambar layar penuh (lihat bagian Aturan).

## 0. Sebelum mulai (untuk yang baru pertama kali)

Kamu tidak perlu paham terminal, ANSI, atau raw mode. Cukup pahami lima istilah
ini:

| Istilah | Artinya di panduan ini |
|---|---|
| grid | buffer karakter layar. Kamu menggambar ke buffer, bukan langsung ke terminal |
| flush | mengirim isi grid ke terminal. Biasanya sudah dilakukan `ui_screen_end` |
| callback | fungsi yang **kamu** tulis, lalu dipanggil UI; mis. fungsi menggambar layar |
| sel | indeks pilihan `0..n-1`. Ini bukan nomor menu |
| mode pipa | output dialihkan (mis. `sh tests/run_tests.sh`); layar jadi teks polos |

Aturan pembagian kerja: **P1 memiliki `main.c` dan `src/ui/`. Kamu menulis
modulmu di `src/<modul>/` dan memanggil fungsi UI dari sana.** P1 yang
menyambungkan modulmu ke menu game.

Cara mencoba hasilmu:

```bash
make            # kompilasi game + modul
./bin/game      # Windows: bin\game.exe
make test       # memastikan skenario teks tidak rusak
```

Semua contoh di bawah bisa langsung ditaruh di file modulmu. Kalau baru mulai,
salin contoh paling kecil ini dulu, panggil fungsinya dari modulmu, lalu
kembangkan:

```c
#include "ui.h"

static const char *const contohItems[] = { "Mulai", "Keluar" };
static const int contohNums[] = { 1, 0 };

static void gambarContoh(void *ctx, int sel)
{
    (void)ctx;
    ui_screen_begin("CONTOH", "layar pertama saya");
    ui_menu_v(UI_CONTENT_ROW, 6, contohItems, contohNums, 2, sel);
    ui_screen_end("Panah: pilih   Enter: konfirmasi");
}

int contohJalan(void)
{
    int pilih = ui_menu_pick(contohItems, contohNums, 2, 0, 1, gambarContoh, NULL);
    return pilih;   /* 1 = Mulai, 0 = Keluar/kembali, -1 = EOF */
}
```

Yang terjadi: `ui_screen_begin` membersihkan layar dan menulis judul,
`ui_menu_v` menggambar dua pilihan, `ui_screen_end` menulis footer dan
mengirim semuanya ke terminal. `ui_menu_pick` yang menangani tombol panah dan
Enter.

Kesalahan pemula yang paling sering:

| Gejala | Penyebab |
|---|---|
| layar kosong | lupa `ui_screen_end`/`ui_flush`, jadi grid tidak pernah dikirim |
| tulisan numpuk | memanggil `printf` di samping helper UI |
| pilihan tidak cocok | menganggap kembalian `ui_menu_pick` sebagai indeks; padahal itu nomor dari `nums` |
| crash saat mode test | callback menggambar memakai `sel` sebagai indeks saat `sel == -1` |
| fungsi tidak ditemukan | lupa `#include "ui.h"` atau file belum masuk `Makefile` (minta P1) |

## 1. Kerangka layar

Urutan tetap satu frame:

```c
#include "ui.h"

typedef struct {
    const char *const *items;
    int n;
} Layar;

static void gambarLayar(void *ctx, int sel)
{
    Layar *s = ctx;
    ui_screen_begin("NAMA LAYAR", "informasi satu baris");
    /* isi layar mulai UI_CONTENT_ROW */
    ui_list_panel(UI_CONTENT_ROW, 4, 40, 10, "Menu", s->items, s->n, sel);
    ui_screen_end("Panah: pilih   Enter: konfirmasi   Esc: batal");
}
```

| Konstanta | Nilai | Arti |
|---|---|---|
| `UI_INFO_ROW` | 2 | baris info tepat di bawah garis judul |
| `UI_CONTENT_ROW` | 4 | baris pertama untuk isi layar |
| `UI_COLS` | 50 | lebar acuan tata letak |
| `UI_ROWS` | 20 | tinggi acuan tata letak |
| `MIN_COLS` / `MIN_ROWS` | 40 / 18 | di bawah ini layar menolak menggambar |

Fungsi kerangka:

| Fungsi | Efek |
|---|---|
| `ui_screen_begin(judul, info)` | hapus grid, gambar `GACHA QUEST · judul`, garis, dan info di baris 2 |
| `ui_screen_end(hint)` | footer berisi `hint` (boleh `NULL`), pesan sistem satu baris di bawah footer, lalu `ui_flush()` |
| `ui_frame(judul)` + `ui_flush()` | versi mentah kalau tidak memakai dua helper di atas |

Footer dan pesan digambar otomatis; jangan menentukan barisnya sendiri.

## 2. Menu dan pemilihan

### `ui_menu_pick`

```c
int pilih = ui_menu_pick(items, nums, n, minNum, maxNum, gambarLayar, ctx);
```

- `items[i]` teks menu, `nums[i]` nomor yang dikembalikan.
- `gambarLayar(ctx, sel)` menggambar seluruh layar untuk indeks terpilih `sel`.
  Di mode pipa callback dipanggil sekali dengan `sel == -1`, jadi jangan
  mengindeks array dengan `sel` tanpa memeriksa `sel >= 0`.
- Tombol: panah/W/S, angka langsung, Enter memilih, Esc kembali (`0`) bila ada
  nomor 0, selain itu diabaikan.
- Kembalian: nomor dari `nums`, `0` untuk kembali, `-1` saat EOF.

### `ui_menu_v` / `ui_menu_h`

Menggambar menu tanpa loop. Hanya dipakai di dalam callback gambar:

```c
ui_menu_v(row, col, items, nums, n, sel);
ui_menu_h(row, col, items, nums, n, sel);
```

### `ui_list_panel`

Untuk daftar panjang (inventory, shop, leaderboard). Panel punya border,
sorotan, dan scroll otomatis mengikuti `sel`.

```c
ui_list_panel(row, col, w, h, "Inventory", namaItem, jumlah, sel);
```

- `h` termasuk dua baris border, jadi isi yang tampil = `h - 2`.
- `sel` adalah indeks 0..n-1; helper menggeser jendela supaya `sel` terlihat.
- Baris terpilih memakai marker dan gaya `ST_SELECTED`.

### `ui_pick_row`

Loop pemilihan baris untuk tabel. Kembalian indeks 0..count-1 atau `-1`.

```c
int idx = ui_pick_row(jumlah, gambarTabel, ctx);
```

Di mode pipa, pengguna mengetik nomor baris 1-based (`0` = batal).

## 3. Peta dungeon

`ui_map_draw` menggambar jendela kamera dari grid karakter. Modul yang punya
peta cukup menyediakan grid, posisi kamera, dan callback gaya per sel.

```c
static Style gayaPeta(void *ctx, int x, int y, char c)
{
    LayarDungeon *d = ctx;
    (void)x; (void)y;
    switch (c) {
    case '#': return ST_BIOME(d->biome);
    case '@': return ST_ACCENT;
    case 'E': return ST_DANGER;
    case '$': return ST_GOLD;
    default:  return ST_TEXT;
    }
}

ui_map_draw(UI_CONTENT_ROW, 4, 25, 15,
            (const char *)area, AREA_MAX, AREA_MAX, AREA_MAX,
            camX, camY, gayaPeta, d);
```

| Parameter | Arti |
|---|---|
| `row, col` | sudut kiri atas di layar |
| `w, h` | ukuran jendela kamera (dungeon: 25x15) |
| `cells` | grid row-major, mis. `(const char *)area` |
| `stride` | panjang satu baris grid, mis. `AREA_MAX` |
| `mapW, mapH` | batas peta yang valid |
| `camX, camY` | sel peta yang tampil di kiri atas |
| `style` | callback gaya per sel; `NULL` = `ST_TEXT` |
| `ctx` | diteruskan apa adanya ke callback |

Aturan:

- Helper hanya menggambar sel; border panel, judul, kamera, dan arti karakter
  tetap milik modul dungeon.
- Sel di luar `mapW`/`mapH` otomatis kosong, jadi kamera di pinggir peta aman.
- `style` dipanggil untuk setiap sel, termasuk yang kosong; boleh
  mengembalikan `ST_FOG_LEVEL`/`ST_BIOME` sesuai jarak dan biome.
- Untuk peta berpanel, panggil `ui_panel` lebih dulu, lalu gambar peta di
  dalamnya dengan offset satu sel.

## 4. Log dan panel pendukung

| Fungsi | Kegunaan |
|---|---|
| `ui_log(fmt, ...)` | tambah satu baris log; mode pipa langsung mencetak |
| `ui_log_event(actor, aksi, target, nilai, stActor, stTarget, stNilai)` | log kaya warna, mis. `Farras menyerang Slime 13 damage` |
| `ui_log_clear()` | kosongkan log saat masuk layar baru |
| `ui_log_panel(row, col, w, h, judul, lines)` | gambar `lines` entri terakhir di dalam panel |
| `ui_panel(row, col, w, h, judul)` | panel kosong berborder |
| `ui_status_panel_at(&pemain, row, col, w)` | panel Pemain/Level/HP/Gold/Potion |
| `ui_hp_bar_n(row, col, w, hp, maxHp)` | bar HP lebar tepat `w`, warna ikut ambang (dipakai panel Status dan Battle) |
| `ui_hp_bar(row, col, w, hp, maxHp)` | bar HP berpadding; dipakai `ui_status_panel_at`, modul lain pakai `ui_hp_bar_n` |
| `ui_log_count()` / `ui_log_text(i)` / `ui_log_draw(row, col, w, i)` | akses log per baris bila layar butuh tata letak sendiri |
| `ui_bar(row, col, w, cur, max, gaya)` | bar umum (EXP, progress) |
| `set_message(fmt, ...)` | siapkan pesan sistem untuk frame berikutnya |
| `ui_footer_message()` | gambar pesan itu satu baris di bawah footer |
| `ui_footer(hint)` | footer mentah; `ui_screen_end` sudah memanggil ini |
| `ui_footer_line(teks)` | footer pengganti tanpa hint menu (mis. layar peta) |
| `ui_message(row)` | pesan di baris tetap; hindari, pakai `ui_footer_message` |

Contoh layar dengan log:

```c
ui_screen_begin("DUNGEON", "Pusat kegiatan");
ui_status_panel_at(&pemain, UI_CONTENT_ROW, 4, 30);
ui_log_panel(UI_CONTENT_ROW, 40, 56, 10, "Log", 8);
ui_screen_end("Panah: gerak   Enter: aksi   Esc: keluar");
```

## 5. Input baris

Semua input teks memakai kotak Input sendiri. Jangan memakai `printf` + `fgets`
langsung untuk layar, dan jangan memanggil `term_cooked()` sendiri.

```c
ui_input_screen("DUNGEON");
ui_line_begin("Jumlah langkah: ");
int status = ui_line_read(buf, sizeof buf);
ui_line_end();
if (status == 0) return;       /* EOF: keluar dari layar */
if (status < 0) return;        /* Esc: batal */
```

| Fungsi | Efek |
|---|---|
| `ui_input_screen(nama)` | judul header untuk prompt-prompt berikutnya |
| `ui_line_begin(fmt, ...)` | gambar kotak dengan pertanyaan, mulai buffer kosong |
| `ui_line_read(buf, maks)` | edit satu baris; `1` Enter, `0` EOF, `-1` Esc |
| `ui_line_error(fmt, ...)` | gambar ulang kotak + catatan bahaya (validasi gagal) |
| `ui_line_note(fmt, ...)` | gambar ulang kotak + catatan sukses |
| `ui_line_prompt()` | gambar ulang kotak tanpa mengubah catatan |
| `ui_line_end()` | kembalikan kursor |

Pola validasi:

```c
for (;;) {
    char *akhir;
    long nilai;

    ui_line_begin("Jumlah langkah (1-20): ");
    int r = ui_line_read(buf, sizeof buf);
    if (r <= 0) { ui_line_end(); return; }

    nilai = strtol(buf, &akhir, 10);
    if (akhir != buf && *akhir == '\0' && nilai >= 1 && nilai <= 20) {
        ui_line_end();
        break;
    }
    ui_line_error("Masukkan angka 1 sampai 20.");
}
```

Konversi dan aturan domain tetap di modul masing-masing. `util.h` hanya
menyediakan `readInt`/`readIntTry` untuk alur teks lama; jangan dipakai di
dalam layar.

## 6. Konfirmasi

```c
if (ui_confirm_box("Simpan permainan sekarang?"))
    simpan();
```

- `1` = Ya, `0` = Tidak/Esc/EOF.
- Di mode pipa selalu `0`; jangan menggantungkan alur wajib pada konfirmasi.

## 7. Gaya dan warna

Gunakan token di `style.h`, jangan ANSI mentah:

`ST_TEXT`, `ST_TITLE`, `ST_ACCENT`, `ST_BORDER`, `ST_DIM`, `ST_GOLD`,
`ST_DANGER`, `ST_SUCCESS`, `ST_SELECTED`, serta `style_rarity(rarity)` dan
`style_hp(hp, maks)`.

Untuk menggambar teks bebas:

```c
render_text(row, col, ST_TEXT, "%s", teks);
render_text_clipped(row, col, lebar, ST_DIM, "%s", teks);
render_span(row, col, lebar, fg, bg, teks);
```

`render_*` adalah lapisan bawah; modul P2-P5 sebaiknya memakai helper widget di
atas. Kalau butuh teks satu baris di luar panel, `render_text` boleh.

## 8. Aturan yang wajib diikuti

1. Satu frame = `ui_screen_begin` -> isi -> `ui_screen_end` (atau
   `ui_frame` -> isi -> `ui_flush`). Jangan flush di tengah isi.
2. Jangan memanggil `printf` untuk layar. Mode pipa hanya boleh mencetak lewat
   `ui_log` dan helper input/menu.
3. Jangan menentukan sendiri baris judul, garis, footer, atau pesan; itu milik
   `ui_screen_*`.
4. Jangan memanggil API legacy (`draw_header`, `draw_box_*`, `clear_screen`,
   `format_hp_bar`, `ANSI_*`). Itu sisa layar lama dan tidak dipelihara.
5. Jangan mengubah `src/ui/*.h`; usulkan ke P1 kalau helper kurang.
6. Angka acak selalu `plat_rand()` dari `platform.h`, bukan `rand()`.
7. Layar penuh hanya digambar saat interaktif:

```c
if (ui_interactive()) {
    ui_menu_pick(...);
} else {
    /* alur teks: prompt input/validasi saja */
}
```

Helper input/menu sendiri sudah memilih jalur pipa; kondisi di atas hanya
diperlukan kalau modul memanggil `ui_screen_*` langsung di luar `ui_menu_pick`.

## 9. Contoh lengkap: layar dengan menu + log + input

Potongan ini mengandaikan modulmu sudah punya `Player` (dari `player.h`) dan
struktur state sendiri. Ganti `LayarDungeon` dengan struktur modulmu.

```c
#include "ui.h"
#include "player.h"

typedef struct {
    Player pemain;
    const char *aksi[4];
    int nomor[4];
    int nAksi;
} LayarDungeon;

static void gambarDungeon(void *ctx, int sel)
{
    LayarDungeon *d = ctx;
    ui_screen_begin("DUNGEON", "Pusat kegiatan");
    ui_status_panel_at(&d->pemain, UI_CONTENT_ROW, 4, 30);
    ui_list_panel(UI_CONTENT_ROW, 40, 30, 8, "Aksi", d->aksi, d->nAksi, sel);
    ui_log_panel(UI_CONTENT_ROW + 9, 40, 30, 7, "Log", 5);
    ui_screen_end("Panah: pilih   Enter: jalankan   Esc: keluar");
}

int dungeonJalan(LayarDungeon *d)
{
    for (;;) {
        int aksi = ui_menu_pick(d->aksi, d->nomor, d->nAksi, 1, 4,
                                gambarDungeon, d);
        if (aksi <= 0) return 1;
        /* jalankan aksi modul; hasilnya lewat ui_log / set_message */
    }
}
```

`Player` berasal dari `player.h` dan sudah tersedia untuk P2-P5. Struktur
`LayarDungeon` hanya contoh pemegang state layar; sesuaikan dengan modulmu.

## 10. Kalau layar bermasalah

| Gejala | Penyebab umum | Perbaikan |
|---|---|---|
| teks menempel di luar kotak Input | memakai `printf`/`readInt` saat mode interaktif | pakai `ui_line_*` |
| footer/pesan dobel | memanggil `ui_footer` dua kali dengan teks berbeda | satu `ui_screen_end(hint)` |
| prompt muncul dua kali | memanggil `ui_line_prompt()` setelah `ui_line_begin` | panggil begin sekali, atau prompt hanya setelah note/error |
| layar berkedip atau isi hilang | `ui_flush()` di tengah penggambaran | flush sekali di akhir frame |
| output skenario penuh escape | menggambar layar penuh saat mode pipa | bungkus dengan `ui_interactive()` |
| karakter rusak | ANSI manual atau asumsi lebar 1 untuk glyph lebar | pakai `lebar_tampil()`/`render_text_clipped` |
