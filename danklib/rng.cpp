#include "rng.hpp"
#include "dankmath.hpp"
#include "point.hpp"
#include <stdio.h>

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

float hash_noise2(point v, int seed) {
    int x1 = dm_floor(v.x);
    int x2 = x1 + 1;
    int y1 = dm_floor(v.y);
    int y2 = y1 + 1;

    return dm_bilinear3(
        hash_floatn(x1 + y1 * 987423234 + seed * 342447, 0, 1), 
        hash_floatn(x2 + y1 * 987423234 + seed * 342447, 0, 1), 
        hash_floatn(x1 + y2 * 987423234 + seed * 342447, 0, 1), 
        hash_floatn(x2 + y2 * 987423234 + seed * 342447, 0, 1), 
        dm_frac(v.x), dm_frac(v.y)
    );
}

// _4 is how many hash_noise2 calls
float hash_fbm2_4(point v, int seed) {
    return (
        1.000 * hash_noise2(1.0f * v, seed + 234243) +
        0.500 * hash_noise2(2.0f * v, seed + 980732) +
        0.250 * hash_noise2(4.0f * v, seed + 895642) +
        0.125 * hash_noise2(8.0f * v, seed + 987443)
    ) / 1.875;
}

uint32_t just_float_bytes(float f) {
    return *reinterpret_cast<uint32_t *>(&f);
}

float bytes_to_float(uint32_t float_bytes) {
    return *reinterpret_cast<float *>(&float_bytes);
}

// for best results num_bits should be < mantissa length lmao
uint32_t round_float(uint32_t float_bytes, int num_bits) {
    // not sure about rounding up in binary, is that a thing?
    auto mask = 0xFF800000;
    for (int i = 0; i < num_bits; i++) {
        mask |= (1 << (23 - i));
    }
    printf("mask %d %X\n", num_bits, mask);
    return float_bytes & mask;
}