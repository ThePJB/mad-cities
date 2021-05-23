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
    BIOME_DESERT,
    BIOME_PLATEAU,
    NUM_BIOMES,
};

struct biome_prototype {
    rgb colour;
    float defence;
    float income;

    biome_prototype(rgb colour, float defence, float income) {
        this->colour = colour;
        this->defence = defence;
        this->income = income;
    }
};

const static biome_prototype biome_prototypes[NUM_BIOMES] = {
    biome_prototype(rgb(0, 0, 1), 999, 0),
    biome_prototype(rgb(0.2, 0.8, 0), 0.4, 0.5),
    biome_prototype(rgb(0.1, 0.6, 0.1), 1.0, 0.35),
    biome_prototype(rgb(0.3, 0.3, 0.3), 999, 0),
    biome_prototype(rgb(0.7, 0.7, 0.0), 0.8, 0.2),
    biome_prototype(rgb(0.7, 0.7, 0.3), 1.2, 0.1),
};


enum water_factor {
    WF_NONE,
    WF_SMALL,
    WF_BIG,
    WF_SEA,
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
    vla<float> rivers = vla<float>();

    // scalars
    float height(point p);
    float rainfall(point p);
    float defensive_power(int face_idx, int edge_idx);
    float income(int face_idx);
    water_factor get_water_factor(int face_idx);
    biome get_biome(point p);
    
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

const static int faction_gaia = 99999989;