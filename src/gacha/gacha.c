#include <stddef.h>
#include "gacha.h"

void gachaMenu(Player *p, const Item catalog[], int count)
{
    (void)p;
    (void)catalog;
    (void)count;
}

int rollRarity(int *pity)
{
    if (pity != NULL) ++(*pity);
    return RARITY_COMMON;
}

int findItemInInventory(const Player *p, int id)
{
    int i;
    if (p == NULL) return -1;
    for (i = 0; i < p->itemCount; ++i) {
        if (p->inventory[i].id == id) return i;
    }
    return -1;
}

int duplicatePrice(int rarity)
{
    switch (rarity) {
        case RARITY_RARE: return DUPLICATE_PRICE_RARE;
        case RARITY_EPIC: return DUPLICATE_PRICE_EPIC;
        case RARITY_LEGENDARY: return DUPLICATE_PRICE_LEGENDARY;
        default: return DUPLICATE_PRICE_COMMON;
    }
}

const char *rarityName(int rarity)
{
    switch (rarity) {
        case RARITY_RARE: return "Rare";
        case RARITY_EPIC: return "Epic";
        case RARITY_LEGENDARY: return "Legendary";
        default: return "Common";
    }
}

int pullOnce(Player *p, const Item catalog[], int count, int rarity)
{
    (void)p;
    (void)catalog;
    (void)count;
    (void)rarity;
    return 1;
}
