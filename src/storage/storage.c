#include <stddef.h>
#include "storage.h"

int loadCatalog(const char *file, Item items[])
{
    (void)file;
    (void)items;
    return 0;
}

int loadMonsters(const char *file, Monster monsters[])
{
    (void)file;
    (void)monsters;
    return 0;
}

int saveGame(const Player *p)
{
    (void)p;
    return 0;
}

int saveGameSlot(const Player *p, int slot)
{
    (void)p;
    (void)slot;
    return 0;
}

int loadGame(Player *p, const Item items[], int count)
{
    return loadGameSlot(p, items, count, 1);
}

int loadGameSlot(Player *p, const Item items[], int count, int slot)
{
    (void)p;
    (void)items;
    (void)count;
    (void)slot;
    return 0;
}

void saveSlotPath(int slot, char *out, int size)
{
    const char *prefix = "data/save_";
    int i = 0;
    if (out == NULL || size <= 0) return;
    while (prefix[i] != '\0' && i < size - 1) { out[i] = prefix[i]; ++i; }
    if (i < size - 1) out[i++] = (char)('0' + slot);
    if (i < size - 1) out[i++] = '.';
    if (i < size - 1) out[i++] = 't';
    if (i < size - 1) out[i++] = 'x';
    if (i < size - 1) out[i++] = 't';
    out[i < size ? i : size - 1] = '\0';
}

int saveSlotExists(int slot)
{
    (void)slot;
    return 0;
}

int saveSlotName(int slot, char *out, int size)
{
    int i;
    if (out == NULL || size <= 0) return 0;
    for (i = 0; i < size; ++i) out[i] = '\0';
    out[0] = 'S';
    out[1] = 'l';
    out[2] = 'o';
    out[3] = 't';
    out[4] = ' ';
    out[5] = (char)('0' + slot);
    out[6] = ' ';
    out[7] = '-';
    out[8] = ' ';
    out[9] = 'k';
    out[10] = 'o';
    out[11] = 's';
    out[12] = 'o';
    out[13] = 'n';
    out[14] = 'g';
    return 1;
}

int saveSlotFirstFree(void)
{
    return 1;
}

void recordScore(const Player *p)
{
    (void)p;
}

void showLeaderboard(void)
{
}
