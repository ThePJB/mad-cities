#pragma once

#include <stdint.h>

enum hf_personality {
    P_GOOD_GENERAL,
    P_BAD_GENERAL,
    P_BAD_ADMINISTRATOR,
    P_GOOD_ADMINISTRATOR,
    NUM_PERSONALITIES
};

struct historical_figure {
    hf_personality personality;
    int first_name = 0;
    int last_name = 0;
    int dynasty_number = 0;

    historical_figure(uint32_t rng);
    historical_figure(uint32_t rng, historical_figure parent);

    void print_name() const;
};