#pragma once

#include "lib/jc_voronoi.h"
#include "vla.hpp"
#include "point.hpp"


// wrapper for jc_voronoi
// all points 0-1
struct voronoi {
    jcv_diagram diagram;
    vla<int> num_neighbours = vla<int>();

    voronoi(){}; // lmao
    voronoi(vla<point> points);
    void compute_num_neighbours();

    int get_num_neighbours(int idx) {return num_neighbours.items[idx];};
    int get_neighbour_idx(int idx, int which_neighbour);

    void relax();
    const jcv_site *get_site(int idx);
    int num_sites() {return diagram.numsites;};
    int get_idx_containing_point(point p);
};

// im going to hide so much shit in here lol and then
// not have to worry about it