#include <stddef.h>
#include "gacha.h"
#include "ui.h"
#include "render.h"
#include "style.h"
#include "rng.h"

static const char *gachaItems[] = {"Pull 1x (100 gold)", "Pull 10x (900 gold)", "Kembali"};
static const int gachaNums[] = {1, 2, 0};

static void gambarGacha(void *ctx, int sel)
{
    const Player *p = (const Player *)ctx;
    int i, n, start;

    ui_frame("Gacha");

    /* Panel Status: baris 2..7 */
    if (p != NULL)
        ui_status_panel_at(p, 2, 0, 50);

    /* Panel PULL: baris 8..12 */
    ui_panel(8, 0, 50, 5, "PULL");
    if (p != NULL)
    {
        render_text(9, 2, ST_TEXT, "Pity      : %d/%d", p->pity, PITY_THRESHOLD);
        render_text(10, 2, ST_TEXT, "Total pull: %d", p->totalPull);
        ui_bar(11, 3, 40, p->pity, PITY_THRESHOLD, ST_ACCENT);
    }

    /* Panel Hasil: isi 4 baris log terakhir */
    ui_panel(14, 0, 50, 6, "Hasil");
    n = ui_log_count();
    start = (n > 4) ? n - 4 : 0;
    for (i = start; i < n; ++i)
    {
        ui_log_draw(15 + (i - start), 2, 46, i);
    }

    /* Menu di bawah panel */
    ui_menu_v(render_max_row() + 2, 0, gachaItems, gachaNums, 3, sel);
    ui_footer(NULL);
    ui_footer_message();
    ui_flush();
}

/* Satu pull: bayar, undi, simpan. Mengembalikan 1 bila pull berhasil
   (item baru atau duplikat yang jadi gold), 0 bila gagal (uang kembali). */
static int doOnePull(Player *p, const Item catalog[], int count, int cost)
{
    int rarity, got;
    int oldPity = p->pity;
    int oldCount = p->itemCount;
    int goldAfterPay;

    p->gold -= cost;
    goldAfterPay = p->gold;
    rarity = rollRarity(&p->pity);
    p->totalPull++;

    got = pullOnce(p, catalog, count, rarity);
    if (!got)
    {
        /* pull gagal: uang, pity, dan hitungan pull dikembalikan */
        p->gold += cost;
        p->pity = oldPity;
        p->totalPull--;

        if (p->itemCount >= MAX_ITEM)
            ui_log("[%s] tas penuh, gold dikembalikan.", rarityName(rarity));
        else
            ui_log("[%s] gagal: katalog kosong atau tanpa item ini.",
                   rarityName(rarity));
        return 0;
    }

    if (p->itemCount > oldCount)
        ui_log("[%s] %s (power %d) -> baru!", rarityName(rarity),
               p->inventory[p->itemCount - 1].name,
               p->inventory[p->itemCount - 1].power);
    else
        ui_log("[%s] DUPLIKAT, jadi +%d gold", rarityName(rarity),
               p->gold - goldAfterPay);
    return 1;
}

void gachaMenu(Player *p, const Item catalog[], int count)
{
    int choice, i;

    if (p == NULL || catalog == NULL)
        return;

    ui_log_clear();  /* buang sisa log dari layar lain */
    set_message(""); /* buang pesan "Selamat datang" */

    for (;;)
    {
        choice = ui_menu_pick(gachaItems, gachaNums, 3, 0, 2, gambarGacha, p);

        if (choice == 0 || choice < 0)
            return;

        if (choice == 1)
        {
            if (p->gold < PULL_COST)
            {
                set_message("Gold kurang! Butuh %d gold", PULL_COST);
                continue;
            }
            ui_log_clear();
            doOnePull(p, catalog, count, PULL_COST);
        }
        else if (choice == 2)
        {
            if (p->gold < PULL10_COST)
            {
                set_message("Gold kurang! Butuh %d gold", PULL10_COST);
                continue;
            }
            ui_log_clear();
            /* 900 untuk 10 pull = 90 per pull */
            for (i = 0; i < 10; ++i)
            {
                if (!doOnePull(p, catalog, count, PULL10_COST / 10))
                    break;
            }
        }
    }
}

int rollRarity(int *pity)
{
    int roll = (int)(plat_rand() % 100) + 1; /* 1..100 */
    int rarity;

    if (roll >= 98)
        rarity = RARITY_LEGENDARY;
    else if (roll >= 86)
        rarity = RARITY_EPIC;
    else if (roll >= 61)
        rarity = RARITY_RARE;
    else
        rarity = RARITY_COMMON;

    if (pity == NULL)
        return rarity;

    if (rarity == RARITY_EPIC)
    {
        *pity = 0;
    }
    else
    {
        (*pity)++;
        if (*pity >= PITY_THRESHOLD)
        {
            rarity = RARITY_EPIC;
            *pity = 0;
        }
    }
    return rarity;
}

int findItemInInventory(const Player *p, int id)
{
    int i;
    if (p == NULL)
        return -1;
    for (i = 0; i < p->itemCount; ++i)
    {
        if (p->inventory[i].id == id)
            return i;
    }
    return -1;
}

int duplicatePrice(int rarity)
{
    switch (rarity)
    {
    case RARITY_RARE:
        return DUPLICATE_PRICE_RARE;
    case RARITY_EPIC:
        return DUPLICATE_PRICE_EPIC;
    case RARITY_LEGENDARY:
        return DUPLICATE_PRICE_LEGENDARY;
    default:
        return DUPLICATE_PRICE_COMMON;
    }
}

const char *rarityName(int rarity)
{
    switch (rarity)
    {
    case RARITY_RARE:
        return "Rare";
    case RARITY_EPIC:
        return "Epic";
    case RARITY_LEGENDARY:
        return "Legendary";
    default:
        return "Common";
    }
}

int pullOnce(Player *p, const Item catalog[], int count, int rarity)
{
    int i, matches = 0, pick, chosen = -1;

    if (p == NULL || catalog == NULL || count <= 0)
        return 0;

    /* hitung item katalog dengan rarity ini */
    for (i = 0; i < count; ++i)
    {
        if (catalog[i].rarity == rarity)
            matches++;
    }
    if (matches == 0)
        return 0;

    /* ambil salah satu secara acak */
    pick = (int)(plat_rand() % (unsigned)matches);
    for (i = 0; i < count; ++i)
    {
        if (catalog[i].rarity == rarity)
        {
            if (pick == 0)
            {
                chosen = i;
                break;
            }
            pick--;
        }
    }
    if (chosen < 0)
        return 0;

    /* duplikat: id sudah ada di tas -> tidak ditambah, dibayar gold */
    if (findItemInInventory(p, catalog[chosen].id) >= 0)
    {
        p->gold += duplicatePrice(rarity);
        return 1;
    }

    /* item baru tapi tas penuh: tidak masuk, pemanggil mengembalikan uang */
    if (p->itemCount >= MAX_ITEM)
        return 0;

    p->inventory[p->itemCount] = catalog[chosen];
    p->itemCount++;
    return 1;
}