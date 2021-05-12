#include <stdint.h>
#include "point.hpp"

int hash_intn(uint32_t seed, int min, int max);
float hash_floatn(uint32_t seed, float min, float max);
uint32_t hash(uint32_t seed);
float hash_noise2(point v, int seed);
float hash_fbm2_4(point v, int seed);

uint32_t just_float_bytes(float f);