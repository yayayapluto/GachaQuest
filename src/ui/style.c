#include "config.h"
#include <string.h>

#include "style.h"
#include "caps.h"

static const char *fgTable[ST_COUNT];
static const char *bgTable[ST_COUNT];
static int styleReady;

static void fillMono(void)
{
    int i;
    for (i = 0; i < ST_COUNT; ++i) {
        fgTable[i] = "";
        bgTable[i] = "";
    }
}

static void fillColor(int rich)
{
    int i;
    fillMono();
    fgTable[ST_TITLE] = "\033[1;36m";
    fgTable[ST_ACCENT] = "\033[36m";
    fgTable[ST_BORDER] = "\033[90m";
    fgTable[ST_DIM] = "\033[2m";
    fgTable[ST_GOLD] = "\033[33m";
    fgTable[ST_DANGER] = "\033[31m";
    fgTable[ST_SUCCESS] = "\033[32m";
    fgTable[ST_RARITY_COMMON] = "\033[37m";
    fgTable[ST_RARITY_RARE] = "\033[36m";
    fgTable[ST_RARITY_EPIC] = "\033[35m";
    fgTable[ST_RARITY_LEGENDARY] = "\033[33m";
    fgTable[ST_BRONZE] = "\033[94m";
    bgTable[ST_SELECTED] = "\033[7m";
    fgTable[ST_FOG1] = "\033[90m";
    fgTable[ST_FOG2] = "\033[2;37m";
    fgTable[ST_FOG3] = "\033[1;30m";

    if (rich) {
        fgTable[ST_BORDER] = "\033[38;5;244m";
        fgTable[ST_GOLD] = "\033[38;5;214m";
        fgTable[ST_SUCCESS] = "\033[38;5;46m";
        bgTable[ST_SELECTED] = "\033[48;5;24m";
        for (i = 0; i < 11; ++i) {
            fgTable[ST_HP_GRAD_BASE + i] = (i >= 8) ? "\033[38;5;46m" :
                (i >= 5) ? "\033[38;5;214m" : "\033[38;5;196m";
        }
        for (i = 0; i < 8; ++i) fgTable[ST_BIOME_BASE + i] = "\033[38;5;70m";
        for (i = 0; i < BIOME_FOG_TIERS * BIOME_COUNT; ++i) {
            fgTable[ST_BIOME_FOG_BASE + i] = "\033[38;5;240m";
        }
    } else {
        for (i = 0; i < 11; ++i) {
            fgTable[ST_HP_GRAD_BASE + i] = (i >= 8) ? "\033[32m" :
                (i >= 5) ? "\033[33m" : "\033[31m";
        }
        for (i = 0; i < 8; ++i) fgTable[ST_BIOME_BASE + i] = "\033[32m";
        for (i = 0; i < BIOME_FOG_TIERS * BIOME_COUNT; ++i) {
            fgTable[ST_BIOME_FOG_BASE + i] = "\033[90m";
        }
    }
}

void style_init(void)
{
    ColorMode mode = caps_color();
    if (mode == COLOR_MONO) fillMono();
    else fillColor(mode == COLOR_256 || mode == COLOR_TRUE);
    styleReady = 1;
}

Style style_rarity(int rarity)
{
    switch (rarity) {
        case RARITY_RARE: return ST_RARITY_RARE;
        case RARITY_EPIC: return ST_RARITY_EPIC;
        case RARITY_LEGENDARY: return ST_RARITY_LEGENDARY;
        default: return ST_RARITY_COMMON;
    }
}

Style style_hp(int hp, int maxHp)
{
    int pct;
    if (maxHp <= 0) return ST_HP_GRAD(0);
    pct = (hp * 100) / maxHp;
    if (pct > 100) pct = 100;
    if (pct < 0) pct = 0;
    return ST_HP_GRAD((pct * 10) / 100);
}

const char *style_fg(Style s)
{
    if (!styleReady) style_init();
    if (s < 0 || s >= ST_COUNT) return "";
    return fgTable[s];
}

const char *style_bg(Style s)
{
    if (!styleReady) style_init();
    if (s < 0 || s >= ST_COUNT) return "";
    return bgTable[s];
}
