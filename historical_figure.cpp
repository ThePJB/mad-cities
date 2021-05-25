#include <stdio.h>
#include "historical_figure.hpp"
#include "rng.hpp"
#include "roman.hpp"
#include "vla.hpp"

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
    "Cain",
    "Abel",
};

auto hf_titles = vla<vla<const char*>>(NUM_PERSONALITIES);
void hf_titles_init() {
    auto bad_general_titles = vla<const char*>();
    bad_general_titles.push("Feeble");
    bad_general_titles.push("Unlucky");
    bad_general_titles.push("Cowardly");
    bad_general_titles.push("Impotent");
    bad_general_titles.push("Meek");
    bad_general_titles.push("Unremarkable");
    
    auto good_general_titles = vla<const char*>();
    good_general_titles.push("Conqueror");
    good_general_titles.push("Great");
    good_general_titles.push("Terrible");
    good_general_titles.push("Mighty");
    good_general_titles.push("General");
    good_general_titles.push("Brave");
    good_general_titles.push("Fierce");

    auto bad_administrator_titles = vla<const char*>();
    bad_administrator_titles.push("Brash");
    bad_administrator_titles.push("Slow");
    bad_administrator_titles.push("Ignoramus");
    bad_administrator_titles.push("Unintelligent");
    bad_administrator_titles.push("Foolhardy");
    bad_administrator_titles.push("Dull");
    bad_administrator_titles.push("Unwise");

    auto good_administrator_titles = vla<const char*>();
    good_administrator_titles.push("Shrewd");
    good_administrator_titles.push("Administrator");
    good_administrator_titles.push("Wise");
    good_administrator_titles.push("Reformer");
    good_administrator_titles.push("Noble");

    hf_titles.items[P_BAD_GENERAL] = bad_general_titles;
    hf_titles.items[P_GOOD_GENERAL] = good_general_titles;
    hf_titles.items[P_GOOD_ADMINISTRATOR] = good_administrator_titles;
    hf_titles.items[P_BAD_ADMINISTRATOR] = bad_administrator_titles;
}

historical_figure::historical_figure(uint32_t rng) {
    dynasty_number = 0;
    first_name = hash_intn(rng, 0, len(hf_first_name));
    personality = (hf_personality)hash_intn(rng+435236, 0, NUM_PERSONALITIES);
    last_name = hash_intn(rng+345347, 0, hf_titles.get(personality)->length);
}

historical_figure::historical_figure(uint32_t rng, historical_figure parent) {
    dynasty_number = parent.dynasty_number + 1;
    first_name = parent.first_name;
    personality = (hf_personality)hash_intn(rng+435236, 0, NUM_PERSONALITIES);
    last_name = hash_intn(rng+345347, 0, hf_titles.get(personality)->length);
}

void historical_figure::print_name() const {
    if (dynasty_number == 0) {
        printf("%s the %s", hf_first_name[first_name], *hf_titles.get(personality)->get(last_name));
    } else {
        char buf[16];
        int2roman(dynasty_number + 1, buf);
        printf("%s %s the %s", hf_first_name[first_name], buf, *hf_titles.get(personality)->get(last_name));
    }
}