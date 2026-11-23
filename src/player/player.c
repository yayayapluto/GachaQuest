#include <stddef.h>
#include "player.h"
#include "ui.h"
#include "render.h"

void initPlayer(Player *p, const char *name)
{
    int i;

    if (p == NULL) return;
    for (i = 0; i < (int)sizeof(p->name); ++i) p->name[i] = '\0';
    if (name != NULL) {
        for (i = 0; name[i] != '\0' && i < MAX_NAME - 1; ++i) p->name[i] = name[i];
    }
    p->hp = 100;
    p->maxHp = 100;
    p->atk = 10;
    p->def = 3;
    p->level = 1;
    p->exp = 0;
    p->gold = 200;
    p->potion = 2;
    p->pity = 0;
    p->totalPull = 0;
    p->score = 0;
    p->seed = 0;
    p->itemCount = 0;
    p->weaponIndex = -1;
    p->armorIndex = -1;
}

void resetPlayer(Player *p)
{
    char name[MAX_NAME];
    unsigned int seed;
    int i;

    if (p == NULL) return;
    for (i = 0; i < MAX_NAME; ++i) name[i] = p->name[i];
    seed = p->seed;
    initPlayer(p, name);
    p->seed = seed;
}

void addExp(Player *p, int exp)
{
    if (p == NULL || exp <= 0) return;
    p->exp += exp;
    while (p->level < MAX_LEVEL && p->exp >= p->level * 50) {
        p->exp -= p->level * 50;
        p->level++;
        p->maxHp += 15;
        p->atk += 2;
        p->def += 1;
        p->hp = p->maxHp;
    }
    if (p->level >= MAX_LEVEL) p->exp = 0;
}

int totalAtk(const Player *p)
{
    int total;

    if (p == NULL) return 0;
    total = p->atk;
    if (p->weaponIndex >= 0 && p->weaponIndex < MAX_ITEM)
        total += p->inventory[p->weaponIndex].power;
    return total;
}

int totalDef(const Player *p)
{
    int total;

    if (p == NULL) return 0;
    total = p->def;
    if (p->armorIndex >= 0 && p->armorIndex < MAX_ITEM)
        total += p->inventory[p->armorIndex].power;
    return total;
}

static void gambarStatus(void *ctx, int sel)
{
    const Player *p = ctx;
    int r;

    (void)sel;
    ui_screen_begin("Status", NULL);
    ui_status_panel_at(p, UI_CONTENT_ROW, 2, 46);
    r = render_max_row() + 2;
    ui_panel(r, 2, 46, 10, "STATISTIK & PERLENGKAPAN");

    if (p->level >= MAX_LEVEL)
        render_text(r + 1, 4, ST_TEXT, "Level      : %d (MAX)", p->level);
    else
        render_text(r + 1, 4, ST_TEXT, "Level      : %d (EXP %d/%d)",
                    p->level, p->exp, p->level * 50);
    render_text(r + 2, 4, ST_DIM, "----------------------------------------");
    render_text(r + 3, 4, ST_TEXT, "ATK        : %d   DEF: %d", totalAtk(p), totalDef(p));
    render_text(r + 4, 4, ST_TEXT, "Potion     : %d   Pity: %d/%d", p->potion, p->pity, PITY_THRESHOLD);
    render_text(r + 5, 4, ST_TEXT, "Total pull : %d   Item: %d/%d", p->totalPull, p->itemCount, MAX_ITEM);
    render_text(r + 6, 4, ST_TEXT, "Seed dunia : %u", p->seed);

    if (p->weaponIndex >= 0 && p->weaponIndex < p->itemCount)
        render_text_clipped(r + 7, 4, 40, ST_TEXT, "Senjata    : %s (+%d)",
                            p->inventory[p->weaponIndex].name,
                            p->inventory[p->weaponIndex].power);
    else
        render_text(r + 7, 4, ST_DIM, "Senjata    : -");

    if (p->armorIndex >= 0 && p->armorIndex < p->itemCount)
        render_text_clipped(r + 8, 4, 40, ST_TEXT, "Armor      : %s (+%d)",
                            p->inventory[p->armorIndex].name,
                            p->inventory[p->armorIndex].power);
    else
        render_text(r + 8, 4, ST_DIM, "Armor      : -");

    ui_screen_end("Enter / Esc untuk kembali");
}

void showStatus(const Player *p)
{
    static const char *const kembali[] = { "Kembali" };

    if (p == NULL) return;
    ui_menu_pick(kembali, NULL, 1, 0, 0, gambarStatus, (void *)p);
}
