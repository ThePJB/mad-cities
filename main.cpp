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

const auto num_points = 800;
const auto p_faction = 0.05;

struct faction_info {
    uint32_t key;
    int n_territories;
    float net_income;
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

    world w(seed, num_points, p_faction);
    auto rc = render_context(renderer, 0, 0, 900, 900);

    auto keep_going = true;
    while (keep_going) {
        const auto start_tick = SDL_GetPerformanceCounter();
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                keep_going = false;
                fflush(stdout);
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    const auto click_point = rc.pick(e.button.x, e.button.y);
                    rollover_faction = w.regions.items[w.v.pick_face(click_point)].faction_key;
                    if (w.factions.contains(rollover_faction)) {
                        const auto f = w.factions.get(rollover_faction);
                        printf("selected %s\n", f->name);
                        printf("\tmoney: %.3f\n", f->money);
                        printf("\tincome: %.3f\n", f->prev_income);
                        printf("\tupkeep: %.3f\n", f->prev_upkeep);
                        printf("\tnet income: %.3f\n", f->prev_income - f->prev_upkeep);
                    } else {
                        printf("selected gaia\n");
                    }
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

        w.draw(&rc, rollover_faction);

        dt = 1.0f/480.0f;

        const auto prev_year = floorf(w.time);

        w.update(dt);

        auto keystate = SDL_GetKeyboardState(NULL);

        if (keystate[SDL_SCANCODE_LSHIFT]) {
            w.update(dt);
            w.update(dt);
            w.update(dt);
        }

        const auto current_year = floorf(w.time);
        if (current_year != prev_year) {
            auto faction_info_list = vla<faction_info>();

            printf("Year %.0f\n", current_year);
            w.factions.iter_begin();
            while (auto f = w.factions.iter_next()) {
                if (f->item.owned_regions.length == 0) continue;
                faction_info_list.push({f->key, f->item.owned_regions.length, f->item.prev_income - f->item.prev_upkeep});
            }
            faction_info_list.sort([](faction_info f1, faction_info f2) {
                return f1.n_territories > f2.n_territories;
            });
            for (int i = 0; i < faction_info_list.length; i++) {
                printf("\t[%d] [%2.2f/y] - %s\n", 
                    faction_info_list.get(i)->n_territories,
                    faction_info_list.get(i)->net_income, 
                    w.factions.get(faction_info_list.get(i)->key)->name
                );
            }
            faction_info_list.destroy();
        }

        SDL_RenderPresent(renderer);
        const auto end_tick = SDL_GetPerformanceCounter();
        dt = ((double)(end_tick - start_tick))/tick_freq;
        const auto fps = 1.0 / dt;
    }
}