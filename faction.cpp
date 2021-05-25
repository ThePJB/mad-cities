#include "faction.hpp"
#include "rng.hpp"

#define M_PHI 0.618033988749895
#define len(X) (sizeof(X)/sizeof(X[0]))


const char *name_first_part[] = {
    "White",
    "Black",
    "Grey",
    "Red",
    "Dark",
    "Light",
    "Fire",
    "Ice",
    "High",
    "Hail",
    "Dune",
    "Mist",
    "Sleet",
    "Fleet",
    "Rain",
    "Shine",
    "Tail",
    "Long",
    "Night",
    "Day",
    "Known",
    "Watch",
    "Flat",
    "Slow",
    "Fast",
    "Old",
    "New",
    "Cut",
    "Sun",
    "Moon",
    "Under",
    "Over",
    "Cash",
    "Great",
    "Hit",
    "Hard",
    "Turn",
};

const char *name_second_part[] = {
    "blade",
    "bite",
    "bat",
    "bush",
    "dog",
    "fox",
    "horse",
    "mouse",
    "rat",
    "foot",
    "cat",
    "fish",
    "goose",
    "goat",
    "stance",
    "box",
    "glow",
    "fall",
    "dawn",
    "light",
    "shock",
    "piece",
    "list",
    "loss",
    "love",
    "gore",
    "death",
    "hand",
    "chest",
    "boat",
    "gate",
    "skate",
    "strait",
    "land",
    "wood",
    "dance",
    "bloat",
    "helm",
    "heart",
    "mind",
    "body",
    "soul",
    "key",
    "lock",
    "house",
};

const char *state_type[] = {
    "Republic",
    "Kingdom",
    "Empire",
    "Alliance",
};

faction::faction(uint32_t seed, int capital_idx) {
    static int n_faction = 0;

    capital = capital_idx;
    id = hash(seed + capital_idx);
    
    const auto start_angle = hash_floatn(seed + 32445324, 0, 360);
    n_faction++;
    const auto angle = fmod(start_angle + 360*M_PHI*n_faction, 360); // like how plants work
    colour = hsv(angle, 0.5, 0.9);
    
    const auto first = hash_intn(seed + id, 0, len(name_first_part));
    const auto second = hash_intn(seed + id + 34253, 0, len(name_second_part));
    const auto state_type_idx = hash_intn(seed + id + 234235, 0, len(state_type));

    name = (char*)calloc(2 + strlen(name_first_part[first]) + strlen(name_second_part[second]) + strlen(state_type[state_type_idx]), sizeof(char));
    sprintf(name, "%s%s %s", name_first_part[first], name_second_part[second], state_type[state_type_idx]);

    leader = historical_figure(seed + n_faction + 435324);

    n_faction++;
}