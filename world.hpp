#pragma once

#include "voronoi.hpp"
#include "rendercontext.hpp"
#include "point.hpp"
#include "dict.hpp"

struct faction {
    uint32_t id;
    hsv colour = hsv(0,0,0);
    float money;
    vla<int> owned_regions;
    int capital;
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

struct region {
    int voronoi_idx;
    int faction_key;
    biome m_biome;

    region(int idx, float x, float y, uint32_t seed);
};


struct world {
    voronoi v;
    //vla<faction> factions = vla<faction>();
    dict<faction> factions = dict<faction>();
    
    vla<region> regions = vla<region>();
    vla<road_segment> roads = vla<road_segment>();

    uint32_t rng = 23424;
    
    world(uint32_t seed, int n_points);
    void destroy();
    void draw(render_context *rc);
    void update(double dt);
};