#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "danklib/vla.hpp"
#include "danklib/rng.hpp"
#include "danklib/dict.hpp"

#define JC_VORONOI_IMPLEMENTATION
#include "lib/jc_voronoi.h"
#include "voronoi.hpp"

#include "colour.hpp"
#include "point.hpp"

#define fatal(msg) printf("Fatal Error: %s (%s:%d)\n", msg, __FILE__, __LINE__)
#define len(X) (sizeof(X)/sizeof(X[0]))

int chosen_one = 0;

struct region {
    int voronoi_idx;
    int faction_idx;
};

struct faction {
    uint32_t id;
    hsv colour;
    float money;
    vla<int> owned_regions;
};

struct road_segment {
    int start_site;
    int end_site;
    road_segment(int ss, int es) {
        start_site = ss;
        end_site = es;
    }
};

float road_cost(point p1, point p2) {
    return 5*p1.dist(p2);
}

struct world {
    voronoi v;
    vla<faction> factions = vla<faction>();
    vla<region> regions = vla<region>();
    vla<road_segment> roads = vla<road_segment>();

    uint32_t rng = 23424;
    
    world() {
        uint32_t seed = 0;
        auto points = vla<jcv_point>();
        const auto npoint = 100;
        for (int i = 0; i < npoint; i++) {
            seed = hash(seed);
            float x = hash_floatn(seed, 0, 1);
            float y = hash_floatn(seed * 323098, 0, 1);
            const jcv_point point = {x, y};
            points.push(point);
        }
        v = voronoi(points);
        v.relax();

        // populate region vla
        for (int i = 0; i < v.num_sites(); i++) {
            const auto site = v.get_site(i);
            const jcv_graphedge* edge = site->edges;

            regions.push((region) {
                .voronoi_idx = i,
                .faction_idx = i,
            });

            factions.push((faction){
                .id = hash(i),
                .colour = (hsv) {
                    .h = hash_floatn(hash(i), 0, 360),
                    .s = 0.5,
                    .v = 0.9,
                },
                .money = 0,
                .owned_regions = vla<int>(),
            });
            factions.items[factions.length-1].owned_regions.push(i);
        }

        points.destroy();
    }

    void draw(SDL_Renderer *renderer, int xres, int yres) {
        for (int i = 0; i < v.num_sites(); i++) {
            const auto site = v.get_site(i);
            const auto faction_idx = regions.items[site->index].faction_idx;
            auto e = site->edges;
            while (e) {
                // look up the colour of the faction of the site
                auto rgb = hsv2rgb(factions.items[faction_idx].colour);

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
  
                filledTrigonRGBA(renderer,
                    xres * site->p.x, yres * site->p.y, 
                    xres * e->pos[0].x, yres * e->pos[0].y, 
                    xres * e->pos[1].x, yres * e->pos[1].y,
                    rgb.r * 255,
                    rgb.g * 255,
                    rgb.b * 255,
                    255);
                e = e->next;
            }
        }
        // draw roads
        for (int i = 0; i < roads.length; i++) {
            const auto r = roads.items[i];
            const auto start = v.get_site(r.start_site)->p;
            const auto end = v.get_site(r.end_site)->p;
            thickLineRGBA(renderer, start.x * xres, start.y * yres, end.x * xres, end.y * yres, 5, 0xCC, 0xAA, 0x55, 0xFF);
        }
    }

    void update(double dt) {
        for (int i = 0; i < v.num_sites(); i++) {
            const auto faction_idx = regions.items[i].faction_idx;
            const auto faction_ptr = &factions.items[faction_idx];

            const auto money_rate = hash_floatn(faction_idx + 2312314, 0, 1);
            faction_ptr->money += money_rate*dt;
        
            // consider building road to neighbours
            // pick a random neighbour
            rng = hash(rng);
            const auto neighbour_idx = v.get_neighbour_idx(i, hash_intn(rng, 0, v.get_num_neighbours(i) - 1));
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

    const auto xres = 800;
    const auto yres = 800;

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

    auto w = world();

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
                }
            }
        }

        // draw
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        w.draw(renderer, xres, yres);
        w.update(dt);

        SDL_RenderPresent(renderer);
        const auto end_tick = SDL_GetPerformanceCounter();
        dt = ((double)(end_tick - start_tick))/tick_freq;
        const auto fps = 1.0 / dt;
    }
}