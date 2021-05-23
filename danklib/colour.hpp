#pragma once

struct rgb {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
    rgb(){}
    rgb(double r, double g, double b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
};

struct hsv {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
    hsv(){}
    hsv(double h, double s, double v) {
        this->h = h;
        this->s = s;
        this->v = v;
    }
};

hsv rgb2hsv(rgb in);
rgb hsv2rgb(hsv in);