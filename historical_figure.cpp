#include <stdio.h>
#include "historical_figure.hpp"
#include "rng.hpp"
#include "roman.hpp"

#define len(X) (sizeof(X)/sizeof(X[0]))

const char *hf_first_name[] = {
    "Richard",
    "Bob",
    "Alexander",
    "Philip",
    "Ken Bruce",
    "Agronak",
    "James",
    "Benjamin",
    "Luke",
    "Aidan",
    "Patrick",
    "Vlork",
    "Justinian",
    "Kieren",
    "Henry",
};

const char *hf_last_name[] = {
    "the Mad",
    "the Terrible",
    "the Foolish",
    "the Great",
    "the Diplomat",
    "the Wise",
    "the Mighty",
    "the Conqueror",
    "the Impotent",
};

historical_figure::historical_figure(uint32_t rng) {
    dynasty_number = 0;
    first_name = hash_intn(rng, 0, len(hf_first_name));
    last_name = hash_intn(rng+345347, 0, len(hf_last_name));
    personality = (hf_personality)hash_intn(rng+435236, 0, NUM_PERSONALITIES);
}

historical_figure::historical_figure(uint32_t rng, historical_figure parent) {
    dynasty_number = parent.dynasty_number + 1;
    first_name = parent.first_name;
    last_name = hash_intn(rng+345347, 0, len(hf_last_name));
    personality = (hf_personality)hash_intn(rng+435236, 0, NUM_PERSONALITIES);
}

void historical_figure::print_name() const {
    if (dynasty_number == 0) {
        printf("%s %s", hf_first_name[first_name], hf_last_name[last_name]);
    } else {
        char buf[16];
        int2roman(dynasty_number + 1, buf);
        printf("%s %s", hf_first_name[first_name], buf);
    }
}