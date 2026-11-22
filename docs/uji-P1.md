# Log uji P1 — S01–S18

Dijalankan pada `main` commit `d6c67cd`, Windows + MSYS2 GCC, lewat:

```sh
make
sh tests/run_tests.sh
```

Hasil: **6 lulus, 17 gagal** dari 23 cek (9 driver + 13 skenario + S16 ganda).
17 kegagalan berasal dari modul P2–P5 yang masih stub, bukan dari lapisan P1.

## Driver

| Cek | Hasil | Modul |
|---|---|---|
| Pemain: statistik awal, EXP, naik level, total serangan | gagal | P3 `player` |
| Battle: rumus damage dan rentang hasil | lulus | P3 `battle` |
| S08 urut & cari inventory | gagal | P4 `inventory` |
| S06/S07 rarity, pity, duplikat | gagal | P4 `gacha` |
| S12/S14 dungeon prosedur: seed deterministik, DFS & BFS | gagal | P2 `dungeon` |
| S17 autosave atomik: CRC, .bak, kunci sesi | lulus | P1 `platform` |
| S10 save-load round trip | gagal | P5 `storage` |
| S15 leaderboard urut & top 10 | gagal | P5 `storage` |
| S01/S02 bacaAngka & bacaString | lulus | P1 `util` |

## Skenario

| Skenario | Hasil | Penyebab |
|---|---|---|
| S01 nama kosong/terlalu panjang | gagal | `showStatus` P3 masih stub |
| S02 input huruf pada menu | lulus | — |
| S03 battle sampai menang | gagal | `battle` P3 stub |
| S04 battle sampai kalah | gagal | `battle` P3 stub |
| S18 kalah membuka layar game over | gagal | alur game over P3 |
| S05 gacha 10x gold kurang | gagal | `gacha` P4 stub |
| S09 jual item | gagal | `inventory`/`shop` P4/P5 stub |
| S10 simpan-lanjutkan | gagal | `storage` P5 stub |
| S15 leaderboard tercatat | gagal | `storage` P5 stub |
| S11 save.txt dirusak | lulus | — |
| S12 gerak ke tembok | gagal | `dungeon` P2 stub |
| S13 hint gold kurang | gagal | `dungeon` P2 stub |
| S16 seed dunia diminta saat game baru | lulus | — |
| S16 seed yang sama dipakai lagi setelah Load Game | gagal | `storage` P5 stub |

## Cara memperbarui

Jalankan `sh tests/run_tests.sh`, ganti tabel di atas dengan keluaran asli, dan
catat commit yang diuji. Target rilis tetap 23 lulus, 0 gagal.
