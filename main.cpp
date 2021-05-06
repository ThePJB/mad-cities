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

#define fatal(msg) printf("Fatal Error: %s (%s:%d)\n", msg, __FILE__, __LINE__)
#define len(X) (sizeof(X)/sizeof(X[0]))

int chosen_one = 0;



const auto num_points = 400;


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

    world w(seed, num_points);
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
                    const auto new_world = world(seed, num_points);
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