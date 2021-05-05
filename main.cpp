#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "danklib/vla.hpp"
#include "danklib/rng.hpp"
#include "danklib/dict.hpp"
#include "rendercontext.hpp"

#define JC_VORONOI_IMPLEMENTATION
#include "lib/jc_voronoi.h"
#include "voronoi.hpp"

#include "colour.hpp"
#include "point.hpp"

#define fatal(msg) printf("Fatal Error: %s (%s:%d)\n", msg, __FILE__, __LINE__)
#define len(X) (sizeof(X)/sizeof(X[0]))

int chosen_one = 0;


const int faction_gaia = 99999989;

const auto num_points = 400;
const auto upkeep_coefficient = 1.5;


struct faction {
    uint32_t id;
    hsv colour = hsv(0,0,0);
    float money;
    vla<int> owned_regions;
    int capital;
};

struct road_segment {
    int start_site;
    int end_site;
    road_segment(int ss, int es) {
        start_site = ss;
        end_site = es;
    }
};

enum biome {
    BIOME_OCEAN,
    BIOME_PLAINS,
    BIOME_MOUNTAIN,
    BIOME_BIG_MOUNTAIN,
};

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

struct region {
    int voronoi_idx;
    int faction_key;
    biome m_biome;

    region(int idx, float x, float y, uint32_t seed) {
        const auto b = get_biome(x, y, seed);
        if (b == BIOME_OCEAN || b == BIOME_BIG_MOUNTAIN) {
            // no faction
            faction_key = faction_gaia;
        } else {
            // new faction
            faction_key = hash(idx);
        }
        voronoi_idx = idx;
        m_biome = b;
    }
};

float road_cost(point p1, point p2) {
    return 15*p1.dist(p2);
}


struct world {
    voronoi v;
    //vla<faction> factions = vla<faction>();
    dict<faction> factions = dict<faction>();
    
    vla<region> regions = vla<region>();
    vla<road_segment> roads = vla<road_segment>();

    uint32_t rng = 23424;
    
    world(uint32_t seed) {
        auto points = vla<point>();
        for (int i = 0; i < num_points; i++) {
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

            regions.push(region(i, site->p.x, site->p.y, seed));
            if (regions.items[regions.length-1].faction_key != faction_gaia) {
                factions.set(hash(i), (faction){
                    .id = hash(i),
                    .colour = hsv(
                        hash_floatn(hash(i)+seed, 0, 360),
                        0.5,
                        0.9
                    ),
                    .money = 0,
                    .owned_regions = vla<int>(),
                    .capital = i,
                });
                factions.get(hash(i))->owned_regions.push(i);
            }
        }

        points.destroy();
    }

    void destroy() {
        regions.destroy();
        factions.destroy();
        roads.destroy();
        v.destroy();
    }

    void draw(render_context *rc) {
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

            rgb rgb = {1, 1, 1}; 
            if (regions.items[i].m_biome == BIOME_PLAINS) {
                rgb = hsv2rgb(f_ptr->colour);
            } else if (regions.items[i].m_biome == BIOME_MOUNTAIN) {
                auto mut_hsv = f_ptr->colour; 
                mut_hsv.s *= 0.6;
                //mut_hsv.v *= 1.2;
                rgb = hsv2rgb(mut_hsv);
            } else if (regions.items[i].m_biome == BIOME_OCEAN) {
                rgb = {0, 0, 0.8};
            } else if (regions.items[i].m_biome == BIOME_BIG_MOUNTAIN) {
                rgb = {0.4, 0.4, 0.4};
            } else {
                rgb = {1, 0, 1};
            }

            // colour selected poly
            if (i == chosen_one) {
                rgb = {1, 0, 0};
            }

            // colour neighbours of selected poly
            for (int j = 0; j < v.get_num_neighbours(i); j++) {
                const auto nidx = v.get_neighbour_idx(i, j);
                if (nidx == chosen_one) {
                    rgb = {0, 1, 0};
                    break;
                }
            }

            if (income_view && regions.items[i].m_biome != BIOME_OCEAN && regions.items[i].m_biome != BIOME_BIG_MOUNTAIN) {
                auto money_rate = hash_fbm2_4(4 * site->p, 2312314);
                rgb = {1-money_rate, money_rate, 0.0f};
            }

            auto e = site->edges;
            while (e) {
                rc->draw_triangle(rgb, site->p, e->pos[0], e->pos[1]);
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
                rc->draw_line(rgb(1,1,1), edge->pos[0], edge->pos[1], 1);
            }
            edge = edge->next;
        }

        factions.iter_begin();
        while (auto f = factions.iter_next()) {
            auto capital = f->item.capital;
            auto capital_faction = regions.items[capital].faction_key;
            if (capital_faction == f->key) {
                rc->draw_circle(rgb(0,0,0), v.get_site(capital)->p, 3);
            } else {
                // faction with no capital lol
            }
        }
    }

    void update(double dt) {
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
            if (other_faction == faction_gaia) continue;
            if (other_faction != faction_key) {
                // price = ?? maybe amount of money
                //const auto price = 2 + factions.items[other_faction].money;
                auto price = 10;
                if (other_region->m_biome == BIOME_MOUNTAIN) {
                    price = 50;
                }
                rng = hash(rng);
                if ((hash_floatn(rng, 0, 1) < 0.01) && price < faction_ptr->money) {
                    //printf("faction %d buying region %d from %d\n", faction_key, neighbour_idx, other_faction);
                    faction_ptr->money -= price;
                    regions.items[neighbour_idx].faction_key = faction_key;

                    factions.get(other_faction)->owned_regions.remove_item(neighbour_idx);
                    if (factions.get(other_faction)->owned_regions.length == 0) {
                        printf("faction %u eliminated by %u\n", other_faction, faction_key);
                    }
                    faction_ptr->owned_regions.push(neighbour_idx);
                }
            }


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
        }
    }
};

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fatal("couldn't initialize SDL");
    };

    const auto tick_freq = SDL_GetPerformanceFrequency();

    auto dt = 0.0;

    uint32_t seed = 0;
    const auto xres = 1600;
    const auto yres = 900;

    const auto window = SDL_CreateWindow(
        "mad cities", 
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        xres,
        yres,
        SDL_WINDOW_SHOWN
    );
    if (window == NULL) fatal("null window");

    const auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) fatal("null renderer");

    world w(seed);
    auto rc = render_context(renderer, 0, 0, 900, 900);

    auto keep_going = true;
    while (keep_going) {
        const auto start_tick = SDL_GetPerformanceCounter();
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                keep_going = false;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    // get index of polygon
                    const auto mouse_point = point(((float)e.button.x)/xres, ((float)e.button.y)/yres);
                    chosen_one = w.v.get_idx_containing_point(mouse_point);
                    printf("chosen one set to %d\n", chosen_one);
                    printf("biome %d\n", w.regions.items[chosen_one].m_biome);
                }
            } else if (e.type == SDL_KEYDOWN) {
                const auto sym = e.key.keysym.sym;
                if (sym == SDLK_r) {
                    seed++;
                    //w.remake(seed);
                    printf("remake time\n");
                    w.destroy();
                    const auto new_world = world(seed);
                    memcpy(&w, &new_world, sizeof(world));
                }
            }
        }

        // draw
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        w.draw(&rc);
        w.update(dt);

        SDL_RenderPresent(renderer);
        const auto end_tick = SDL_GetPerformanceCounter();
        dt = ((double)(end_tick - start_tick))/tick_freq;
        const auto fps = 1.0 / dt;
    }
}