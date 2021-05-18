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
    "Proud",
    "Nasty",
    "Moonglow",
    "Muzz",
    "Skank",
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

biome world::get_biome(point p, uint32_t seed) {
    const auto h = height(p);
    if (h < 0.3) {
        return BIOME_OCEAN;
    } else if (h < 0.5) {
        return BIOME_PLAINS;
    } else if (h < 0.7) {
        return BIOME_MOUNTAIN;
    } else {
        return BIOME_BIG_MOUNTAIN;
    }
}

region::region(int idx, uint32_t faction, point p, world *w) {
    const auto b = w->get_biome(p, w->seed);
    faction_key = faction;
    voronoi_idx = idx;
    m_biome = b;
}

float road_cost(point p1, point p2) {
    return 15*p1.dist(p2);
}

float world::height(point p) {
    return 1.0 * hash_noise2(3 * p, 98944 + seed);
//    return 1.0 * hash_fbm2_4(3 * p, 98944 + seed);
}

int world::get_lowest_edge(int vert_idx) {
    // select edge
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
    return lowest_idx;
}

void world::make_rivers() {
    /*
    OK so here's how we're going to do rivers.
    The data structure is a vla of segments. Segments will be hash of start point + hash of end point, so like a commutative double dict.
    There will be upper and lower segment based on sampling heightmap. water flows downhill.

    will we sample each vertex or each edge. maybe vertices better. rivers shouldnt fork right?
    not sure how to sample the vertices though lol.

    so for each vertex, calculate rainfall, follow it downhill, adding rainfall to each visited edge. Keep going til it reaches the sea,
    or until there's a local minima. If local minima, maybe contribute a standing water value that can be used to make a lake or something
    at which point maybe you would rerun the algorithm to exit the lake

    then to draw the river its just look up the thickness of each segment
    */

   // get all edges
        // can I look up for an edge specific neighbouring edges?

    for (int i = 0; i < v.verts.length; i++) {
        const auto rainfall = 0.1; // todo noise
        auto current_vertex_idx = i;
        while (true) {
            auto current_vertex = v.verts.get(i);

            // check if river has flowed into ocean
            for (int j = 0; j < current_vertex->edge_idx.size(); j++) {
                const auto edge_idx = current_vertex->edge_idx.contents[j]; // wtf am i doing muddling verts and edges
                auto edge_neighbour_faces = v.edges.get(edge_idx)->face_idx;
                for (int k = 0; k < edge_neighbour_faces.size(); k++) {
                    if (regions.get(edge_neighbour_faces.contents[k])->m_biome == BIOME_OCEAN) {
                        goto done_river;
                    }
                }
            }

            const auto lowest_outgoing_edge_idx = get_lowest_edge(current_vertex_idx);
            const auto lowest_outgoing_edge = v.edges.get(v.verts.get(current_vertex_idx)->edge_idx.contents[lowest_outgoing_edge_idx]);
            const auto river_segment_key = hash(lowest_outgoing_edge->vert_idx.contents[0]) + hash(lowest_outgoing_edge->vert_idx.contents[1]);

            if (river_segments.contains(river_segment_key)) {
                *river_segments.get(river_segment_key) += rainfall;
            } else {
                river_segments.set(river_segment_key, rainfall);
            }

            // then I guess we hash it and store the result
            // and keep going til ocean or local minimum


            // figure out which edge to take
            // add the river segment thing
            // keep going until water tile

            // how to stop infinite loopage, maybe store generation of each river segment?
            // current vertex = other vertex
            auto old_vertex_idx = current_vertex_idx;
            current_vertex_idx = lowest_outgoing_edge->other_vertex(current_vertex_idx);
            auto new_current_v = v.verts.get(current_vertex_idx);

            // check if river is at a local minima
            // well that would be also if the height of the place its going is greater than the height of here
            if (height(v.verts.get(current_vertex_idx)->site) > height(v.verts.get(old_vertex_idx)->site)) {
                printf("Local minima\n");
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
    for (int i = 0; i < v.num_face_neighbours(idx); i++) {
        const auto neighbour = v.get_face_neighbour(idx, i);
        if (regions.get(neighbour)->faction_key == region->faction_key) {
            any_allies = true;
        }
    }

    if (!any_allies) {
        price *= 0.1;
    }

    return price;
}

enum overlay_type {
    OL_NONE,
    OL_INCOME,
    OL_HEIGHT,
    OL_EDGE_DOWNHILL,
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
    }


    for (int i = 0; i < v.faces.length; i++) {
        auto f = &v.faces.items[i];
        const auto faction_key = regions.items[f->idx].faction_key;

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

        if (overlay == OL_INCOME && regions.items[i].m_biome != BIOME_OCEAN && regions.items[i].m_biome != BIOME_BIG_MOUNTAIN) {
            const auto money_rate = hash_fbm2_4(4 * f->site, 2312314);
            colour = {1-money_rate, money_rate, 0.0f};
        }

        if (overlay == OL_HEIGHT) {
            const auto h = height(this->v.faces.get(i)->site);
            colour = {h, h, h};
        }

        for (int j = 0; j < f->edges.length; j++) {
            auto e = v.edges.get(*f->edges.get(j));
            const auto p1 = f->site;
            const auto p2 = v.verts.get( e->vert_idx.contents[0] )->site;
            const auto p3 = v.verts.get( e->vert_idx.contents[1] )->site;
            rc->draw_triangle(colour, p1, p2, p3);
        }
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

        const auto river_segment_hash = hash(edge->vert_idx.contents[0]) + hash(edge->vert_idx.contents[1]);
        if (river_segments.contains(river_segment_hash)) {
            const auto riverness = *river_segments.get(river_segment_hash);
            if (riverness > 0.0) {
                const auto width = sqrtf(10*riverness);
                rc->draw_line(rgb(0,0,1), p1, p2, width);
            }
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
    for (int i = 0; i < v.faces.length; i++) {
        auto f = &v.faces.items[i];
        const auto faction_key = regions.items[i].faction_key;
        if (faction_key == faction_gaia) continue;
        faction *faction_ptr = factions.get(faction_key);

        // first make money
        auto money_rate = hash_fbm2_4(4 * f->site, 2312314); // wow some retarded stuff going on with capital site
        faction_ptr->money += money_rate*dt;
        const auto capital_site = &v.faces.items[faction_ptr->capital];
        const auto capital_point = capital_site->site;
        const auto upkeep = upkeep_coefficient * f->site.dist(capital_point);
        faction_ptr->money -= upkeep*dt;
    
        // pick a random neighbour
        rng = hash(rng);
        const auto neighbour_idx = v.get_face_neighbour(i, hash_intn(rng, 0, v.num_face_neighbours(i)));
        
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
