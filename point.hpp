#pragma once

#include <math.h>

struct point {
    float x;
    float y;

    point() {x = 0; y = 0; }
    point(float x, float y) {
        this->x = x;
        this->y = y;
    }

    float dist(point p2) const {
        return sqrtf((x - p2.x) * (x - p2.x) + (y - p2.y) * (y - p2.y));
    }

    point operator+(const point& p) {
        return point(x + p.x, y + p.y);
    }

    point operator*(const point& p) {
        return point(x * p.x, y * p.y);
    }

    point operator*(float f) {
        return point(x * f, y * f);
    }
};

inline point operator*(float f, const point& p) {
    return point(p.x * f, p.y * f);    
}