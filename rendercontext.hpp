#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "point.hpp"
#include "colour.hpp"

// Draw to it in 0,1 space and it will draw to a part of the screen
struct render_context {
    SDL_Renderer *renderer;
    int xo;
    int yo;
    int w;
    int h;

    render_context(SDL_Renderer *renderer, int xo, int yo, int w, int h) {
        this->renderer = renderer;
        this->xo = xo;
        this->yo = yo;
        this->w = w;
        this->h = h;
    }

    point pick(int x, int y) {
        return point(float(x - xo) / w, float(y - yo) / h);
    }

    void draw_rect(rgb colour, float x, float y, float w, float h) {
        SDL_SetRenderDrawColor(renderer, colour.r * 255, colour.g * 255, colour.b * 255, 255);
        SDL_Rect r = {lround(x*this->w + xo), lround(y*this->h + yo), this->w*w, this->h*h};
        SDL_RenderFillRect(renderer, &r);
    }

    void draw_rect(hsv colour, float x, float y, float w, float h) {
        const auto rgb = hsv2rgb(colour);
        draw_rect(rgb, x, y, w, h);
    }

    void draw_triangle(rgb colour, point p1, point p2, point p3) {
        filledTrigonRGBA(renderer, 
            lround(p1.x*this->w + xo),
            lround(p1.y*this->h + yo),
            lround(p2.x*this->w + xo),
            lround(p2.y*this->h + yo),
            lround(p3.x*this->w + xo),
            lround(p3.y*this->h + yo),
            colour.r * 255, colour.g * 255, colour.b * 255, 255
        );
    }

    void draw_triangle(hsv colour, point p1, point p2, point p3) {
        const auto rgb = hsv2rgb(colour);
        draw_triangle(rgb, p1, p2, p3);
    }

    void draw_line(rgb colour, point p1, point p2, int w) {
        thickLineRGBA(renderer, 
            lround(p1.x*this->w + xo),
            lround(p1.y*this->h + yo),
            lround(p2.x*this->w + xo),
            lround(p2.y*this->h + yo),
            w, colour.r * 255, colour.g * 255, colour.b * 255, 255
        );
    }

    void draw_line(hsv colour, point p1, point p2, int w) {
        const auto rgb = hsv2rgb(colour);
        draw_line(rgb, p1, p2, w);
    }

    void draw_circle(rgb colour, point p, int radius) {
        filledCircleRGBA(renderer, 
            lround(p.x*this->w + xo),
            lround(p.y*this->h + yo),
            radius,
            colour.r * 255, colour.g * 255, colour.b * 255, 255
        );
    }

    void draw_circle(hsv colour, point p, int radius) {
        const auto rgb = hsv2rgb(colour);
        draw_circle(rgb, p, radius);
    }

    void draw_arrow(hsv colour, point pbase, point ptip, float width) {
        float dx = ptip.x - pbase.x;
        float dy = ptip.y - pbase.y;

        dx /= sqrtf(dx*dx + dy*dy);
        dy /= sqrtf(dx*dx + dy*dy);

        const auto p2 = point(pbase.x + width*dy, pbase.y - width*dx);
        const auto p3 = point(pbase.x - width*dy, pbase.y + width*dx);

        draw_triangle(colour, ptip, p2, p3);
        
    }
};