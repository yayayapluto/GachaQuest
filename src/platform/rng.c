#include "rng.h"

static unsigned long long rngState = 0x9E3779B97F4A7C15ULL;

void plat_srand(unsigned seed)
{
    rngState = ((unsigned long long)seed << 32) ^ (unsigned long long)seed;
    rngState ^= 0xD1B54A32D192ED03ULL;
    if (rngState == 0ULL) rngState = 1ULL;
    (void)plat_rand();
}

unsigned plat_rand(void)
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 7;
    rngState ^= rngState << 17;
    rngState ^= 0x9E3779B97F4A7C15ULL;
    return (unsigned)(rngState >> 32);
}

unsigned long long plat_rng_state(void)
{
    return rngState;
}

void plat_rng_set(unsigned long long state)
{
    rngState = (state == 0ULL) ? 1ULL : state;
}
