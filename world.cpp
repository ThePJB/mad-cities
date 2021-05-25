#include "world.hpp"
#include "coolmath.hpp"
#include "historical_figure.hpp"
#include <string.h>

const auto upkeep_coefficient = 2.5;

region::region(int idx, uint32_t faction, point p, world *w) {
    const auto b = w->get_biome(p);
    faction_key = faction;
    voronoi_idx = idx;
    m_biome = b;
}

float road_cost(point p1, point p2) {
    return 15*p1.dist(p2);
}

const char *world::faction_name(uint32_t faction_id) {
    if (faction_id == faction_gaia) {
        return "gaia";
    }
    if (!factions.contains(faction_id)) {
        printf("warning: unknown faction name for key %u\n", faction_id);
        return "UNKNOWN_FACTION";
    }
    return factions.get(faction_id)->name;
    
}

void world::change_region_owner(int region_idx, uint32_t new_owner) {
    if (region_idx < 0 || region_idx >= regions.length) {
        printf("bad change region owner region idx %d length %d\n", region_idx, regions.length);
        exit(1);
    }
    const auto old_owner = regions.get(region_idx)->faction_key;

    regions.get(region_idx)->faction_key = new_owner;

    factions.get(new_owner)->owned_regions.push(region_idx);

    if (old_owner == faction_gaia) return;
    factions.get(old_owner)->owned_regions.remove_item(region_idx);
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
    hf_titles_init();
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

    // set up regions
    for (int i = 0; i < v.faces.length; i++) {
        auto f = &v.faces.items[i];

        auto new_region = region(i, faction_gaia, f->site, this);
        regions.push(new_region);
    }

    // set up factions
    for (int i = 0; i < v.faces.length; i++) {
        auto r = regions.get(i);
        if (r->m_biome == BIOME_OCEAN ||
            r->m_biome == BIOME_BIG_MOUNTAIN ||
            hash_floatn(seed + 32434 + i, 0, 1) > 0.1) {
            continue;
        }

        // create a new faction here
        auto new_faction = faction(seed, i);
        factions.set(new_faction.id, new_faction);
        printf("creating %s\n", new_faction.name);
        r->city = true;
        change_region_owner(i, new_faction.id);
    }

    // set up rivers
    for (int i = 0; i < v.edges.length; i++) {
        rivers.push(0.0f);
    }
    make_rivers();

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

// handles instability
// handles income
void world::update_faction(bucket<faction> *f, double dt) {
    
    // instability ticks down at 1.0 per year
    f->item.instability -= 1.0 * dt;
    if (f->item.instability < 0) {
        f->item.instability = 0;
    }

    // Income and upkeep
    auto this_cycle_income = 0.0;
    auto this_cycle_upkeep = 0.0;

    const auto leader = f->item.leader;
    const auto upkeep_multi =
            leader.personality == P_GOOD_ADMINISTRATOR ? 0.5 :
            leader.personality == P_BAD_ADMINISTRATOR ? 2.0 :
            1.0;

    for (int i = 0; i < f->item.owned_regions.length; i++) {
        const auto region_idx = f->item.owned_regions.items[i];
        this_cycle_income += income(region_idx);

        const auto capital_point = v.faces.items[f->item.capital].site;
        this_cycle_upkeep += upkeep_multi * upkeep_coefficient * v.faces.get(region_idx)->site.dist(capital_point);
    }

    f->item.prev_income = this_cycle_income;
    f->item.prev_upkeep = this_cycle_upkeep;

    f->item.money += dt * this_cycle_income;
    f->item.money -= dt * this_cycle_upkeep;

    // if not enough money it gets put on the instability tab
    if (f->item.money < 0) {
        f->item.instability -= f->item.money;
        f->item.money = 0;
    }

    rng = hash(rng);
    // Natural leader death
    if (hash_floatn(rng, 0, 1) < 0.0002) {
        // leader dies
        f->item.leader.print_name();
        printf(" of %s ", f->item.name);
        printf(" has died of natural causes");
        if (hash_floatn(rng + 12324, 0, 1) < 0.1) {
            // succession crisis
            printf(", leaving no heir. He is succeeded by ");
            f->item.leader = historical_figure(rng + 45334);
            f->item.leader.print_name();
        } else {
            // succession ok
            printf(". He is succeeded by his son ");
            f->item.leader = historical_figure(rng + 45334, f->item.leader);
            f->item.leader.print_name();
        }
        printf(". Long may he reign!\n");
        return;
    }

    // assassination due to instability
    auto assassination_tendency = 0.0005 * f->item.instability;
    if (assassination_tendency > 0.002) {
        assassination_tendency = 0.002;
    }

    if (hash_floatn(rng + 574726, 0, 1) < assassination_tendency) {
        // leader dies
        f->item.leader.print_name();
        printf(" of %s ", f->item.name);
        printf(" has been assassinated");
        if (hash_floatn(rng + 125324, 0, 1) < 0.1) {
            // succession crisis
            printf(", leaving no heir. He is succeeded by ");
            f->item.leader = historical_figure(rng + 45334);
            f->item.leader.print_name();
        } else {
            // succession ok
            printf(". He is succeeded by his son ");
            f->item.leader = historical_figure(rng + 45334, f->item.leader);
            f->item.leader.print_name();
        }
        printf(". Long may he reign!\n");
        return;

    }

    // city defection
    if (f->item.instability > 10) {
        for (int i = 0; i < f->item.owned_regions.length; i++) {
            const auto region_idx = f->item.owned_regions.items[i];         // faction owned region contains something out of bounds?

            fflush(stdout);
            if (region_idx < 0 || region_idx >= regions.length) {
                printf("bad region idx: %d for faction %s owned region %d (array length %d)\n", region_idx, f->item.name, i, f->item.owned_regions.length);
                // does it work fine if i continue here?
            }

            if (regions.items[region_idx].city) {                           // segv here??? bad region idx?
                if (hash_floatn(rng + 53452 + i*43325 + region_idx*342345, 0, 1) < f->item.instability * 0.00002) {
                    // city defects
                    auto new_faction = faction(rng+region_idx+seed+f->item.id, region_idx);
                    
                    // get money = to the instability for momentum
                    new_faction.money = f->item.instability;
                    factions.set(new_faction.id, new_faction);
                    change_region_owner(region_idx, new_faction.id);

                    // bonus points the name could be derived eg peoples or independent

                    // always seems to be the capital, and always seems to desync as well
                    // maybe give em a bit of money or something
                    // probably handle faction transfers through world
                    // or could have cells everywhere that turn and then get squashed or whatever


                    printf("A city defected from %s due to instability, forming %s, lead by ", f->item.name, new_faction.name);
                    new_faction.leader.print_name();
                    printf("\n");
                }
            }
        }
    }
}




void world::update(double dt) {
    time += dt;

    // zero faction income/upkeep
    factions.iter_begin();
    while (auto f = factions.iter_next()) {
        if (f->item.owned_regions.length == 0) {
            continue;
        }
        update_faction(f, dt);
    }

    for (int i = 0; i < v.faces.length; i++) {
        auto f = &v.faces.items[i];
        const auto faction_key = regions.items[i].faction_key;
        if (faction_key == faction_gaia) continue;
        faction *faction_ptr = factions.get(faction_key);

        // capturing
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

            const auto price_multi =
                faction_ptr->leader.personality == P_GOOD_GENERAL ? 0.5 :
                faction_ptr->leader.personality == P_BAD_GENERAL ? 2.0 :
                1.0;

            const auto price = price_multi * defensive_power(neighbour_idx, shared_edge_idx);
            rng = hash(rng);
            if ((hash_floatn(rng, 0, 1) < 0.01) && price < faction_ptr->money) {
                faction_ptr->money -= price;
                change_region_owner(neighbour_idx, faction_key);

                // report elimination
                if (other_faction != faction_gaia) {
                    if (factions.get(other_faction)->owned_regions.length == 0) {
                        printf("faction %s eliminated by %s\n", factions.get(other_faction)->name, faction_ptr->name);
                    }
                }
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
