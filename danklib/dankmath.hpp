#pragma once

#define M_PI 3.14159265358979323846
#define DEG_TO_RAD (M_PI/180.0)
#define RAD_TO_DEG (180/M_PI)


int dm_mod(int x, int y);

float dm_bilinear(float a1, float a2,float b1, float b2, float tnum, float talpha);
float dm_bilinear3(float a1, float a2,float b1, float b2, float tnum, float talpha);

float dm_floor(float x);
float dm_abs(float x);
float dm_frac(float x);
float dm_lerp(float a, float b, float t);
float dm_unlerp(float a, float b, float t);
float dm_remap(float prev_lower, float prev_upper, float new_lower, float new_upper, float a);

float dm_clamp(float lower, float x, float upper);

// easing functions
float dm_slow_start(float t);
float dm_slow_stop(float t);
float dm_slow_start2(float t);
float dm_slow_stop2(float t);
float dm_slow_start_stop(float t);
float dm_slow_start_stop2(float t);