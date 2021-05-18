#pragma once

#include "voronoi.hpp"
#include "rendercontext.hpp"
#include "point.hpp"
#include "dict.hpp"

struct faction {
    uint32_t id;
    hsv colour = hsv(0,0,0);
    float money = 0;
    vla<int> owned_regions = vla<int>();
    int capital;
    char *name;

    faction(){};
    faction(uint32_t seed, int capital_idx);
};

struct road_segment {
    int start_site;
    int end_site;
    road_segment(int ss, int es) {
        start_site = ss;
        end_site = es;
    }
};

enum biome {
    BIOME_OCEAN,
    BIOME_PLAINS,
    BIOME_MOUNTAIN,
    BIOME_BIG_MOUNTAIN,
};

struct world;

struct region {
    int voronoi_idx;
    int faction_key;
    biome m_biome;

    region(int idx, uint32_t faction, point p, world *w);
};


struct world {
    voronoi v;
    //vla<faction> factions = vla<faction>();
    dict<faction> factions = dict<faction>();
    
    vla<region> regions = vla<region>();
    vla<road_segment> roads = vla<road_segment>();

    dict<float> river_segments = dict<float>(); // looking up by endpoints preferably?

    // edge* get_downstream_edge(edge*)
    // just call height fn at each edge and see what lower

    //  maybe<int,int> get_down_

    biome get_biome(point p, uint32_t seed);
    float height(point p);
    void make_rivers();

    uint32_t seed;
    uint32_t rng = 23424;
    
    world(uint32_t seed, int n_points, float p_faction);
    void destroy();
    void draw(render_context *rc, uint32_t highlight_faction);
    void update(double dt);
    float capture_price(int idx);
    int get_lowest_edge(int vert_idx);
};