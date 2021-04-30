#include <stdint.h>

int hash_intn(uint32_t seed, int min, int max);
float hash_floatn(uint32_t seed, float min, float max);
uint32_t hash(uint32_t seed);