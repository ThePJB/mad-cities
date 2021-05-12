#include "newvoronoi.hpp"
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

newvoronoi::newvoronoi(vla<point> points) {
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
        points.destroy();
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
            auto e = edge();
            e.idx = edge_idx;
            e.face_idx.put(jedge->sites[0]->index);
            e.face_idx.put(jedge->sites[1]->index);
            faces.get(e.face_idx.contents[0])->edges.push(edge_idx);
            faces.get(e.face_idx.contents[1])->edges.push(edge_idx);
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
            auto vertex_try_adder = [&](point p){
                edges.get(edge_idx)->vert_idx.put(vert_idx); // edge keeps vert
                const auto p_hash = hash(just_float_bytes(p.x)) ^ hash(just_float_bytes(p.y));
                if (!vertex_dict.contains(p_hash)) {
                    vertex v = vertex();
                    v.site = p;
                    v.idx = vert_idx;
                    v.edge_idx.put(edge_idx);
                    verts.push(v);
                    vertex_dict.set(p_hash, vert_idx);

                    vert_idx++;
                } else {
                    const auto this_vert_idx = vertex_dict.get(p_hash);
                    auto v = verts.get(*this_vert_idx);
                    v->edge_idx.put(edge_idx);
                    const auto face_index1 = edges.get(edge_idx)->face_idx.contents[0];
                    const auto face_index2 = edges.get(edge_idx)->face_idx.contents[1];
                    v->face_idx.put(face_index1);
                    v->face_idx.put(face_index2);
                    // well faces dont store the vertex
                }
            };

            vertex_try_adder(jedge->pos[0]);
            vertex_try_adder(jedge->pos[1]);

            edge_idx++;
            jedge = jedge->next;
        }
        vertex_dict.destroy();
    }

    // cleanup
    jcv_diagram_free(&diagram);
}