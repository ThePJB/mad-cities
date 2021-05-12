#pragma once

#include "vla.hpp"
#include "point.hpp"
#include "fixed_set.hpp"

// this should be a voronoi + a delauney when its done

struct face {
    int idx;
    vla<int> edges;
    point site;
};

struct edge {
    int idx;
    fixed_set<2> vert_idx = fixed_set<2>();
    fixed_set<2> face_idx = fixed_set<2>();
};

struct vertex {
    int idx = -1;
    fixed_set<3> edge_idx = fixed_set<3>();
    fixed_set<3> face_idx = fixed_set<3>();
    point site;
};

struct newvoronoi {
    vla<face> faces;
    vla<edge> edges;
    vla<vertex> verts;

    newvoronoi(vla<point> points);
};