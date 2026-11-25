#include <ctype.h>
#include <stdio.h>
#include "inventory.h"
#include "ui.h"
#include "render.h"
#include "style.h"
#include "util.h"

#define TABLE_ROWS 7 /* baris item yang tampil sekaligus di panel */

static const char *invItems[] = {"Urutkan", "Cari item", "Equip item", "Kembali"};
static const int invNums[] = {1, 2, 3, 0};

static const char *sortItems[] = {"Rarity (naik)", "Power (turun)", "Nama (A-Z)", "Batal"};
static const int sortNums[] = {1, 2, 3, 0};

/* ---------- fungsi bantu internal (static, tidak masuk header) ---------- */

static char lowerChar(char c)
{
    return (char)tolower((unsigned char)c);
}

/* bandingkan dua string tanpa peduli huruf besar-kecil */
static int cmpNameCI(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0')
    {
        char ca = lowerChar(*a);
        char cb = lowerChar(*b);
        if (ca != cb)
            return (int)(unsigned char)ca - (int)(unsigned char)cb;
        a++;
        b++;
    }
    return (int)(unsigned char)lowerChar(*a) - (int)(unsigned char)lowerChar(*b);
}

/* 1 bila word adalah substring dari name (tidak peka huruf besar-kecil) */
static int nameContains(const char *name, const char *word)
{
    int i, j;

    for (i = 0; name[i] != '\0'; ++i)
    {
        for (j = 0; word[j] != '\0'; ++j)
        {
            if (name[i + j] == '\0')
                return 0;
            if (lowerChar(name[i + j]) != lowerChar(word[j]))
                break;
        }
        if (word[j] == '\0')
            return 1;
    }
    return 0;
}

static void swapItems(Item *a, Item *b)
{
    Item t = *a;
    *a = *b;
    *b = t;
}

static int indexOfId(const Player *p, int id)
{
    int i;
    for (i = 0; i < p->itemCount; ++i)
    {
        if (p->inventory[i].id == id)
            return i;
    }
    return -1;
}

/* ---------- tampilan ---------- */

/* Tabel item di dalam panel. sel = baris tersorot (-1 bila tidak ada). */
static void gambarTabel(const Player *p, int sel)
{
    char title[40];
    int top = (sel >= 0) ? (sel / TABLE_ROWS) * TABLE_ROWS : 0;
    int r, i;

    snprintf(title, sizeof title, "Inventory (%d/%d)", p->itemCount, MAX_ITEM);
    ui_panel(8, 0, 50, TABLE_ROWS + 3, title);
    render_text(9, 2, ST_DIM, "%-5s%-17s %c %-9s %5s",
                "Idx", "Nama", 'T', "Rarity", "Power");

    for (r = 0; r < TABLE_ROWS; ++r)
    {
        char mark = ' ';
        Style st;

        i = top + r;
        if (i >= p->itemCount)
            break;
        if (i == p->weaponIndex)
            mark = '*';
        else if (i == p->armorIndex)
            mark = '#';

        st = (i == sel) ? ST_SELECTED : style_rarity(p->inventory[i].rarity);
        render_text(10 + r, 2, st, "%2d%c  %-17.17s %c %-9s %5d",
                    i, mark, p->inventory[i].name, p->inventory[i].type,
                    p->inventory[i].rarity == RARITY_LEGENDARY ? "Legendary"
                    : p->inventory[i].rarity == RARITY_EPIC    ? "Epic"
                    : p->inventory[i].rarity == RARITY_RARE    ? "Rare"
                                                               : "Common",
                    p->inventory[i].power);
    }
}

/* Layar menu Inventory. ctx = Player*, sel = pilihan menu */
static void gambarInventory(void *ctx, int sel)
{
    const Player *p = (const Player *)ctx;

    ui_frame("Inventory");
    if (p != NULL)
    {
        ui_status_panel_at(p, 2, 0, 50);
        gambarTabel(p, -1);
    }
    ui_menu_v(render_max_row() + 2, 0, invItems, invNums, 4, sel);
    ui_footer(NULL);
    ui_footer_message();
    ui_flush();
}

/* Layar pemilih baris untuk Equip. sel = baris tersorot */
static void gambarPilihBaris(void *ctx, int sel)
{
    const Player *p = (const Player *)ctx;

    ui_frame("Inventory");
    if (p != NULL)
    {
        ui_status_panel_at(p, 2, 0, 50);
        gambarTabel(p, sel);
    }
    ui_footer("Enter: pasang");
    ui_footer_message();
    ui_flush();
}

/* Layar pilih mode urut (tabel tetap terlihat). */
static void gambarUrut(void *ctx, int sel)
{
    const Player *p = (const Player *)ctx;

    ui_frame("Inventory");
    if (p != NULL)
    {
        ui_status_panel_at(p, 2, 0, 50);
        gambarTabel(p, -1);
    }
    ui_menu_v(render_max_row() + 2, 0, sortItems, sortNums, 4, sel);
    ui_footer(NULL);
    ui_footer_message();
    ui_flush();
}

/* ---------- kontrak publik ---------- */

void syncEquipped(Player *p, int oldWeaponId, int oldArmorId)
{
    if (p == NULL)
        return;
    p->weaponIndex = (oldWeaponId < 0) ? -1 : indexOfId(p, oldWeaponId);
    p->armorIndex = (oldArmorId < 0) ? -1 : indexOfId(p, oldArmorId);
}

void sortInventory(Player *p, int mode)
{
    int n, i, j, best;
    int oldWeaponId = -1, oldArmorId = -1;

    if (p == NULL || mode < 1 || mode > 3)
        return;
    n = p->itemCount;

    /* simpan id perlengkapan sebelum item berpindah posisi */
    if (p->weaponIndex >= 0 && p->weaponIndex < n)
        oldWeaponId = p->inventory[p->weaponIndex].id;
    if (p->armorIndex >= 0 && p->armorIndex < n)
        oldArmorId = p->inventory[p->armorIndex].id;

    if (mode == 1)
    {
        /* Bubble Sort stabil: rarity menaik (tukar hanya bila benar-benar lebih besar) */
        for (i = 0; i < n - 1; ++i)
            for (j = 0; j < n - 1 - i; ++j)
                if (p->inventory[j].rarity > p->inventory[j + 1].rarity)
                    swapItems(&p->inventory[j], &p->inventory[j + 1]);
    }
    else if (mode == 2)
    {
        /* Selection Sort: power menurun */
        for (i = 0; i < n - 1; ++i)
        {
            best = i;
            for (j = i + 1; j < n; ++j)
                if (p->inventory[j].power > p->inventory[best].power)
                    best = j;
            if (best != i)
                swapItems(&p->inventory[i], &p->inventory[best]);
        }
    }
    else
    {
        /* Bubble Sort stabil: nama A-Z, tidak peka huruf besar-kecil */
        for (i = 0; i < n - 1; ++i)
            for (j = 0; j < n - 1 - i; ++j)
                if (cmpNameCI(p->inventory[j].name, p->inventory[j + 1].name) > 0)
                    swapItems(&p->inventory[j], &p->inventory[j + 1]);
    }

    syncEquipped(p, oldWeaponId, oldArmorId);
}

/* Pencarian linear substring. Mengembalikan jumlah item yang cocok
   (0 bila tidak ada), sesuai yang dicek tests/t_inventory.c. */
int searchItemByName(const Player *p, const char *word)
{
    int i, count = 0;

    if (p == NULL || word == NULL || word[0] == '\0')
        return 0;
    for (i = 0; i < p->itemCount; ++i)
    {
        if (nameContains(p->inventory[i].name, word))
            count++;
    }
    return count;
}

void equipItem(Player *p, int index)
{
    if (p == NULL)
        return;
    if (index < 0 || index >= p->itemCount)
    {
        set_message("Indeks tidak ada di inventory.");
        return;
    }

    if (p->inventory[index].type == 'S')
    {
        p->weaponIndex = index;
        set_message("%s dipasang.", p->inventory[index].name);
    }
    else if (p->inventory[index].type == 'A')
    {
        p->armorIndex = index;
        set_message("%s dipasang.", p->inventory[index].name);
    }
    else
    {
        set_message("%s bukan senjata atau armor.", p->inventory[index].name);
    }
}

void inventoryMenu(Player *p)
{
    int choice, mode, row, i, found;
    char word[MAX_NAME];

    if (p == NULL)
        return;
    set_message("");

    for (;;)
    {
        choice = ui_menu_pick(invItems, invNums, 4, 0, 3, gambarInventory, p);
        if (choice <= 0)
            return;

        if (choice == 1)
        {
            mode = ui_menu_pick(sortItems, sortNums, 4, 0, 3, gambarUrut, p);
            if (mode >= 1 && mode <= 3)
            {
                sortInventory(p, mode);
                set_message("Inventory terurut.");
            }
        }
        else if (choice == 2)
        {
            if (!readStringTry("Cari nama item:", word, MAX_NAME))
                continue;
            found = searchItemByName(p, word);
            if (found == 0)
            {
                set_message("Tidak ada item yang cocok.");
            }
            else
            {
                for (i = 0; i < p->itemCount; ++i)
                    if (nameContains(p->inventory[i].name, word))
                        break;
                set_message("Ditemukan %d: idx %d %s (tipe %c, power %d)",
                            found, i, p->inventory[i].name,
                            p->inventory[i].type, p->inventory[i].power);
            }
        }
        else if (choice == 3)
        {
            if (p->itemCount == 0)
            {
                set_message("Inventory kosong.");
                continue;
            }
            row = ui_pick_row(p->itemCount, gambarPilihBaris, p);
            if (row >= 0)
                equipItem(p, row);
        }
    }
}