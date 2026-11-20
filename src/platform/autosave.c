#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "autosave.h"
#include "atomicfile.h"
#include "rng.h"

#define AS_VERSION 1
#define AS_HEADER 16
#define AS_PATH "data/autosave.dat"
#define AS_BACKUP "data/autosave.bak"

static AutoSnapshot current;
static int dirty;
static int started;
static int errorPending;
static char errorMessage[160];

static uint32_t crcTable[256];
static int crcReady;

static void buildCrc(void)
{
    unsigned int i, j;
    uint32_t c;
    if (crcReady) return;
    for (i = 0; i < 256; ++i) {
        c = i;
        for (j = 0; j < 8; ++j) c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        crcTable[i] = c;
    }
    crcReady = 1;
}

static uint32_t crc32Bytes(const unsigned char *data, size_t len)
{
    uint32_t crc = 0xFFFFFFFFu;
    size_t i;
    buildCrc();
    for (i = 0; i < len; ++i) crc = crcTable[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

static void setError(const char *msg)
{
    if (errorMessage[0] == '\0') snprintf(errorMessage, sizeof(errorMessage), "%s", msg);
    errorPending = 1;
}

static int writeSnapshot(void)
{
    unsigned char buf[AS_HEADER + sizeof(AutoSnapshot)];
    uint32_t crc;
    int version = AS_VERSION;
    int len = (int)sizeof(AutoSnapshot);

    memset(buf, 0, sizeof(buf));
    buf[0] = 'G'; buf[1] = 'Q'; buf[2] = 'A'; buf[3] = 'S';
    memcpy(buf + 4, &version, sizeof(version));
    memcpy(buf + 8, &len, sizeof(len));
    crc = crc32Bytes((const unsigned char *)&current, sizeof(current));
    memcpy(buf + 12, &crc, sizeof(crc));
    memcpy(buf + AS_HEADER, &current, sizeof(current));

    if (!file_write_atomic(AS_PATH, buf, sizeof(buf))) {
        setError("Gagal menulis autosave");
        return 0;
    }
    dirty = 0;
    return 1;
}

static int readSnapshot(const char *path, AutoSnapshot *out)
{
    unsigned char buf[AS_HEADER + sizeof(AutoSnapshot)];
    FILE *f = fopen(path, "rb");
    size_t n;
    int version;
    int len;
    uint32_t crc;

    if (f == NULL) return 0;
    n = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    if (n != sizeof(buf)) return 0;
    if (buf[0] != 'G' || buf[1] != 'Q' || buf[2] != 'A' || buf[3] != 'S') return 0;
    memcpy(&version, buf + 4, sizeof(version));
    memcpy(&len, buf + 8, sizeof(len));
    memcpy(&crc, buf + 12, sizeof(crc));
    if (version != AS_VERSION || len != (int)sizeof(AutoSnapshot)) return 0;
    if (crc != crc32Bytes(buf + AS_HEADER, sizeof(AutoSnapshot))) return 0;
    memcpy(out, buf + AS_HEADER, sizeof(AutoSnapshot));
    return 1;
}

static void backupExisting(void)
{
    unsigned char buf[AS_HEADER + sizeof(AutoSnapshot)];
    FILE *f = fopen(AS_PATH, "rb");
    size_t n;
    if (f == NULL) return;
    n = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    if (n == sizeof(buf)) (void)file_write_atomic(AS_BACKUP, buf, n);
}

void autosave_start(void)
{
    memset(&current, 0, sizeof(current));
    dirty = 0;
    started = 1;
    errorPending = 0;
    errorMessage[0] = '\0';
}

void autosave_stop(void)
{
    if (!started) return;
    if (dirty) writeSnapshot();
    started = 0;
}

void autosave_publish(const Player *p, int screen,
                      const Monster *enemy, int enemyHp,
                      const DungeonState *dst, int steps, int hints,
                      int important)
{
    AutoSnapshot next;

    if (!started) autosave_start();
    if (p == NULL) return;
    memset(&next, 0, sizeof(next));
    next.player = *p;
    next.screen = screen;
    next.battleActive = (enemy != NULL);
    if (enemy != NULL) next.battleEnemy = *enemy;
    next.battleEnemyHp = enemyHp;
    next.dungeonActive = (dst != NULL);
    if (dst != NULL) next.dungeon = *dst;
    next.dungeonSteps = steps;
    next.dungeonHints = hints;
    next.rng = plat_rng_state();

    if (memcmp(&next, &current, sizeof(next)) == 0) return;
    backupExisting();
    current = next;
    dirty = 1;
    if (important) writeSnapshot();
}

int autosave_load(AutoSnapshot *out)
{
    if (out == NULL) return 0;
    if (readSnapshot(AS_PATH, out)) return 1;
    return readSnapshot(AS_BACKUP, out);
}

int autosave_poll_error(char *buf, int len)
{
    if (!errorPending || buf == NULL || len <= 0) return 0;
    snprintf(buf, (size_t)len, "%s", errorMessage);
    errorPending = 0;
    errorMessage[0] = '\0';
    return 1;
}
