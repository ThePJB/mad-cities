#pragma once
#ifndef M_PI
    #define M_PI 3.14159265358979323846264338327950288
#endif

int cm_clamp(int lower, int x, int upper);
float cm_clamp(float lower, float x, float upper);

float cm_floor(float x);
float cm_abs(float x);
float cm_frac(float x);
float cm_lerp(float a, float b, float t);
float cm_unlerp(float a, float b, float t);
float cm_remap(float prev_lower, float prev_upper, float new_lower, float new_upper, float a);

// easing functions
float cm_slow_start(float t);
float cm_slow_stop(float t);
float cm_slow_start2(float t);
float cm_slow_stop2(float t);
float cm_slow_start_stop(float t);
float cm_slow_start_stop2(float t);
float cm_slow_start_stop3(float t);
float cm_slow_start_stop4(float t);