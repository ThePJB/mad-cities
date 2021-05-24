#include "world.hpp"
#include "coolmath.hpp"
#include <string.h>

const auto upkeep_coefficient = 2.5;

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
    "Proud",
    "Nasty",
    "Moonglow",
    "Muzz",
    "Dank",
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

#define M_PHI 0.618033988749895
// actually phi-1 but oh well

faction::faction(uint32_t seed, int capital_idx) {
    static int n_faction = 0;

    capital = capital_idx;
    id = hash(capital_idx);
    owned_regions.push(capital_idx);
    
    const auto start_angle = hash_floatn(seed + 32445324, 0, 360);
    n_faction++;
    const auto angle = fmod(start_angle + 360*M_PHI*n_faction, 360); // like how plants work
    colour = hsv(angle, 0.5, 0.9);
    
    const auto first = hash_intn(seed + id, 0, len(first_name));
    const auto second = hash_intn(seed + id + 34253, 0, len(second_name));

    name = (char*)calloc(2 + strlen(first_name[first]) + strlen(second_name[second]), sizeof(char));
    sprintf(name, "%s %s", first_name[first], second_name[second]);

    n_faction++;
}

region::region(int idx, uint32_t faction, point p, world *w) {
    const auto b = w->get_biome(p);
    faction_key = faction;
    voronoi_idx = idx;
    m_biome = b;
}

float road_cost(point p1, point p2) {
    return 15*p1.dist(p2);
}

int world::get_lowest_edge(int vert_idx) {
    int lowest_idx = -1;
    float lowest = 99999.0;
    for (int j = 0; j < v.verts.get(vert_idx)->edge_idx.size(); j++) {
        const auto edge_idx = v.verts.get(vert_idx)->edge_idx.contents[j];
        const auto other_vert = v.other_vert(edge_idx, vert_idx);
        const auto p = v.verts.get(other_vert)->site;
        const auto h = height(p);

        if (h < lowest) {
            lowest = h;
            lowest_idx = j;
        }
    }
    return v.verts.get(vert_idx)->edge_idx.contents[lowest_idx];
}

void world::make_rivers() {
    for (int i = 0; i < v.verts.length; i++) {
        auto current_vertex_idx = i;
        while (true) {

            // check if river has flowed into ocean
            auto edge_it = v.verts.get(current_vertex_idx)->edge_idx.iter();
            while (edge_it.has_next()) {
                const auto e_idx = edge_it.next();
                auto face_it = v.edges.get(e_idx)->face_idx.iter();
                while (face_it.has_next()) {
                    const auto f_idx = face_it.next();
                    const auto region = regions.get(f_idx);
                    if (region->m_biome == BIOME_OCEAN) {
                        goto done_river;
                    }
                }
            }

            const auto lowest_outgoing_edge_idx = get_lowest_edge(current_vertex_idx);
            rivers.items[lowest_outgoing_edge_idx] += rainfall(v.verts.get(i)->site);

            auto old_vertex_idx = current_vertex_idx;
            current_vertex_idx = v.edges.get(lowest_outgoing_edge_idx)->other_vertex(current_vertex_idx);

            if (old_vertex_idx == current_vertex_idx) {
                printf("wtf - vertex is own next vertex lol\n");
                goto done_river;
            }

            auto new_current_v = v.verts.get(current_vertex_idx);

            // check if river is at a local minima
            // well that would be also if the height of the place its going is greater than the height of here
            if (height(v.verts.get(current_vertex_idx)->site) > height(v.verts.get(old_vertex_idx)->site)) {
                printf("Local minima\n");
                // currently should not happen
                goto done_river;
            }
        }
        
        done_river:(void(0));
    }
}

world::world(uint32_t seed, int n_points, float p_faction) {
    this->seed = seed;
    this->rng = hash(seed + 324897);
    auto points = vla<point>();
    for (int i = 0; i < n_points; i++) {
        seed = hash(seed);
        float x = hash_floatn(seed, 0, 1);
        float y = hash_floatn(seed * 323098, 0, 1);
        const point point = {x, y};
        points.push(point);
    }
    v = voronoi(points);

    // populate region vla
    for (int i = 0; i < v.faces.length; i++) {
        auto f = &v.faces.items[i];

        auto new_region = region(i, faction_gaia, f->site, this);
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

    for (int i = 0; i < v.edges.length; i++) {
        rivers.push(0.0f);
    }

    points.destroy();

    make_rivers();
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

enum overlay_type {
    OL_NONE,
    OL_INCOME,
    OL_HEIGHT,
    OL_EDGE_DOWNHILL,
    OL_RAINFALL,
    OL_BIOMES,
    OL_DEFENCE,
    OL_FACTIONS,
};

void world::draw(render_context *rc, uint32_t highlight_faction) {
    auto keys = SDL_GetKeyboardState(NULL);
    auto overlay = OL_NONE;
    if (keys[SDL_SCANCODE_I]) {
        overlay = OL_INCOME;
    } else if (keys[SDL_SCANCODE_H]) {
        overlay = OL_HEIGHT;
    } else if (keys[SDL_SCANCODE_E]) {
        overlay = OL_EDGE_DOWNHILL;
    } else if (keys[SDL_SCANCODE_L]) {
        overlay = OL_RAINFALL;
    } else if (keys[SDL_SCANCODE_B]) {
        overlay = OL_BIOMES;
    } else if (keys[SDL_SCANCODE_F]) {
        overlay = OL_FACTIONS;
    } else if (keys[SDL_SCANCODE_D]) {
        overlay = OL_DEFENCE;
    }

    for (int i = 0; i < v.faces.length; i++) {
        auto f = &v.faces.items[i];
        const auto faction_key = regions.items[f->idx].faction_key;

        faction *f_ptr = NULL;
        if (faction_key != faction_gaia) {
            f_ptr = factions.get(faction_key);
        };

        // Draw faces
        const auto face_colour = [&](){
            const auto biome_colour = biome_prototypes[regions.get(i)->m_biome].colour;
            if (overlay == OL_BIOMES ) {
                return biome_colour;
            }
            
            if (overlay == OL_INCOME) {
                const auto money_rate = income(i);
                return rgb(1-money_rate, money_rate, 0.0f);
            }

            if (overlay == OL_HEIGHT) {
                const auto h = height(this->v.faces.get(i)->site);
                return rgb(h,h,h);
            }

            if (overlay == OL_RAINFALL) {
                const auto r = 5*rainfall(v.faces.get(i)->site);
                return rgb(cm_lerp(1, 0, r), cm_lerp(1, 0, r), cm_lerp(0, 1, r));
            }

            if (overlay == OL_DEFENCE) {
                const auto d = defensive_power_base(i);
                // 0 .. 1.2 clamp
                auto t = d / 1.2;
                if (t > 1) t = 1;

                return rgb(1-t, 0, t);
            }

            if (faction_key == faction_gaia) {
                return biome_colour;
            }

            const auto faction_colour = hsv2rgb(f_ptr->colour);
            if (overlay == OL_FACTIONS) {
                return faction_colour;
            }

            const auto blend_t = 0.7;
            return rgb(
                cm_lerp(biome_colour.r, faction_colour.r, blend_t),
                cm_lerp(biome_colour.g, faction_colour.g, blend_t),
                cm_lerp(biome_colour.b, faction_colour.b, blend_t)
            );
        }();

        v.fill_face(rc, i, rgb2hsv(face_colour));
    }
    // draw roads
    for (int i = 0; i < roads.length; i++) {
        const auto r = roads.items[i];
        const auto start = v.faces.items[r.start_site].site;
        const auto end = v.faces.items[r.end_site].site;
        rc->draw_line(rgb(0.7, 0.7, 0.4), start, end, 3);
    }

    // draw edges
    for (int i = 0; i < v.edges.length; i++) {
        auto edge = &v.edges.items[i];
        
        // edge is not guaranteed to have 2 things, skip these
        if (edge->face_idx.contents[1] == empty_sentinel) {
            continue;
        }
                
        const auto f1 = regions.items[edge->face_idx.contents[0]].faction_key;
        const auto f2 = regions.items[edge->face_idx.contents[1]].faction_key;
        const auto p1 = v.verts.items[edge->vert_idx.contents[0]].site;
        const auto p2 = v.verts.items[edge->vert_idx.contents[1]].site;
        
        if (f1 != f2) {

            if (highlight_faction != faction_gaia && (highlight_faction == f1 || highlight_faction == f2)) {
                rc->draw_line(rgb(1,1,1), p1, p2, 2);
            }
            rc->draw_line(rgb(1,1,1), p1, p2, 1);
        }

        const auto riverness = rivers.items[i];
        if (riverness > 0.5) {
            const auto width = sqrtf(4*riverness);
            rc->draw_line(rgb(0,0,1), p1, p2, width);
        }

        if (overlay == OL_EDGE_DOWNHILL) {
            if (height(p1) > height(p2)) {
                // draw a triangle pointing from p1 to p2
                rc->draw_arrow(hsv(0,0,1), p1, p2, 0.005);
            } else {
                rc->draw_arrow(hsv(0,0,1), p2, p1, 0.005);
            }   
        }
    }
    

    factions.iter_begin();
    while (auto f = factions.iter_next()) {
        auto capital = f->item.capital;
        auto capital_faction = regions.items[capital].faction_key;
        const auto city_site = v.faces.items[capital].site;
        if (capital_faction == f->key) {
            rc->draw_circle(rgb(0,0,0), city_site, 5);
        } else {
            // faction with no capital lol
            rc->draw_circle(rgb(0,0,0), city_site, 3);
        }
    }
}

void world::update(double dt) {
    time += dt;

    // zero faction income/upkeep
    factions.iter_begin();
    while (auto f = factions.iter_next()) {
        f->item.prev_income = 0;
        f->item.prev_upkeep = 0;
    }

    for (int i = 0; i < v.faces.length; i++) {
        auto f = &v.faces.items[i];
        const auto faction_key = regions.items[i].faction_key;
        if (faction_key == faction_gaia) continue;
        faction *faction_ptr = factions.get(faction_key);

        // first make money
        auto money_rate = income(i);
        faction_ptr->prev_income += money_rate;
        faction_ptr->money += money_rate*dt;
        const auto capital_site = &v.faces.items[faction_ptr->capital];
        const auto capital_point = capital_site->site;
        const auto upkeep = upkeep_coefficient * f->site.dist(capital_point);
        faction_ptr->money -= upkeep*dt;
        faction_ptr->prev_upkeep += upkeep;
    
        // pick a random neighbour
        rng = hash(rng);
        const auto neighbour_rel_idx = hash_intn(rng, 0, v.num_face_neighbours(i));
        const auto neighbour_idx = v.get_face_neighbour(i, neighbour_rel_idx);
        
        // then consider just buying them
        const auto other_region = &regions.items[neighbour_idx];
        const auto other_faction = other_region->faction_key;
        if (other_region->m_biome == BIOME_BIG_MOUNTAIN ||
            other_region->m_biome == BIOME_OCEAN) continue;

        if (other_faction != faction_key) {
            // price = ?? maybe amount of money
            //const auto price = 2 + factions.items[other_faction].money;
            const auto shared_edge_idx = v.face_shared_edge(i, neighbour_idx); // hopefully not -1
            const auto price = defensive_power(neighbour_idx, shared_edge_idx);
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
