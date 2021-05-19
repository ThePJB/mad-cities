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
#include "world.hpp"
#include "fatal.hpp"
#define len(X) (sizeof(X)/sizeof(X[0]))

uint32_t rollover_faction = 0;
int rollover_vertex = 0;

const auto num_points = 400;
const auto p_faction = 0.1;

void test_round(float f) {
    auto bytes = just_float_bytes(f);
    auto round4 = round_float(bytes, 4);
    auto back4 = bytes_to_float(round4);
    auto round6 = round_float(bytes, 6);
    auto back6 = bytes_to_float(round6);
    auto round8 = round_float(bytes, 8);
    auto back8 = bytes_to_float(round8);
    printf("test round %f - %X:\n", f, bytes);
    printf("\t %f - %X\n", back4, round4);
    printf("\t %f - %X\n", back6, round6);
    printf("\t %f - %X\n", back8, round8);
}

int main(int argc, char** argv) {
    /*
    test_round(1.000f);
    test_round(0.9999999f);
    test_round(0.15243f);
    test_round(0.15247f);
    test_round(0.125f);
    test_round(0.124f);
    test_round(0.126f);
    return 0;
    */

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

    world w(seed, num_points, p_faction);
    auto rc = render_context(renderer, 0, 0, 900, 900);

    for (int i = 0; i < w.v.verts.length; i++) {
        printf("%d: %.10f %.10f\n", i, w.v.verts.items[i].site.x, w.v.verts.items[i].site.y);
    }

    auto keep_going = true;
    while (keep_going) {
        const auto start_tick = SDL_GetPerformanceCounter();
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                keep_going = false;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    const auto click_point = rc.pick(e.button.x, e.button.y);
                    rollover_vertex = w.v.pick_vertex(click_point);
                    rollover_faction = w.regions.items[w.v.pick_face(click_point)].faction_key;
                    printf("selected %s\n", w.factions.contains(rollover_faction) ? w.factions.get(rollover_faction)->name : "gaia");
                    printf("selected vertex %d\n", rollover_vertex);
                }
            } else if (e.type == SDL_KEYDOWN) {
                const auto sym = e.key.keysym.sym;
                if (sym == SDLK_r) {
                    seed++;
                    printf("remake time\n");
                    w.destroy();
                    const auto new_world = world(seed, num_points, p_faction);
                    memcpy(&w, &new_world, sizeof(world));
                }
            }
        }

        // draw
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        w.v.draw(&rc);
        w.draw(&rc, rollover_faction);

        rc.draw_circle(hsv(0, 1, 1), w.v.verts.get(rollover_vertex)->site, 5);
        const auto rollover_vertex_lowest_edge = w.v.edges.get(w.get_lowest_edge(rollover_vertex));
        rc.draw_line(hsv(0, 1, 1), 
            w.v.verts.get(rollover_vertex_lowest_edge->vert_idx.contents[0])->site,
            w.v.verts.get(rollover_vertex_lowest_edge->vert_idx.contents[1])->site,
            3
        );

        dt = 1.0f/60.0f;
        w.update(dt);

        auto keystate = SDL_GetKeyboardState(NULL);

/*
        if (keystate[SDL_SCANCODE_LSHIFT]) {
            w.update(dt);
            w.update(dt);
            w.update(dt);
        }
*/
        SDL_RenderPresent(renderer);
        const auto end_tick = SDL_GetPerformanceCounter();
        dt = ((double)(end_tick - start_tick))/tick_freq;
        const auto fps = 1.0 / dt;
    }
}