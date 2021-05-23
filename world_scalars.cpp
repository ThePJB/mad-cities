#include "world.hpp"

/*
Scalars
Base:
    - height
    - rainfall
    - water factor
Derived:
    - income
    - defence
    - biome
*/

float world::height(point p) {
        //return sqrtf(p.x*p.x + p.y*p.y);
    //return 1.0 * hash_noise2(3 * p, 98944 + seed);
    /*
    const auto new_point = point(
            hash_noise2(2*p, 324583 + seed),
            hash_noise2(2*p, 583628 + seed)
    );
    return 1.0 * hash_noise2(3 * new_point, 98944 + seed);
    */
    return 1.0 * hash_noise2(3 * p, 98944 + seed);
    //return 1.0 * hash_fbm2_4(3 * p, 98944 + seed);
}

float world::rainfall(point p) {
    return 0.2 * hash_noise2(3 * p, 44548328 + seed);
}

water_factor riverness_to_wf(float riverness) {
    return riverness > 3.0 ? WF_BIG :
        riverness > 1.0 ? WF_SMALL :
        WF_NONE;
}

float world::defensive_power(int face_idx, int edge_idx) {
    const auto p = v.faces.get(face_idx)->site;
    const auto intervening_riverness = riverness_to_wf(*rivers.get(edge_idx));
    const auto biome = regions.get(face_idx)->m_biome;
    const auto biome_factor = 
        biome == BIOME_BIG_MOUNTAIN ? 9999.0 :
        biome == BIOME_MOUNTAIN ? 1.0 :
        biome == BIOME_PLATEAU ? 1.2 :
        biome == BIOME_DESERT ? 0.8 :
        biome == BIOME_PLAINS ? 0.4 :
        biome == BIOME_OCEAN ? 9999.0 :
        0.0;

    const auto river_factor =
        intervening_riverness == WF_BIG ? 1.0 :
        intervening_riverness == WF_SMALL ? 0.5 :
        intervening_riverness == WF_NONE ? 0.0 :
        0.0;

    
    // check connection to neighbours
    bool isolated = true;
    const auto is_gaia = regions.get(face_idx)->faction_key == faction_gaia;
    if (is_gaia) {
        isolated = true;
    } else if (factions.get(regions.get(face_idx)->faction_key)->capital == face_idx) {
        isolated = false;
    } else {
        for (int i = 0; i < v.num_face_neighbours(face_idx); i++) {
            const auto neighbour = v.get_face_neighbour(face_idx, i);
            if (regions.get(neighbour)->faction_key == regions.get(face_idx)->faction_key) {
                isolated = false;
            }
        }
    }

    const auto isolation_factor = isolated ? 0.3 : 1.0;

    return isolation_factor * 20 * (biome_factor + river_factor); //+ isolatedness + riverness
}

water_factor world::get_water_factor(int face_idx) {
    const auto f = v.faces.get(face_idx);
    float max_water = 0;

    for (int i = 0; i < f->neighbour_faces.length; i++) {
        const auto neigh_f_idx = f->neighbour_faces.items[i];
        if (regions.get(neigh_f_idx)->m_biome == BIOME_OCEAN) {
            return WF_SEA;
        }
    }

    for (int i = 0; i < f->edges.length; i++) {
        const auto edge_idx = f->edges.items[i];
        const auto edge_water = *rivers.get(edge_idx);
        if (edge_water > max_water) {
            max_water = edge_water;
        }
    }
    return riverness_to_wf(max_water);    
}

float world::income(int face_idx) {
    const auto biome = regions.get(face_idx)->m_biome;
    const auto biome_factor = 
        biome == BIOME_BIG_MOUNTAIN ? 0 :
        biome == BIOME_MOUNTAIN ? 0.35 :
        biome == BIOME_DESERT ? 0.2 :
        biome == BIOME_PLAINS ? 0.5 :
        biome == BIOME_PLATEAU ? 0.1 :
        biome == BIOME_OCEAN ? 0 :
        0.0;

    const auto water = get_water_factor(face_idx);
    const auto water_factor =
        water == WF_NONE ? 0 :
        water == WF_SMALL ? 0.2 :
        water == WF_BIG ? 0.4 :
        water == WF_SEA ? 0.6 :
        0.0;

    return biome_factor + water_factor;
}

biome world::get_biome(point p) {
    const auto h = height(p);
    const auto r = rainfall(p);

    if (h < 0.3) {
        return BIOME_OCEAN;
    } else if (h < 0.5) {
        if (r < 0.1) {
            return BIOME_DESERT;
        }
        return BIOME_PLAINS;
    } else if (h < 0.7) {
        if (r < 0.1) {
            return BIOME_PLATEAU;
        }
        return BIOME_MOUNTAIN;
    } else {
        return BIOME_BIG_MOUNTAIN;
    }
}