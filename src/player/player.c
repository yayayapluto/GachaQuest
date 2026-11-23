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
    (void)p;
    (void)exp;
}

int totalAtk(const Player *p)
{
    return (p == NULL) ? 0 : p->atk;
}

int totalDef(const Player *p)
{
    return (p == NULL) ? 0 : p->def;
}

void showStatus(const Player *p)
{
    (void)p;
}
