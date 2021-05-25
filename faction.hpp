#pragma once

#include <stdint.h>
#include "colour.hpp"
#include "vla.hpp"
#include "historical_figure.hpp"

struct faction {
    uint32_t id;
    hsv colour = hsv(0,0,0);
    float money = 0;
    vla<int> owned_regions = vla<int>();
    int capital;
    char *name;
    float prev_income = 0;
    float prev_upkeep = 0;
    historical_figure leader = historical_figure(0);

    faction(){};
    faction(uint32_t seed, int capital_idx);
};
