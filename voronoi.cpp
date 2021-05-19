#include "voronoi.hpp"
#include "lib/jc_voronoi.h"
#include "dict.hpp"
#include <string.h>

vla<point> voronoi_relax_points(jcv_diagram *diagram) {
    auto new_points = vla<point>();
    auto sites = jcv_diagram_get_sites(diagram);
    for (int i = 0; i < diagram->numsites; i++) {
        const auto site = &sites[i];
        point sum = site->p;
        const jcv_graphedge* edge = site->edges;

        int count = 1;
        while (edge) {
            sum.x += edge->pos[0].x;
            sum.y += edge->pos[0].y;
            count++;
            edge = edge->next;
        }
        new_points.push({sum.x / count, sum.y / count});
    }

    return new_points;
}

voronoi::voronoi(vla<point> points) {
    points.sort([](point p1, point p2){return p1.y < p2.y;});
        jcv_rect r = {
        .min = {0, 0},
        .max = {1, 1},
    };

    jcv_diagram diagram;
    {
        memset(&diagram, 0, sizeof(jcv_diagram));
        jcv_diagram_generate(points.length, points.items, &r, NULL, &diagram);
        auto new_points = voronoi_relax_points(&diagram);
        new_points.sort([](point p1, point p2){return p1.y < p2.y;});
        jcv_diagram_free(&diagram);
        memset(&diagram, 0, sizeof(jcv_diagram));
        jcv_diagram_generate(new_points.length, new_points.items, &r, NULL, &diagram);
        new_points.destroy();
    }

    {
        const jcv_site* sites = jcv_diagram_get_sites(&diagram);

        // first push all faces
        for (int i = 0; i < diagram.numsites; i++) {
            face f = face();
            f.idx = i;
            f.site = sites[i].p;
            faces.push(f);
        }
    }

    // then push all edges and do all edge neighbourness
    {
        int edge_idx = 0;
        const jcv_edge *jedge = jcv_diagram_get_edges(&diagram);
        while (jedge) {
            if (jedge->pos[0].x == jedge->pos[1].x && jedge->pos[0].y == jedge->pos[1].y) {
                jedge = jedge->next;
                continue;
            }
            auto e = edge();
            e.idx = edge_idx;

            const auto face_idx1 = jedge->sites[0]->index;

            e.face_idx.put(face_idx1);
            auto face = faces.get(face_idx1);
            if (!face->edges.contains(edge_idx)) face->edges.push(edge_idx);
        
            if (jedge->sites[1]) {
                const auto face_idx2 = jedge->sites[1]->index;
                if (!face->neighbour_faces.contains(face_idx2)) face->neighbour_faces.push(face_idx2);
                e.face_idx.put(face_idx2);
                auto face = faces.get(face_idx2);
                if (!face->edges.contains(edge_idx)) face->edges.push(edge_idx);
                if (!face->neighbour_faces.contains(face_idx1)) face->neighbour_faces.push(face_idx1);
            }

            // actually nah its certainly wrong here

            // current problem: edges have stupid face indexes
            // it looks like their fate is closely tied to the faces which seem more correct,
            // like we write to the edge what its face is, then we look up that face and write to it that it has this edge

            // do edges get fucked with later?

            // could certainly write a function check consistency

            edges.push(e);

            edge_idx++;
            jedge = jedge->next;
        }
    }

    // now do vertices
    // this should fill them out
    {
        auto vertex_dict = dict<int>();
        int edge_idx = 0;
        int vert_idx = 0;
        const jcv_edge *jedge = jcv_diagram_get_edges(&diagram);
        while (jedge) {
            if (jedge->pos[0].x == jedge->pos[1].x && jedge->pos[0].y == jedge->pos[1].y) {
                jedge = jedge->next;
                continue;
            }
            auto vertex_try_adder = [&](point p){
                const auto round_bits = 10;
                const auto p_hash = hash(
                    round_float(just_float_bytes(p.x), round_bits) + 
                    344325*round_float(just_float_bytes(p.y), round_bits));
                if (!vertex_dict.contains(p_hash)) {
                    vertex v = vertex();
                    v.site = p;
                    v.idx = vert_idx;
                    v.edge_idx.put(edge_idx);
                    verts.push(v);
                    vertex_dict.set(p_hash, vert_idx);
                    edges.get(edge_idx)->vert_idx.put(vert_idx); // edge keeps vert

                    vert_idx++;
                } else {
                    const auto this_vert_idx = vertex_dict.get(p_hash);
                    auto v = verts.get(*this_vert_idx);
                    v->edge_idx.put(edge_idx);
                    edges.get(edge_idx)->vert_idx.put(*this_vert_idx); // edge keeps vert
                }
            };

            vertex_try_adder(jedge->pos[0]);
            vertex_try_adder(jedge->pos[1]);

            edge_idx++;
            jedge = jedge->next;
        }
        vertex_dict.destroy();
    }

    // shitty edge fixa (can I just delete these maybe?)
    for (int i = 0; i < edges.length; i++) {
        auto e = edges.get(i);
        if (e->vert_idx.contents[1] == empty_sentinel) {
            e->vert_idx.contents[1] = e->vert_idx.contents[0];
        }
    }

    // cleanup
    jcv_diagram_free(&diagram);
}