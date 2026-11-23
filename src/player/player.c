#include <stddef.h>
#include "player.h"

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

void showStatus(const Player *p)
{
    (void)p;
}
