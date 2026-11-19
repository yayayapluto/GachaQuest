#include <stdio.h>
#include <string.h>
#include "config.h"
#include "inventory.h"
#include "player.h"

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s line %d\n", #cond, __LINE__); return 1; } } while (0)

static void addItem(Player *p, int id, const char *name, char type, int rarity, int power)
{
    p->inventory[p->itemCount].id = id;
    strncpy(p->inventory[p->itemCount].name, name, MAX_NAME - 1);
    p->inventory[p->itemCount].name[MAX_NAME - 1] = '\0';
    p->inventory[p->itemCount].type = type;
    p->inventory[p->itemCount].rarity = rarity;
    p->inventory[p->itemCount].power = power;
    p->itemCount++;
}

int main(void)
{
    Player p;
    Player kosong;

    initPlayer(&p, "Cari");
    addItem(&p, 1, "Alpha", 'S', 1, 5);
    addItem(&p, 2, "Bravo", 'S', 2, 9);
    addItem(&p, 3, "Charlie", 'S', 1, 7);

    /* Selection Sort: power descending -> Bravo 9, Charlie 7, Alpha 5 */
    sortInventory(&p, 2);
    CHECK(p.inventory[0].id == 2);
    CHECK(p.inventory[1].id == 3);
    CHECK(p.inventory[2].id == 1);

    /* Name A-Z: Alpha, Bravo, Charlie */
    sortInventory(&p, 3);
    CHECK(strcmp(p.inventory[0].name, "Alpha") == 0);
    CHECK(strcmp(p.inventory[1].name, "Bravo") == 0);
    CHECK(strcmp(p.inventory[2].name, "Charlie") == 0);

    /* Binary Search on the sorted names */
    CHECK(searchItemByNameBinary(&p, "bravo") == 1);
    CHECK(searchItemByNameBinary(&p, "ALPHA") == 0);
    CHECK(searchItemByNameBinary(&p, "Charlie") == 2);
    CHECK(searchItemByNameBinary(&p, "zzz") == -1);

    /* Empty inventory never finds anything */
    initPlayer(&kosong, "Kosong");
    CHECK(searchItemByNameBinary(&kosong, "Alpha") == -1);
    return 0;
}
