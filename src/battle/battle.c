#include <stdio.h>
#include "battle.h"
#include "player.h"
#include "ui.h"
#include "render.h"
#include "rng.h"

static int runAbort;
static int runEnded;

static const char *const aksiBattle[] = { "Serang", "Bertahan", "Potion", "Kabur" };
static const int nomorBattle[] = { 1, 2, 3, 4 };
static const char *const aksiLanjut[] = { "Lanjut" };
static const int nomorLanjut[] = { 1 };

typedef struct {
    const Player *p;
    const Monster *e;
    int enemyHp;
    int selesai;
} BattleView;

static void gambarBattle(void *ctx, int sel)
{
    const BattleView *v = ctx;

    ui_screen_begin("Battle", v->e->name);
    ui_panel(UI_CONTENT_ROW, 2, 46, 6, "Pertempuran");
    render_text(UI_CONTENT_ROW + 1, 4, ST_TEXT, "%s  Lv %d  ATK %d  DEF %d",
                v->p->name, v->p->level, totalAtk(v->p), totalDef(v->p));
    ui_hp_bar_n(UI_CONTENT_ROW + 2, 4, 40, v->p->hp, v->p->maxHp);
    render_text(UI_CONTENT_ROW + 3, 4, ST_DANGER, "%s", v->e->name);
    ui_hp_bar_n(UI_CONTENT_ROW + 4, 4, 40, v->enemyHp, v->e->hp);
    ui_log_panel(UI_CONTENT_ROW + 6, 2, 46, 7, "Log", 5);
    if (v->selesai)
        ui_menu_h(UI_CONTENT_ROW + 13, 2, aksiLanjut, nomorLanjut, 1, sel);
    else
        ui_menu_h(UI_CONTENT_ROW + 13, 2, aksiBattle, nomorBattle, 4, sel);
    ui_screen_end(NULL);
}

/* Satu-satunya tempat yang memutuskan crit; semua serangan lewat sini. */
static int rollHit(int atk, int def, int *crit)
{
    int dmg = calculateDamage(atk, def);

    *crit = (int)(plat_rand() % 100u) < CRIT_CHANCE;
    if (*crit) dmg *= CRIT_MULTIPLIER;
    return dmg;
}

static void logHit(const char *actor, const char *target, int dmg, int crit,
                   Style actorSt, Style targetSt)
{
    char val[48];

    if (crit) snprintf(val, sizeof val, "%d damage (CRIT!)", dmg);
    else snprintf(val, sizeof val, "%d damage", dmg);
    ui_log_event(actor, "menyerang", target, val, actorSt, targetSt, ST_GOLD);
}

static void runBattle(Player *p, const Monster *e, int enemyHp)
{
    BattleView v;
    int pilih, defending, dmg, crit, heal, levelAwal;

    if (enemyHp > e->hp) enemyHp = e->hp;
    if (enemyHp < 1) enemyHp = 1;
    v.p = p;
    v.e = e;
    v.selesai = 0;
    ui_log_clear();
    ui_log_style(ST_DANGER, "%s muncul!", e->name);

    for (;;) {
        v.enemyHp = enemyHp;
        pilih = ui_menu_pick(aksiBattle, nomorBattle, 4, 1, 4, gambarBattle, &v);
        if (pilih < 0) return;
        if (pilih == 0) continue;

        defending = 0;
        if (pilih == 1) {
            dmg = rollHit(totalAtk(p), e->def, &crit);
            enemyHp -= dmg;
            if (enemyHp < 0) enemyHp = 0;
            logHit(p->name, e->name, dmg, crit, ST_ACCENT, ST_DANGER);
        } else if (pilih == 2) {
            defending = 1;
            ui_log_style(ST_ACCENT, "%s bertahan.", p->name);
        } else if (pilih == 3) {
            if (p->potion <= 0) {
                ui_log_style(ST_DANGER, "Potion habis!");
                continue;
            }
            p->potion--;
            heal = p->maxHp - p->hp;
            if (heal > POTION_HEAL) heal = POTION_HEAL;
            p->hp += heal;
            ui_log_style(ST_SUCCESS, "%s memakai potion, HP +%d.", p->name, heal);
        } else {
            if (plat_rand() % 100u < 60u) {
                ui_log_style(ST_TEXT, "%s berhasil kabur.", p->name);
                return;
            }
            ui_log_style(ST_DANGER, "Gagal kabur!");
        }

        if (enemyHp > 0) {
            dmg = rollHit(e->atk, totalDef(p), &crit);
            if (defending) {
                dmg /= 2;
                if (dmg < 1) dmg = 1;
            }
            p->hp -= dmg;
            if (p->hp < 0) p->hp = 0;
            logHit(e->name, p->name, dmg, crit, ST_DANGER, ST_ACCENT);
        }

        if (enemyHp <= 0) {
            levelAwal = p->level;
            addExp(p, e->exp);
            p->gold += e->gold;
            ui_log_style(ST_GOLD, "Hadiah: %d EXP, %d gold", e->exp, e->gold);
            if (p->level > levelAwal)
                ui_log_style(ST_SUCCESS, "Naik ke level %d!", p->level);
            break;
        }
        if (p->hp <= 0) {
            ui_log_style(ST_DANGER, "Kamu dikalahkan!");
            runEnded = 1;
            break;
        }
    }

    v.enemyHp = enemyHp;
    v.selesai = 1;
    ui_menu_pick(aksiLanjut, nomorLanjut, 1, 1, 1, gambarBattle, &v);
}

void startBattle(Player *p, const Monster m[], int count)
{
    int idx[MAX_MONSTER];
    int n = 0, i, pick;

    if (p == NULL || m == NULL || count <= 0) return;
    for (i = 0; i < count && i < MAX_MONSTER; ++i)
        if (m[i].minLevel <= p->level) idx[n++] = i;
    if (n == 0) idx[n++] = 0;
    pick = idx[(int)(plat_rand() % (unsigned)n)];
    runBattle(p, &m[pick], m[pick].hp);
}

void startBattleAgainst(Player *p, const Monster *enemy)
{
    if (p == NULL || enemy == NULL) return;
    runBattle(p, enemy, enemy->hp);
}

void resumeBattle(Player *p, Monster enemy, int enemyHp)
{
    if (p == NULL) return;
    runBattle(p, &enemy, enemyHp);
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