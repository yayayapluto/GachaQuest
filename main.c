#include <stdio.h>

#include "config.h"
#include "util.h"
#include "ui.h"
#include "term.h"
#include "platform.h"
#include "render.h"
#include "autosave.h"
#include "player.h"
#include "battle.h"
#include "gacha.h"
#include "inventory.h"
#include "shop.h"
#include "storage.h"
#include "dungeon.h"

static Player pemain;
static Item katalog[MAX_ITEM];
static Monster monsters[MAX_MONSTER];
static int jumlahItem;
static int jumlahMonster;

static void gambarJudul(const char *judul, const char *info)
{
    ui_frame(judul);
    if (info != NULL) render_text(4, 4, ST_TEXT, "%s", info);
}

static const char *const menuUtamaItems[] = {
    "Game Baru", "Load Game", "Leaderboard", "Keluar"
};
static const int menuUtamaNums[] = { 1, 2, 3, 0 };
static const char *const hubItems[] = {
    "Status", "Battle", "Dungeon", "Gacha",
    "Inventory", "Shop", "Simpan", "Keluar ke Menu Utama"
};
static const int hubNums[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

static void gambarMenuUtama(void *ctx, int sel)
{
    (void)ctx;
    gambarJudul("GACHA QUEST", "Petualangan terminal deterministik per seed");
    ui_menu_v(6, 6, menuUtamaItems, menuUtamaNums, 4, sel);
    ui_footer("Panah: pilih | Enter: ok | Esc: batal");
    ui_message(17);
    ui_flush();
}

static void gambarHub(void *ctx, int sel)
{
    (void)ctx;
    gambarJudul("HUB", "Pusat kegiatan petualangan");
    ui_menu_v(5, 6, hubItems, hubNums, 8, sel);
    ui_footer("Panah: pilih | Enter: ok | Esc: kembali");
    ui_message(17);
    ui_flush();
}

static void gambarSlot(void *ctx, int sel)
{
    char label[SAVE_SLOTS][MAX_NAME];
    const char *items[SAVE_SLOTS];
    int nums[SAVE_SLOTS];
    int i;

    (void)ctx;
    for (i = 0; i < SAVE_SLOTS; ++i) {
        saveSlotName(i + 1, label[i], MAX_NAME);
        if (!saveSlotExists(i + 1)) {
            int n;
            for (n = 0; label[i][n] != '\0'; ++n) {
                if (label[i][n] == ' ') label[i][n] = ' ';
            }
        }
        items[i] = label[i];
        nums[i] = i + 1;
    }
    gambarJudul("LOAD GAME", "Pilih slot penyimpanan");
    ui_menu_v(6, 6, items, nums, SAVE_SLOTS, sel);
    ui_footer("Enter: muat | Esc: batal");
    ui_message(17);
    ui_flush();
}

static void sebarSnapshot(int screen)
{
    autosave_publish(&pemain, screen, NULL, 0, NULL, 0, 0, 1);
}

static void muatKatalog(void)
{
    if (jumlahItem == 0) jumlahItem = loadCatalog("data/item.txt", katalog);
    if (jumlahMonster == 0) jumlahMonster = loadMonsters("data/monster.txt", monsters);
}

static int mintaNama(char *out, int max)
{
    int status;

    ui_input_screen("GAME BARU");
    for (;;) {
        ui_line_begin("Nama pemain: ");
        status = ui_line_read(out, max);
        ui_line_end();
        if (status <= 0) return 0;
        if (out[0] != '\0') return 1;
        ui_line_error("Nama tidak boleh kosong.");
    }
}

static void bangunDunia(unsigned int seed, int dariSave)
{
    pemain.seed = seed;
    muatKatalog();
    ui_log("dunia dibangun dari seed %u", seed);
    if (dariSave) set_message("Dunia dibangun ulang dari seed %u", seed);
}

static void gameBaru(void)
{
    unsigned int seed;
    char nama[MAX_NAME];

    if (!mintaNama(nama, MAX_NAME)) return;
    initPlayer(&pemain, nama);
    term_cooked();
    seed = promptWorldSeed();
    term_raw();
    bangunDunia(seed, 0);
    set_message("Selamat datang, %s!", pemain.name);
}

static void bukaGame(void)
{
    int slot;
    unsigned int seed;

    slot = ui_menu_pick(NULL, NULL, SAVE_SLOTS, 1, SAVE_SLOTS, gambarSlot, NULL);
    if (slot < 1 || slot > SAVE_SLOTS) return;
    muatKatalog();
    if (!loadGameSlot(&pemain, katalog, jumlahItem, slot)) {
        set_message("Gagal memuat slot %d", slot);
        return;
    }
    seed = pemain.seed;
    bangunDunia(seed, 1);
}

static void hub(void)
{
    for (;;) {
        int pilihan;
        if (platform_shutdown_requested()) return;
        sebarSnapshot(AS_SCREEN_HUB);
        pilihan = ui_menu_pick(hubItems, hubNums, 8, 1, 8, gambarHub, NULL);
        if (pilihan == 8 || pilihan < 1) return;
        switch (pilihan) {
            case 1: showStatus(&pemain); break;
            case 2: startBattle(&pemain, monsters, jumlahMonster); break;
            case 3: runDungeon(&pemain, katalog, jumlahItem, monsters, jumlahMonster); break;
            case 4: gachaMenu(&pemain, katalog, jumlahItem); break;
            case 5: inventoryMenu(&pemain); break;
            case 6: shopMenu(&pemain); break;
            case 7:
                if (saveGameSlot(&pemain, saveSlotFirstFree())) {
                    set_message("Permainan tersimpan.");
                } else {
                    set_message("Gagal menyimpan permainan.");
                }
                break;
            default: break;
        }
        sebarSnapshot(AS_SCREEN_HUB);
    }
}

int main(void)
{
    platform_init();
    ui_init();
    initPlayer(&pemain, "");
    muatKatalog();

    while (!platform_shutdown_requested()) {
        int pilihan = ui_menu_pick(menuUtamaItems, menuUtamaNums, 4, 0, 3, gambarMenuUtama, NULL);
        if (pilihan < 0 || pilihan == 0) break;
        sebarSnapshot(AS_SCREEN_MENU);
        switch (pilihan) {
            case 1: gameBaru(); if (pemain.name[0] != '\0') hub(); break;
            case 2: bukaGame(); if (pemain.name[0] != '\0') hub(); break;
            case 3: showLeaderboard(); break;
            default: break;
        }
    }

    ui_shutdown();
    shutdownGame();
    return 0;
}
