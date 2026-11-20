#include <stddef.h>
#include "dungeon.h"
#include "util.h"
#include "rng.h"

unsigned int cellHash(unsigned int seed, int x, int y)
{
    unsigned int h = seed ^ (unsigned int)(x * 73856093) ^ (unsigned int)(y * 19349663);
    h ^= h >> 13;
    h *= 1274126177u;
    return h ^ (h >> 16);
}

char dungeonCell(unsigned int seed, int x, int y)
{
    (void)seed;
    (void)x;
    (void)y;
    return '#';
}

unsigned int randomWorldSeed(void)
{
    return plat_rand();
}

unsigned int promptWorldSeed(void)
{
    unsigned int seed = 0;
    (void)promptWorldSeedTry(&seed);
    return seed;
}

int promptWorldSeedTry(unsigned int *out)
{
    int seed = 0;
    if (out == NULL) return 0;
    seed = readInt("Seed dunia (kosong = acak): ", 0, 2000000000);
    *out = (unsigned int)seed;
    return 1;
}

int dfsReachable(char area[][AREA_MAX], Point from, Point to)
{
    (void)area;
    (void)from;
    (void)to;
    return 0;
}

int bfsShortestPath(char area[][AREA_MAX], Point from, Point to, Point path[])
{
    (void)area;
    (void)from;
    (void)to;
    (void)path;
    return 0;
}

void runDungeon(Player *p, const Item catalog[], int catalogCount,
                const Monster monsters[], int monsterCount)
{
    (void)p;
    (void)catalog;
    (void)catalogCount;
    (void)monsters;
    (void)monsterCount;
}

void resumeDungeon(Player *p, const DungeonState *saved, int steps, int hints,
                   const Item catalog[], int catalogCount,
                   const Monster monsters[], int monsterCount)
{
    (void)p;
    (void)saved;
    (void)steps;
    (void)hints;
    (void)catalog;
    (void)catalogCount;
    (void)monsters;
    (void)monsterCount;
}
