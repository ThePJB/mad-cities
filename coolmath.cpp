#include "coolmath.hpp"

int cm_clamp(int lower, int x, int upper) {
    if (x < lower) return lower;
    if (x > upper) return upper;
    return x;
}

float cm_clamp(float lower, float x, float upper) {
    if (x < lower) return lower;
    if (x > upper) return upper;
    return x;
}

float cm_floor(float x) {
    if (x > 0) {
        return (int)x;
    } else if (x < 0) {
        return (int)(x-1);
    } else {
        return 0;
    }
}

float cm_abs(float x) {
    return x > 0 ? x : -x;
}

float cm_frac(float x) {
    return x - cm_floor(x);
}

float cm_lerp(float a, float b, float t) {
    return t*b + (1-t)*a;
}

float cm_unlerp(float a, float b, float t) {
    return (t - a) / (b - a);
}

float cm_remap(float prev_lower, float prev_upper, float new_lower, float new_upper, float a) {
    return cm_lerp(new_lower, new_upper, cm_unlerp(prev_lower, prev_upper, a));
}

float cm_slow_start(float t) {
    return t*t;
}

float cm_slow_stop(float t) {
    return 1 - cm_slow_start(1-t);
}

float cm_slow_start2(float t) {
    return cm_slow_start(cm_slow_start(t));
}

float cm_slow_stop2(float t) {
    return cm_slow_stop(cm_slow_stop(t));
}

float cm_slow_start_stop(float t) {
    return cm_lerp(
        cm_slow_start(t),
        cm_slow_stop(t),
        t
    );
}

float cm_slow_start_stop2(float t) {
    return cm_slow_start_stop(cm_slow_start_stop(t));
}

float cm_slow_start_stop3(float t) {
    return cm_slow_start_stop(cm_slow_start_stop(cm_slow_start_stop(t)));
}

float cm_slow_start_stop4(float t) {
    return cm_slow_start_stop(cm_slow_start_stop(cm_slow_start_stop(cm_slow_start_stop(t))));
}