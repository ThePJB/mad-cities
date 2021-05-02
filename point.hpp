#pragma once

#include <math.h>

struct point {
    float x;
    float y;
    point(float x, float y) {
        this->x = x;
        this->y = y;
    }

    float dist(point p2) {
        return sqrtf((x - p2.x) * (x - p2.x) + (y - p2.y) * (y - p2.y));
    }
};