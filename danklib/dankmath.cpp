#include "dankmath.hpp"

// normal % is remainder not mod
int dm_mod(int x, int y) {
  //if (y == 0) exit(-1);
  //if (y == -1) return 0;
  int m = x % y;
  if (m < 0) {
    m = (y < 0) ? m - y : m + y;
  }
  return m;
}

float dm_floor(float x) {
    if (x > 0) {
        return (int)x;
    } else if (x < 0) {
        return (int)(x-1);
    } else {
        return 0;
    }
}

float dm_abs(float x) {
    return x > 0 ? x : -x;
}

float dm_frac(float x) {
    return x - dm_floor(x);
}

// good polynomials t = t*t(3-2*t)
// t = t*t*t(6*t*t - 15*t + 10)
float dm_lerp(float a, float b, float t) {
    return t*b + (1-t)*a;
}

float dm_unlerp(float a, float b, float t) {
    return (t - a) / (b - a);
}

float dm_bilinear(float a1, float a2,float b1, float b2, float tnum, float talpha) {
    return dm_lerp(dm_lerp(a1, a2, tnum), dm_lerp(b1, b2, tnum), talpha);
}

float dm_bilinear3(float a1, float a2,float b1, float b2, float tnum, float talpha) {
    return dm_bilinear(a1, a2, b1, b2, tnum*tnum*(3-2*tnum), talpha*talpha*(3-2*talpha));
}

float dm_remap(float prev_lower, float prev_upper, float new_lower, float new_upper, float a) {
    return dm_lerp(new_lower, new_upper, dm_unlerp(prev_lower, prev_upper, a));
}

float dm_clamp(float lower, float x, float upper) {
    return x < lower ? lower : x > upper ? upper : x;
}

float dm_slow_start(float t) {
    return t*t;
}

float dm_slow_stop(float t) {
    return 1 - dm_slow_start(1-t);
}

float dm_slow_start2(float t) {
    return dm_slow_start(dm_slow_start(t));
}

float dm_slow_stop2(float t) {
    return dm_slow_stop(dm_slow_stop(t));
}

float dm_slow_start_stop(float t) {
    return dm_lerp(
        dm_slow_start(t),
        dm_slow_stop(t),
        t
    );
}

float dm_slow_start_stop2(float t) {
    return dm_slow_start_stop(dm_slow_start_stop(t));
}