# Gacha Quest

Repo kerja tim untuk **Gacha Quest**, permainan terminal C11 dengan battle, gacha,
inventory, shop, storage, dan dungeon prosedural yang deterministik per seed.
Header modul, data katalog, driver uji, `Makefile`, `main.c`, lapisan P1, dan stub
P2–P5 sudah ada. Panduan peran, PRD, dan pembagian tugas ada di repo
[Panduan-GachaQuest](https://github.com/yayayapluto/Panduan-GachaQuest) dan disalin
ke `docs/` repo ini.

Status per 22 Nov 2026: branch `main` berisi scaffold P1 dengan lapisan UI dan
platform hidup. `make` bersih tanpa warning; `make test` menjalankan 23 cek dan
menghasilkan **6 lulus, 17 gagal** karena modul P2–P5 masih stub. Angka ini
bukan target: target rilis adalah 23 lulus, 0 gagal setelah semua modul selesai.

## Daftar Isi

1. [PRD](docs/PRD.md) — kebutuhan produk, aturan game, kontrak modul, kriteria penerimaan.
2. [Project Overview](docs/PROJECT-OVERVIEW.md) — gambaran singkat, struktur repo target, alur runtime.
3. [Pembagian Tugas](docs/PEMBAGIAN-TUGAS.md) — lima peran, fase, tag, jadwal, definition of done.
4. [Panduan 0 — Git dan Kompilasi](docs/panduan-0-git-dan-kompilasi.md) — menyiapkan lingkungan, repo, dan build pertama. Panduan ini juga memuat alur Git per anggota (branch per modul, PR ke `dev`, dan tag) di bagian "Alur Git per Anggota".
5. [Panduan P1 — Scaffold, UI, dan Integrasi](docs/panduan-P1-scaffold-ui.md)
6. [Panduan P2 — Dungeon](docs/panduan-P2-dungeon.md)
7. [Panduan P3 — Battle dan Player](docs/panduan-P3-battle-player.md)
8. [Panduan P4 — Gacha dan Inventory](docs/panduan-P4-gacha-inventory.md)
9. [Panduan P5 — Shop dan Storage](docs/panduan-P5-shop-storage.md)

Urutannya disengaja. Baca PRD lebih dulu karena di sana semua angka dan kontrak
fungsi ditetapkan. Sesudah itu panduan peran bisa dibaca sesuai tanggung jawab
masing-masing, sedangkan Panduan 0 dipakai bersama saat menyiapkan mesin kerja.

## Prasyarat

- **GCC** dengan dukungan C11 (di Windows, pakai MSYS2 atau Git Bash yang membawa
  toolchain `gcc`).
- **Make** untuk menjalankan target `all`, `start`, `test`, dan `clean`.
- **Git** untuk branch per fitur, commit, dan pull request.
- **Terminal minimal 50 kolom × 20 baris.** Di bawah ukuran itu, tampilan UI
  menolak berjalan dan meminta terminal diperbesar.

Cek ketiganya sebelum mulai:

```sh
gcc --version
make --version
git --version
```

## Mulai Cepat

Clone repo ini, masuk ke foldernya, lalu build dan jalankan:

```sh
git clone https://github.com/yayayapluto/GachaQuest.git
cd GachaQuest
make
make start
make test
```

Perintah `make` dan `make test` sudah bisa dijalankan di branch `main`. `make start`
membutuhkan terminal interaktif minimal 50×20.

Repo itu publik: clone dan baca bebas, tetapi push butuh izin tulis dari pemilik.
Cara mengatur remote, minta izin, push branch, dan membuka PR ada di bagian
[Push ke Repo Bersama](docs/panduan-0-git-dan-kompilasi.md#push-ke-repo-bersama)
pada Panduan 0.

## Status Implementasi P1

Lapisan P1 sudah terpasang dan hidup: `Makefile`, `main.c`, `src/util`,
`src/ui`, dan `src/platform`. Menu Utama, Game Baru, Load Game, Leaderboard,
dan Hub bisa digambar dan dikendalikan; mode non-interaktif tetap mencetak teks
polos tanpa kode escape.

Modul P2 sampai P5 masih berupa stub netral sesuai pembagian tugas. Karena itu
`make` berhasil tanpa warning, tetapi `make test` baru menghasilkan 6 lulus dan
17 gagal dari 23 cek. Driver P1 (`t_platform`) dan skenario utilisasi
(`S01/S02 bacaAngka & bacaString`) lulus; sisanya menunggu implementasi modul.

## Branch per Peran

Nama branch di bawah ini sama dengan properti `Branch` di tracker Notion.

| Peran | Branch |
|---|---|
| P1 scaffold, UI, platform | `feat/scaffold-ui` |
| P1 integrasi | `feat/integrasi` |
| P1 rilis | `feat/rilis` |
| P2 dungeon | `feat/dungeon` |
| P3 battle dan player | `feat/battle-player` |
| P4 gacha dan inventory | `feat/gacha-inventory` |
| P5 shop dan storage | `feat/shop-storage` |

Branch `main` adalah satu-satunya branch tetap: semua PR per peran masuk ke sana.

## Alur Kerja per Anggota

Dari clone sampai pull request, urutannya begini:

1. `git clone https://github.com/yayayapluto/GachaQuest.git` lalu `cd GachaQuest`.
2. `git checkout -b feat/<modul>` — satu branch per modul, misalnya `feat/dungeon`.
3. Baca `docs/PRD.md` bagian 6, 7, dan 8 minimal sekali: aturan game, kontrak
   fungsi per modul, dan format file. Semua angka yang kamu butuhkan ada di sana.
4. Buka panduan peranmu sendiri, ikuti `## Langkah Kerja` berurutan, dan cek
   setiap blok `Expected` sebelum lanjut ke langkah berikutnya.
5. Setelah P1 menyelesaikan Milestone 1–2, jalankan `make` lalu `make test`
   sebelum commit. Keduanya harus bersih dari warning dan kegagalan.
6. Commit dengan format `feat(<modul>): ...`, `git push`, buka PR ke `main`,
   lalu tunggu review P1.

## Panduan dan Lampiran untuk Bertanya ke AI

Saat meminta bantuan AI, lampirkan panduan peranmu plus header modul yang
dipakai. Jangan melampirkan seluruh repo.

| Anggota | Panduan yang dilampirkan | Header pendukung | Skenario yang harus lulus |
|---|---|---|---|
| P1 | [Panduan P1](docs/panduan-P1-scaffold-ui.md) | `src/core/config.h`, `src/util/util.h`, `src/platform/*.h` | S01, S02, S17, dan 23 cek `make test` |
| P2 | [Panduan P2](docs/panduan-P2-dungeon.md) | `src/dungeon/dungeon.h`, `src/core/config.h` | S12, S13, S14, S16 |
| P3 | [Panduan P3](docs/panduan-P3-battle-player.md) | `src/player/player.h`, `src/battle/battle.h` | S03, S04, S18 |
| P4 | [Panduan P4](docs/panduan-P4-gacha-inventory.md) | `src/gacha/gacha.h`, `src/inventory/inventory.h` | S05, S08, S09 |
| P5 | [Panduan P5](docs/panduan-P5-shop-storage.md) | `src/storage/storage.h`, `src/shop/shop.h` | S09, S10, S11, S15 |

Setiap panduan peran sudah memuat `## Konteks untuk AI` berisi angka pasti,
prototipe fungsi, format file, larangan, dan skenario uji. Jadi cukup lampirkan
panduan itu, header pendukung di atas, dan file yang sedang kamu kerjakan.
Konteksnya sudah lengkap tanpa seluruh repo, dan jawaban AI tidak akan mengarang
aturan yang tidak ada di kontrak. Untuk urusan git, kompilasi, dan build, ikut
lampirkan juga `docs/panduan-0-git-dan-kompilasi.md`; dokumen itu berlaku untuk
semua anggota.

## Struktur Repo

Repo ini sudah berisi implementasi scaffold, bukan hanya header:

- `Makefile`, `main.c` — target `all`, `start`, `test`, `clean` dan alur menu utama.
- `src/**/*.h` — kontrak publik seluruh modul. Perubahan header lewat P1.
- `src/**/*.c` — implementasi P1 di `util`, `ui`, dan `platform`; modul P2–P5
  masih stub yang mengembalikan nilai netral.
- `data/item.txt` dan `data/monster.txt` — katalog contoh berformat pipa.
- `tests/` — `run_tests.sh`, sembilan driver `t_*`, dan input skenario S01–S18.

Header dipakai apa adanya; kalau sebuah fungsi butuh tanda tangan berbeda,
perubahan diusulkan ke P1 supaya semua pemilik modul menyesuaikan diri pada versi
yang sama.

## Peran Sekilas

| Kode | Level | Peran | File milik |
|---|---|---|---|
| P1 | mahir | UI, platform, integrator | `Makefile`, `main.c`, `src/core/config.h`, `src/util/*`, `src/ui/*`, `src/platform/*`, `README.md` |
| P2 | mahir | Dungeon | `src/dungeon/dungeon.c`, `src/dungeon/dungeon.h`, `tests/t_dungeon.c` |
| P3 | awam | Battle dan player | `src/player/*`, `src/battle/*` |
| P4 | awam | Gacha dan inventory | `src/gacha/*`, `src/inventory/*` |
| P5 | awam | Shop, storage, leaderboard | `src/shop/*`, `src/storage/*` |

Aturan mainnya: satu file satu pemilik. P3 sampai P5 tidak menyentuh `src/ui`,
tidak memanggil `printf` untuk layar, dan tidak memakai `rand()` melainkan
`plat_rand()`. Rincian fase dan cara review ada di
[Pembagian Tugas](docs/PEMBAGIAN-TUGAS.md).

## Menjalankan Pengujian

Dari root repo:

```sh
sh tests/run_tests.sh
```

Skrip ini mengompilasi game dan sembilan driver `t_*`, lalu menjalankan 13
pemanggilan skenario S01–S18. Totalnya 23 cek karena S16 diuji dua kali. Setiap
baris hasil menandai lulus atau gagal. Pengujian butuh `gcc` dan `make` yang sudah
terpasang, jadi selesaikan dulu langkah pada
[Panduan 0](docs/panduan-0-git-dan-kompilasi.md).

## Checklist Demo

Sebelum presentasi, pastikan dua belas layar berikut bisa ditunjukkan tanpa error:

1. **Menu Utama** — Game Baru, Load Game, Leaderboard, Keluar.
2. **Game Baru** — input nama (maksimal 29 karakter) dan seed dunia.
3. **Load Game** — lima slot, slot kosong ditandai, data rusak menghasilkan pesan.
4. **Leaderboard** — sepuluh entri tersimpan, lima teratas ditampilkan urut skor.
5. **Hub** — pintu masuk ke Battle, Dungeon, Gacha, Inventory, Shop, dan Status.
6. **Battle** — menu Serang, Bertahan, Potion, Kabur beserta log pertarungan.
7. **Gacha** — pull 1x (100 gold) dan 10x (900 gold), pity, duplikat.
8. **Inventory** — urutkan, cari item, pasang senjata dan armor.
9. **Shop** — beli potion dan jual item dengan konfirmasi.
10. **Status** — HP, level, EXP, gold, serangan, pertahanan, daftar item terpasang.
11. **Dungeon** — gerak, kamera, treasure, hint, chest, dan monster.
12. **Game Over** — Mulai ulang (reset run) atau kembali ke menu utama.

Skenario penerimaan di PRD §11 memetakan tiap layar ke ID uji, jadi kegagalan
demo biasanya bisa dilacak ke satu baris `run_tests.sh` tertentu.

## Repo Ini Mandiri

Seluruh aturan, angka, format file, dan daftar fungsi publik ditulis ulang di
dalam paket ini. Tidak ada tautan keluar, tidak ada rujukan ke proyek lain, dan
tidak ada bagian yang bergantung pada berkas di luar folder ini. Kalau sebuah
keputusan belum tercantum di PRD, keputusan itu belum menjadi bagian kontrak:
diskusikan dengan P1 dan perbarui dokumennya lebih dulu sebelum menulis kode.
