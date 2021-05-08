#include "world.hpp"
#include <string.h>

const int faction_gaia = 99999989;

const auto upkeep_coefficient = 1.5;

#define len(X) (sizeof(X)/sizeof(X[0]))

const char *first_name[] = {
    "Chungus",
    "Black Dog",
    "Dark Horse",
    "Hungry Boys",
    "Warmonger",
    "New Zanzibar",
    "Naughty",
    "Good",
    "Nice",
    "Unstoppable",
    "Democratic Peoples",
    "Golden",
    "Chosen",
    "Holy",
    "Blessed",
    "Sacred",
    "Rich Gang",
    "Banana",
    "Badman",
    "Zealous",
    "Cheeky",
    "Rising sun",
    "Morning star",
    "Moonlight",
    "Moonshine",
    "Evil",
    "Gangnam",
    "Midnight",
    "Omen",
    "Wrath",

};

const char *second_name[] = {
    "Confederacy",
    "Brotherhood",
    "Collective",
    "Trading Company",
    "Republic",
    "Kingdom",
    "Empire",
    "Alliance",
    "Dominion",
    "Principality",
    "State",
    "Barony",
    "Nation",
};

#define M_PHI 1.618033988749895

faction::faction(uint32_t seed, int capital_idx) {
    static int n_faction = 0;

    capital = capital_idx;
    id = hash(capital_idx);
    owned_regions.push(capital_idx);
    
    const auto start_angle = hash_floatn(seed + 32445324, 0, 360);
    n_faction++;
    const auto angle = fmod(start_angle + 2*M_PI*M_PHI*n_faction, 360); // like how plants work
    colour = hsv(angle, 0.5, 0.9);
    
    const auto first = hash_intn(seed + id, 0, len(first_name));
    const auto second = hash_intn(seed + id + 34253, 0, len(second_name));

    name = (char*)calloc(2 + strlen(first_name[first]) + strlen(second_name[second]), sizeof(char));
    sprintf(name, "%s %s", first_name[first], second_name[second]);

    n_faction++;
}

biome get_biome(float x, float y, uint32_t seed) {
    const auto r = point(x, y).dist(point(0.5, 0.5));
    const auto height = (1.0 - 2*r) * 0.0 + 1.0 * hash_fbm2_4(3 * point(x,y), 98944 + seed);

    if (height < 0.3) {
        return BIOME_OCEAN;
    } else if (height < 0.5) {
        return BIOME_PLAINS;
    } else if (height < 0.7) {
        return BIOME_MOUNTAIN;
    } else {
        return BIOME_BIG_MOUNTAIN;
    }
}

region::region(int idx, uint32_t faction, float x, float y, uint32_t seed) {
    const auto b = get_biome(x, y, seed);
    faction_key = faction;
    voronoi_idx = idx;
    m_biome = b;
}

float road_cost(point p1, point p2) {
    return 15*p1.dist(p2);
}

// ok the way regions are handled rn is retarded
// it should just take faction key and not be smart
// vla .get method probs a good idea
// we are going to have a chance of starting as a faction, otherwise its owner is gaia


world::world(uint32_t seed, int n_points, float p_faction) {
    auto points = vla<point>();
    for (int i = 0; i < n_points; i++) {
        seed = hash(seed);
        float x = hash_floatn(seed, 0, 1);
        float y = hash_floatn(seed * 323098, 0, 1);
        const point point = {x, y};
        points.push(point);
    }
    v = voronoi(points);
    v.relax();

    // populate region vla
    for (int i = 0; i < v.num_sites(); i++) {
        const auto site = v.get_site(i);
        const jcv_graphedge* edge = site->edges;

        auto new_region = region(i, faction_gaia, site->p.x, site->p.y, seed);
        if (new_region.m_biome != BIOME_OCEAN && 
                new_region.m_biome != BIOME_BIG_MOUNTAIN &&
                hash_floatn(seed + 324234 + i, 0, 1) < 0.1) {
            auto new_faction = faction(seed, i);
            new_region.faction_key = new_faction.id;
            printf("creating %s\n", new_faction.name);
            factions.set(new_faction.id, new_faction);
        }
        regions.push(new_region);
    }

    points.destroy();
}

void world::destroy() {
    regions.destroy();

    factions.iter_begin();
    while (auto f = factions.iter_next()) {
        free(f->item.name);
    }

    factions.destroy();
    roads.destroy();
    v.destroy();
}

float world::capture_price(int idx) {
    const auto region = regions.get(idx);
    auto price = 10;
    if (region->m_biome == BIOME_MOUNTAIN) {
        price = 30;
    }
    if (region->faction_key == faction_gaia) {
        price *= 0.5;
    }

    // check connection to neighbours
    bool any_allies = false;
    for (int i = 0; i < v.get_num_neighbours(idx); i++) {
        const auto neighbour = v.get_neighbour_idx(idx, i);
        if (regions.get(neighbour)->faction_key == region->faction_key) {
            any_allies = true;
        }
    }

    if (!any_allies) {
        price *= 0.1;
    }

    return price;
}

void world::draw(render_context *rc, uint32_t highlight_faction) {
    auto keys = SDL_GetKeyboardState(NULL);
    auto income_view = false;
    if (keys[SDL_SCANCODE_I]) {
        income_view = true;
    }


    for (int i = 0; i < v.num_sites(); i++) {
        const auto site = v.get_site(i);
        const auto faction_key = regions.items[site->index].faction_key;
        
        faction *f_ptr = NULL;
        if (faction_key != faction_gaia) {
            f_ptr = factions.get(faction_key);
        };

        rgb colour = {1, 1, 1}; 
        if (regions.items[i].m_biome == BIOME_PLAINS) {
            colour = f_ptr ? hsv2rgb(f_ptr->colour) : rgb(0.3, 0.7, 0.3);
        } else if (regions.items[i].m_biome == BIOME_MOUNTAIN) {
            if (faction_key == faction_gaia) {
                colour = {0.3, 0.5, 0.3};
            } else {
                auto mut_hsv = f_ptr->colour; 
                mut_hsv.s *= 0.6;
                colour = hsv2rgb(mut_hsv);
            }
        } else if (regions.items[i].m_biome == BIOME_OCEAN) {
            colour = {0.1, 0.4, 0.7};
        } else if (regions.items[i].m_biome == BIOME_BIG_MOUNTAIN) {
            colour = {0.4, 0.4, 0.4};
        } else {
            colour = {1, 0, 1};
        }

        if (income_view && regions.items[i].m_biome != BIOME_OCEAN && regions.items[i].m_biome != BIOME_BIG_MOUNTAIN) {
            auto money_rate = hash_fbm2_4(4 * site->p, 2312314);
            colour = {1-money_rate, money_rate, 0.0f};
        }

        auto e = site->edges;
        while (e) {
            rc->draw_triangle(colour, site->p, e->pos[0], e->pos[1]);
            e = e->next;
        }

    }
    // draw roads
    for (int i = 0; i < roads.length; i++) {
        const auto r = roads.items[i];
        const auto start = v.get_site(r.start_site)->p;
        const auto end = v.get_site(r.end_site)->p;
        rc->draw_line(rgb(0.7, 0.7, 0.4), start, end, 3);
    }

    // draw edges
    const jcv_edge* edge = jcv_diagram_get_edges(&v.diagram);
    while(edge) {
        const auto f1 = regions.items[edge->sites[0]->index].faction_key;
        if (!edge->sites[1]) {
            edge = edge->next;
            continue;
        };
        const auto f2 = regions.items[edge->sites[1]->index].faction_key;
        if (f1 != f2) {
            if (highlight_faction == f1 || highlight_faction == f2) {
                rc->draw_line(rgb(1,1,1), edge->pos[0], edge->pos[1], 2);
            }
            rc->draw_line(rgb(1,1,1), edge->pos[0], edge->pos[1], 1);
        }
        edge = edge->next;
    }

    factions.iter_begin();
    while (auto f = factions.iter_next()) {
        auto capital = f->item.capital;
        auto capital_faction = regions.items[capital].faction_key;
        if (capital_faction == f->key) {
            rc->draw_circle(rgb(0,0,0), v.get_site(capital)->p, 5);
        } else {
            // faction with no capital lol
            rc->draw_circle(rgb(0,0,0), v.get_site(capital)->p, 3);
        }
    }
}

void world::update(double dt) {
    for (int i = 0; i < v.num_sites(); i++) {
        const auto faction_key = regions.items[i].faction_key;
        if (faction_key == faction_gaia) continue;
        faction *faction_ptr = factions.get(faction_key);

        // first make money
        auto money_rate = hash_fbm2_4(4 * v.get_site(i)->p, 2312314); // wow some retarded stuff going on with capital site
        faction_ptr->money += money_rate*dt;
        const auto capital_site = v.get_site(faction_ptr->capital);
        const auto capital_pt = point(capital_site->p.x, capital_site->p.y);
        const auto this_site = v.get_site(i);
        const auto this_pt = point(this_site->p.x, this_site->p.y);
        const auto upkeep = upkeep_coefficient * this_pt.dist(capital_pt);
        faction_ptr->money -= upkeep*dt;
    
        // pick a random neighbour
        rng = hash(rng);
        const auto neighbour_idx = v.get_neighbour_idx(i, hash_intn(rng, 0, v.get_num_neighbours(i)));
        
        // then consider just buying them
        const auto other_region = &regions.items[neighbour_idx];
        const auto other_faction = other_region->faction_key;
        if (other_region->m_biome == BIOME_BIG_MOUNTAIN ||
            other_region->m_biome == BIOME_OCEAN) continue;

        if (other_faction != faction_key) {
            // price = ?? maybe amount of money
            //const auto price = 2 + factions.items[other_faction].money;
            const auto price = capture_price(neighbour_idx);
            rng = hash(rng);
            if ((hash_floatn(rng, 0, 1) < 0.01) && price < faction_ptr->money) {
                //printf("faction %d buying region %d from %d\n", faction_key, neighbour_idx, other_faction);
                faction_ptr->money -= price;
                regions.items[neighbour_idx].faction_key = faction_key;

                if (other_faction != faction_gaia) {
                    factions.get(other_faction)->owned_regions.remove_item(neighbour_idx);
                    if (factions.get(other_faction)->owned_regions.length == 0) {
                        printf("faction %s eliminated by %s\n", factions.get(other_faction)->name, faction_ptr->name);
                    }
                }
                faction_ptr->owned_regions.push(neighbour_idx);
            }
        }

/*
        // consider building road to neighbours
        // also check not preexisting road. or could increase road goodness
        const auto i1 = i;
        const auto i2 = neighbour_idx;
        const auto p1 = point(v.get_site(i)->p.x, v.get_site(i)->p.y);
        const auto p2 = point(v.get_site(i2)->p.x, v.get_site(i2)->p.y);
        
        const auto cost = road_cost(p1, p2);
        if (cost < faction_ptr->money && !roads.contains(road_segment(i1, i2)) && !roads.contains(road_segment(i1, i2))) {
            // actually make road
            faction_ptr->money -= cost;
            roads.push(road_segment(i, i2));
        }
*/
    }
}
