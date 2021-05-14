#include "voronoi.hpp"
#include <string.h>
/*
// different to num edges
void voronoi::compute_num_neighbours() {
    for (int i = 0; i < diagram.numsites; i++) {
        auto edge = get_site(i)->edges;
        int count = 0;
        while (edge) {
            if (edge->neighbor != NULL) {
                count++;    
            }
            edge = edge->next;
        }
        num_neighbours.push(count);
    }
}

int voronoi::get_neighbour_idx(int idx, int which_neighbour) {
    auto edge = get_site(idx)->edges;
    int count = 0;
    while (edge) {
        if (edge->neighbor && count == which_neighbour) {
            return edge->neighbor->index;
        }
        if (edge->neighbor != NULL) {
            count++;
        }
        edge = edge->next;
    }
    printf("error getting neighbour idx\n");
    return -1;
}

voronoi::voronoi(vla<point> points) {
    points.sort([](point p1, point p2){return p1.y < p2.y;});
        jcv_rect r = {
        .min = {0, 0},
        .max = {1, 1},
    };

    memset(&diagram, 0, sizeof(jcv_diagram));
    jcv_diagram_generate(points.length, points.items, &r, NULL, &diagram);
    compute_num_neighbours();
}

void voronoi::relax() {
    auto new_points = vla<point>();
    auto sites = jcv_diagram_get_sites(&diagram);
    for (int i = 0; i < diagram.numsites; i++) {
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

    jcv_rect r = {
        .min = {0, 0},
        .max = {1, 1},
    };

    new_points.sort([](point p1, point p2){return p1.y < p2.y;});
    memset(&diagram, 0, sizeof(jcv_diagram));
    jcv_diagram_generate(new_points.length, new_points.items, &r, NULL, &diagram);
    new_points.destroy();
    num_neighbours.destroy();
    num_neighbours = vla<int>();
    compute_num_neighbours();
}

const jcv_site *voronoi::get_site(int idx) {
    auto sites = jcv_diagram_get_sites(&diagram);
    return &sites[idx];
}

int voronoi::get_idx_containing_point(point p) {
    int nearest = -999;
    float distance = 999999999;

    for (int i = 0; i < diagram.numsites; i++) {
        const auto s = get_site(i);
        const auto p1 = point(s->p.x, s->p.y);
        const auto candidate_dist = p.dist(p1);
        if (candidate_dist < distance) {
            distance = candidate_dist;
            nearest = i;
        }
    }
    return nearest;
}

void voronoi::destroy() {
    jcv_diagram_free(&diagram);
    num_neighbours.destroy();
}
*/