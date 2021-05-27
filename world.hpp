#pragma once

#include "voronoi.hpp"
#include "rendercontext.hpp"
#include "point.hpp"
#include "dict.hpp"
#include "historical_figure.hpp"
#include "faction.hpp"

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
    biome_prototype(rgb(0.1, 0.4, 0.7), 999, 0),
    biome_prototype(rgb(0.3, 0.7, 0.3), 0.4, 0.5),
    biome_prototype(rgb(0.3, 0.5, 0.3), 1.0, 0.4),
    biome_prototype(rgb(0.3, 0.3, 0.3), 999, 0),
    biome_prototype(rgb(0.8, 0.8, 0.4), 0.8, 0.3),
    biome_prototype(rgb(0.4, 0.4, 0.4), 1.2, 0.2),
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
    bool city = false;

    region(int idx, uint32_t faction, point p, world *w);
};


struct world {
    voronoi v;
    dict<faction> factions = dict<faction>();
    
    vla<region> regions = vla<region>();
    vla<road_segment> roads = vla<road_segment>();
    vla<float> rivers = vla<float>();

    float time = 0;

    // scalars
    float height(point p);
    float rainfall(point p);
    float defensive_power_base(int face_idx);
    float defensive_power(int face_idx, int edge_idx);
    float income(int face_idx);
    water_factor get_water_factor(int face_idx);
    biome get_biome(point p);
    
    void make_rivers();

    void change_region_owner(int region_idx, uint32_t new_owner);
    const char *faction_name(uint32_t faction_id);

    uint32_t seed;
    uint32_t rng = 23424;


    
    world(uint32_t seed, int n_points, float p_faction);
    void destroy();
    void draw(render_context *rc, uint32_t highlight_faction);
    void update(double dt);
    void update_faction(bucket<faction> *f, double dt);
    int get_lowest_edge(int vert_idx);
};

const static int faction_gaia = 99999989;