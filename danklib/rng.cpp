#include "rng.hpp"

#define U32_MAX 0xFFFFFFFF

// could reinterpret cast to seed with float
uint32_t hash(uint32_t seed) {
    const unsigned int BIT_NOISE1 = 0xB5297A4D;
    const unsigned int BIT_NOISE2 = 0x68E31DA4;
    const unsigned int BIT_NOISE3 = 0x1B56C4E9;

    unsigned int mangled = seed;
    mangled *= BIT_NOISE1;
    mangled ^= (mangled >> 8);
    mangled += BIT_NOISE2;
    mangled ^= (mangled << 8);
    mangled *= BIT_NOISE3;
    mangled ^= (mangled >> 8);
    return mangled;
}

float hash_floatn(uint32_t seed, float min, float max) {
    return ((double)hash(seed)/U32_MAX) * (max - min) + min;
}

int hash_intn(uint32_t seed, int min, int max) {
    return (hash(seed)%(max - min)) + min;
}
