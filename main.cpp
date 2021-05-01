#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "danklib/vla.hpp"
#include "danklib/rng.hpp"
#include "danklib/dict.hpp"

#define JC_VORONOI_IMPLEMENTATION
#include "lib/voronoi/src/jc_voronoi.h"

#include "colour.hpp"

#define fatal(msg) printf("Fatal Error: %s (%s:%d)\n", msg, __FILE__, __LINE__)
#define len(X) (sizeof(X)/sizeof(X[0]))

int chosen_one = 0;

struct point {
    float x;
    float y;
    point(float x, float y) {
        this->x = x;
        this->y = y;
    }
};

struct road_segment {
    int start_site;
    int end_site;
    road_segment(int ss, int es) {
        start_site = ss;
        end_site = es;
    }
};

float dist(point p1, point p2) {
    return sqrtf((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

float road_cost(point p1, point p2) {
    return 5*dist(p1, p2);
}

struct world {
    jcv_diagram voronoi;
    vla<uint32_t> faction = vla<uint32_t>();
    vla<float> money = vla<float>();
    vla<int> num_neighbours = vla<int>();
    vla<road_segment> roads = vla<road_segment>();
    vla<hsv> colours = vla<hsv>();

    uint32_t rng = 23424;
    
    world() {
        uint32_t seed = 0;
        auto points = vla<jcv_point>();
        const auto npoint = 100;
        for (int i = 0; i < npoint; i++) {
            seed = hash(seed);
            float x = hash_floatn(seed, 0, 1);
            float y = hash_floatn(seed * 323098, 0, 1);
            const auto w = 10;
            //const auto x = (1.0 - (i % w * 1.0/w)); 
            //const auto y = (1.0 - (i / w * 1.0/w)); 
            const jcv_point point = {x, y};
            points.push(point);
            
        }
        points.sort([](jcv_point p1, jcv_point p2){return p1.y < p2.y;});
        jcv_rect r = {
            .min = {0, 0},
            .max = {1, 1},
        };

        memset(&voronoi, 0, sizeof(jcv_diagram));
        jcv_diagram_generate(points.length, points.items, &r, NULL, &voronoi);
        const auto sites = jcv_diagram_get_sites(&voronoi);
        for (int i = 0; i < points.length; i++) {
            printf("early pt idx %d %d\n", i, sites[i].index);
        }
/*
        // relax once
        sites = jcv_diagram_get_sites(&voronoi);
        for (int i = 0; i < voronoi.numsites; ++i) {
            const auto site = &sites[i];
            jcv_point sum = site->p;
            const jcv_graphedge* edge = site->edges;

            int count = 1;
            while (edge) {
                sum.x += edge->pos[0].x;
                sum.y += edge->pos[0].y;
                count++;
                edge = edge->next;
            }

            points.items[site->index].x = sum.x / count;
            points.items[site->index].y = sum.y / count;
        }

        jcv_diagram_free(&voronoi);
        memset(&voronoi, 0, sizeof(jcv_diagram));
        jcv_diagram_generate(points.length, points.items, &r, NULL, &voronoi);
    */

        //
        for (int i = 0; i < get_num_sites(); i++) {
            const auto site = get_site(i);
            const jcv_graphedge* edge = site->edges;

            int count = 1;
            while (edge) {
                count++;
                edge = edge->next;
            }
            num_neighbours.push(count - 1);
            faction.push(i);
            money.push(0.0);
            auto sat = 0.5;
            auto val = 0.9;
            auto hue = hash_floatn(get_faction(i), 0, 360);

            colours.push({hue, sat, val});
        }

        points.destroy();

        for (int i = 0; i < get_num_sites(); i++) {
            printf("site %d: index self reports as %d\n", i, get_site(i)->index);
            
        }
    }

    int get_region_containing_point(point p) {
        int nearest = -1;
        float distance = 999999999;

        for (int i = 0; i < get_num_sites(); i++) {
            const auto s = get_site(i);
            const auto p1 = point(s->p.x, s->p.y);
            const auto candidate_dist = dist(p, p1);
            if (candidate_dist < distance) {
                distance = candidate_dist;
                nearest = i;
            }
        }
        return nearest;
    }

    const jcv_site *get_site(int idx) {
        const auto sites = jcv_diagram_get_sites(&voronoi);
        return &sites[idx];
    }
    point get_location(int idx) {
        const auto site = get_site(idx);
        return point(site->p.x, site->p.y);
    }
    int get_num_neighbours(int idx) {
        return num_neighbours.items[idx]; // out of sync?
    }
    const jcv_site *get_neighbour_site(int idx, int which_neighbour) {
        auto edge = get_site(idx)->edges;
        // site has no edges wtf?
        for (int i = 0; i < which_neighbour; i++) {
            if (!edge->next) {
                if (idx == chosen_one) {
                    printf("bad neighbour site, thing %d looking up neighbour %d but only has %d neighbours\n", idx, which_neighbour, get_num_neighbours(idx));
                }
                return NULL;
            }
            edge = edge->next;
        }
        if (!edge) { 
            //printf("!edge\n"); 
            return NULL;
        } // not solving fundamental problem lol
        
        //if (!edge->neighbor) { printf("!edge->neighbour\n"); }
        return edge->neighbor;
    }

    uint32_t get_faction(int idx) {
        return faction.items[idx];
    }
    int get_num_sites() {
        return voronoi.numsites;
    }
    float get_money(int idx) {
        return money.items[idx];
    }
    void set_money(int idx, float amt) {
        money.items[idx] = amt;
    }
    void draw(SDL_Renderer *renderer, int xres, int yres) {
        for (int i = 0; i < get_num_sites(); i++) {
            const auto site = get_site(i);
            auto e = site->edges;
            while (e) {
                auto rgb = hsv2rgb(colours.items[i]);

                // colour selected poly
                if (i == chosen_one) {
                    rgb = {1, 0, 0};
                }

                // colour neighbours of selected poly
                for (int j = 0; j < get_num_neighbours(i); j++) {
                    auto neigh = get_neighbour_site(i, j);
                    if (!neigh) continue;
                    if (neigh->index == chosen_one) {
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
            const auto start = get_location(r.start_site);
            const auto end = get_location(r.end_site);
            thickLineRGBA(renderer, start.x * xres, start.y * yres, end.x * xres, end.y * yres, 5, 0xCC, 0xAA, 0x55, 0xFF);
        }

    }
    void update(double dt) {
        for (int i = 0; i < get_num_sites(); i++) {
            const auto money_rate = hash_floatn(get_faction(i) + 2312314, 0, 1);
            money.items[i] += money_rate*dt;

            // consider building road to neighbours
            // pick a random neighbour
            rng = hash(rng);
            const auto neighbour_site = get_neighbour_site(i, hash_intn(rng, 0, get_num_neighbours(i) - 1));
            if (!neighbour_site) return; // map boundaries might not have
            // also check not preexisting road. or could increase road goodness
            const auto i1 = i;
            const auto i2 = neighbour_site->index;
            const auto p1 = point(get_site(i)->p.x, get_site(i)->p.y);
            const auto p2 = point(neighbour_site->p.x, neighbour_site->p.y);
            
            const auto cost = road_cost(p1, p2);
            if (cost < get_money(i) && !roads.contains(road_segment(i1, i2)) && !roads.contains(road_segment(i1, i2))) {
                // actually make road
                set_money(i, get_money(i) - cost);
                roads.push(road_segment(i, neighbour_site->index));
                //printf("make road from %d to %d\n", i, neighbour_site->index);
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
                    chosen_one = w.get_region_containing_point(mouse_point);
                    printf("chosen one set to %d\n", chosen_one);
                }
            }
        }

        // draw
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        w.draw(renderer, xres, yres);
        w.update(dt);

        for (int i = 0; i < w.get_num_sites(); i++) {
            const float radius = (sqrtf(1 + w.money.items[i]) - 1) * 10;
            filledCircleRGBA(renderer, w.get_site(i)->p.x * xres, w.get_site(i)->p.y * yres, radius, 0xFF, 0x00, 0x00, 0xFF);
        }

        SDL_RenderPresent(renderer);
        const auto end_tick = SDL_GetPerformanceCounter();
        dt = ((double)(end_tick - start_tick))/tick_freq;
        const auto fps = 1.0 / dt;
    }
}