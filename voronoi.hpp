#pragma once

#include <limits.h>
#include "vla.hpp"
#include "point.hpp"
#include "fixed_set.hpp"
#include "rendercontext.hpp"
#include "rng.hpp"

// this should be a voronoi + a delauney when its done

struct face {
    int idx;
    vla<int> edges;
    vla<int> neighbour_faces;
    point site;
};

struct edge {
    int idx;
    fixed_set<2> vert_idx = fixed_set<2>();
    fixed_set<2> face_idx = fixed_set<2>();

    int other_vertex(int this_vertex) {
        return vert_idx.contents[0] == this_vertex ?
            vert_idx.contents[1]:
            vert_idx.contents[0];
    }
};

struct vertex {
    int idx = -1;
    fixed_set<3> edge_idx = fixed_set<3>();
    point site;
};

struct vertex_outgoing_edges_iterator {
    vertex *vert;
    int current = 0;

    vertex_outgoing_edges_iterator(voronoi *diagram, int vertex_idx) {
        this->vert = diagram->verts.get(vertex_idx);
    }

    bool has_next() {
        return current < vert->edge_idx.size();
    }

    int next() {
        return vert->edge_idx.contents[current++];
    }
};

struct voronoi {
    vla<face> faces;
    vla<edge> edges;
    vla<vertex> verts;

    voronoi(){};
    voronoi(vla<point> points);

    void destroy() {
        for (int i = 0; i < faces.length; i++) {
            faces.items[i].edges.destroy();
        }
        faces.destroy();
        edges.destroy();
        verts.destroy();
    }

    void print_diagram() {
        printf("FACES:\n");
        for (int i = 0; i < faces.length; i++) {
            const auto p = faces.items[i].site;
            printf("\tface %d: %.3f %.3f\n", i, p.x, p.y);
            for (int j = 0; j < faces.items[i].edges.length; j++) {
                printf("\t\tedge %d\n", faces.items[i].edges.items[j]);
            }
        }
        printf("\n\n");
        printf("VERTS:\n");
        for (int i = 0; i < verts.length; i++) {
            const auto p = verts.get(i)->site;
            printf("\tvert %d - %.3f %.3f\n", i, p.x, p.y);
        }
        printf("\n\n");
        printf("EDGES:\n");
        for (int i = 0; i < edges.length; i++) {
            //const auto p1 = verts.items[edges.items[i].vert_idx.contents[0]].site;
            //const auto p2 = verts.items[edges.items[i].vert_idx.contents[1]].site;
            //printf("\tedge %d: %.3f,%.3f -- %.3f,%.3f\n", i, p1.x, p1.y, p2.x, p2.y); // segv here
            const auto v1 = edges.get(i)->vert_idx.contents[0];
            const auto v2 = edges.get(i)->vert_idx.contents[1];

            printf("\tedge %d: %d -- %d\n", i, v1, v2); // segv here
            for (int j = 0; j < 2; j++) {
                printf("\t\tface %d\n", edges.items[i].face_idx.contents[j]);
            }
        }
    }

    void check_invariants() {
        printf("checking invariants\n");

        // self identification
        for (int i = 0; i < verts.length; i++) {
            if (verts.get(i)->idx != i) {
                printf("invariant failed: vertex %d self identifies as %d\n", i, verts.get(i)->idx);
            }
        }
        for (int i = 0; i < faces.length; i++) {
            if (faces.get(i)->idx != i) {
                printf("invariant failed: faces %d self identifies as %d\n", i, faces.get(i)->idx);
            }
        }
        for (int i = 0; i < edges.length; i++) {
            if (edges.get(i)->idx != i) {
                printf("invariant failed: edges %d self identifies as %d\n", i, edges.get(i)->idx);
            }
        }

        // vertexes (0,1)
        for (int i = 0; i < verts.length; i++) {
            const auto p = verts.get(i)->site;
            if (p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0) {
                printf("invariant failed: vert %d located out of bounds\n", i);
            }
        }

        // vertexes have 3 edges
        // nah not an invariant, they can have 2
        /*
        for (int i = 0; i < verts.length; i++) {
            auto v = verts.get(i);
            if (v->edge_idx[0] == empty_sentinel || v->edge_idx[1] == empty_sentinel || v->edge_idx[2] == empty_sentinel)
        }
        */

        // edges have 2 verts (can have 1 face)
        for (int i = 0; i < edges.length; i++) {
            const auto e = edges.get(i);
            if (e->vert_idx.contents[0] == empty_sentinel || e->vert_idx.contents[1] == empty_sentinel) {
                printf("invariant failed: edge %d has less than 2 verts\n", i);
            }
        }

        printf("done\n");
    }

    void draw(render_context *rc) {
        for (int i = 0; i < faces.length; i++) {
            auto f = faces.get(i);
            for (int j = 0; j < f->edges.length; j++) {
                auto e = edges.get(*f->edges.get(j));
                const auto p1 = f->site;
                const auto p2 = verts.get( e->vert_idx.contents[0] )->site;
                const auto p3 = verts.get( e->vert_idx.contents[1] )->site;
                const auto colour = hsv(hash_floatn(i, 0, 360), 0.5, 0.9);
                rc->draw_triangle(colour, p1, p2, p3);
            }
        }
    }

    // ah these two are whats fucked like from before.
    // i might make a separate neighbours data structure for the faces
    // that would be easier lmao
    // very easy in fact just push at creation time

    int num_face_neighbours(int face_idx) {
        return faces.items[face_idx].neighbour_faces.length;
    }
    int get_face_neighbour(int face_idx, int neigh_number) {
        return faces.items[face_idx].neighbour_faces.items[neigh_number];
    }

    int pick_face(point p) {
        auto min_idx = 0;
        auto min_dist = 999.0;
        for (int i = 0; i < faces.length; i++) {
            auto d = p.dist(faces.get(i)->site);
            if (d < min_dist) {
                min_idx = i;
                min_dist = d;
            }
        }
        return min_idx;
    }

    int pick_vertex(point p) {
        auto min_idx = 0;
        auto min_dist = 999.0;
        for (int i = 0; i < verts.length; i++) {
            auto d = p.dist(verts.get(i)->site);
            if (d < min_dist) {
                min_idx = i;
                min_dist = d;
            }
        }
        return min_idx;
    }

    int other_vert(int edge_idx, int vert_idx) {
        auto e = edges.get(edge_idx);
        return e->vert_idx.contents[0] == vert_idx ?
            e->vert_idx.contents[1]:
            e->vert_idx.contents[0];
    }

};