#include "battle.h"
#include "rng.h"

static int runAbort;
static int runEnded;

void startBattle(Player *p, const Monster m[], int count)
{
    (void)p;
    (void)m;
    (void)count;
}

void startBattleAgainst(Player *p, const Monster *enemy)
{
    (void)p;
    (void)enemy;
}

void resumeBattle(Player *p, Monster enemy, int enemyHp)
{
    (void)p;
    (void)enemy;
    (void)enemyHp;
}

int calculateDamage(int atk, int def)
{
    int base = atk - def;

    if (base < 1) base = 1;
    return base + (int)(plat_rand() % 4u);
}

int battleRunAborted(void)
{
    return runAbort;
}

void battleClearRunAbort(void)
{
    runAbort = 0;
}

int battleRunEnded(void)
{
    return runEnded;
}

void battleClearRunEnded(void)
{
    runEnded = 0;
}

int battleGameOverScreen(Player *p)
{
    (void)p;
    return 0;
}
