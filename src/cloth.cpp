#include <iostream>
#include <math.h>
#include <random>
#include <vector>

#include "cloth.h"
#include "collision/plane.h"
#include "collision/sphere.h"

using namespace std;

Cloth::Cloth(double width, double height, int num_width_points,
             int num_height_points, float thickness) {
    this->width = width;
    this->height = height;
    this->num_width_points = num_width_points;
    this->num_height_points = num_height_points;
    this->thickness = thickness;

    buildGrid();
    buildClothMesh();
}

Cloth::~Cloth() {
    point_masses.clear();
    springs.clear();

    if (clothMesh) {
        delete clothMesh;
    }
}

void Cloth::buildGrid() {
    int n_w = num_width_points, n_h = num_height_points;

    for (int j = 0; j < n_h; j++) {
        double t = (double) j * (double) height / (double) n_h;
        for (int i = 0; i < n_w; i++) {
            double x = (double) i * (double) width / (double) n_w;
            double y = orientation == HORIZONTAL ? 1 : t;
            double z = orientation == HORIZONTAL ? t : (((double) rand() / ((double) INT_MAX * 1000.)));
            point_masses.emplace_back(PointMass(Vector3D(x, y, z), false));
        }
    }

    for (auto & coords : pinned) {
        point_masses.at(coords[1] * n_w + coords[0]).pinned = true;
    }

    PointMass* pm = point_masses.data();
    for (int i = 0; i < n_w; i++) {
        for (int j = 0; j < n_h; j++, pm++) {
            if (j) springs.emplace_back(Spring(pm, pm - 1,STRUCTURAL));
            if (i) springs.emplace_back(Spring(pm, pm - n_h, STRUCTURAL));
            if (i && j) springs.emplace_back(Spring(pm, pm - 1 - n_h, SHEARING));
            if (i && (j < n_h - 1)) springs.emplace_back(Spring(pm, pm + 1 - n_h, SHEARING));
            if (j > 1) springs.emplace_back(Spring(pm, pm - 2,BENDING));
            if (i > 1) springs.emplace_back(Spring(pm, pm - n_h - n_h, BENDING));
        }
    }
}

void Cloth::simulate(double frames_per_sec, double simulation_steps, ClothParameters *cp,
                     vector<Vector3D> external_accelerations,
                     vector<CollisionObject *> *collision_objects) {
    double mass = width * height * cp->density / num_width_points / num_height_points;
    double delta_t = 1.0f / frames_per_sec / simulation_steps;

    // Compute total force acting on each point mass.
    for (auto & m : point_masses) {
        Vector3D f;
        for (auto & a : external_accelerations) f += mass * a;
        m.forces = f;
    }
    for (auto & s : springs) {
        if (s.spring_type == STRUCTURAL && !cp->enable_structural_constraints) continue;
        if (s.spring_type == SHEARING && !cp->enable_shearing_constraints) continue;
        if (s.spring_type == BENDING && !cp->enable_bending_constraints) continue;
        Vector3D d_ba = s.pm_a->position - s.pm_b->position;
        double norm = d_ba.norm();
        double ks_ = s.spring_type == BENDING ? 1 * cp->ks : cp->ks;
        Vector3D fs = ks_ * (norm - s.rest_length) / norm * d_ba;
        s.pm_a->forces -= fs;
        s.pm_b->forces += fs;
    }

    // Use Verlet integration to compute new point mass positions
    double damp_fac = 1. - cp->damping / 100.;
    for (auto &m : point_masses) {
        if (m.pinned) continue;
        Vector3D curr_pos = m.position;
        m.position = m.position + damp_fac * (m.position - m.last_position)
                + m.forces / mass * delta_t * delta_t;
        m.last_position = curr_pos;
    }

    // Handle self-collisions.
    build_spatial_map();
    for (auto &m : point_masses) {
        self_collide(m, simulation_steps);
    }

    // Handle collisions with other primitives.

    for (auto obj : *collision_objects) {
        for (auto &m : point_masses) {
            obj->collide(m);
        }
    }

    // Constrain the changes to be such that the spring does not change
    // in length more than 10% per timestep [Provot 1995].

    for (auto & s : springs) {
        PointMass &pm_a = *s.pm_a, &pm_b = *s.pm_b;
        double last_dist = (pm_a.last_position - pm_b.last_position).norm();
        Vector3D vec_ba = pm_a.position - pm_b.position;
        double curr_dist = vec_ba.norm();
        double max_dist = 1.1 * last_dist;
        if (curr_dist > max_dist) {
            vec_ba /= curr_dist;
            if (!pm_a.pinned && pm_b.pinned) {
                pm_a.position = pm_b.position + vec_ba * max_dist;
            } else if (pm_a.pinned) {
                pm_b.position = pm_a.position - vec_ba * max_dist;
            } else {
                Vector3D mid_pos = (pm_a.position + pm_b.position) * 0.5;
                pm_a.position = mid_pos + vec_ba * max_dist * 0.5;
                pm_b.position = mid_pos - vec_ba * max_dist * 0.5;
            }
        }
    }
}

void Cloth::build_spatial_map() {
    for (const auto &entry : map) {
        delete (entry.second);
    }
    map.clear();

    for (auto &m : point_masses) {
        float hash = hash_position(m.position);
        if (map.find(hash) == map.end()) map[hash] = new vector<PointMass*>();
        map[hash]->push_back(&m);
    }
}

void Cloth::self_collide(PointMass &pm, double simulation_steps) {
    // (Part 4): Handle self-collision for a given point mass.
    Vector3D sum_corr;
    int total_corr = 0;
    for (PointMass* &m : *map[hash_position(pm.position)]) {
        if (&pm != m) {
            Vector3D vec = pm.position - m->position;
            double dist = vec.norm();
            if (dist < 2 * thickness) {
                Vector3D corr = (2 * thickness - dist) * vec / dist;
                sum_corr += corr;
                total_corr++;
            }
        }
    }
    if (total_corr) {
        pm.position += sum_corr / total_corr / simulation_steps;
    }
}

float Cloth::hash_position(Vector3D pos) {
    // (Part 4): Hash a 3D position into a unique float identifier that represents membership in some 3D box volume.
    double w = 3 * width / num_width_points;
    double h = 3 * height / num_height_points;
    double t = max(w, h);
    double r = max(width, height);
    Vector3D truncated = Vector3D(trunc(pos.x / w), trunc(pos.y / h), trunc(pos.z / t));
    return (float) (truncated.z + r * (truncated.y + r * truncated.x));
}

///////////////////////////////////////////////////////
/// YOU DO NOT NEED TO REFER TO ANY CODE BELOW THIS ///
///////////////////////////////////////////////////////

void Cloth::reset() {
    PointMass *pm = &point_masses[0];
    for (int i = 0; i < point_masses.size(); i++) {
        pm->position = pm->start_position;
        pm->last_position = pm->start_position;
        pm++;
    }
}

void Cloth::buildClothMesh() {
    if (point_masses.size() == 0) return;

    ClothMesh *clothMesh = new ClothMesh();
    vector<Triangle *> triangles;

    // Create vector of triangles
    for (int y = 0; y < num_height_points - 1; y++) {
        for (int x = 0; x < num_width_points - 1; x++) {
            PointMass *pm = &point_masses[y * num_width_points + x];
            // Get neighboring point masses:
            /*                      *
             * pm_A -------- pm_B   *
             *             /        *
             *  |         /   |     *
             *  |        /    |     *
             *  |       /     |     *
             *  |      /      |     *
             *  |     /       |     *
             *  |    /        |     *
             *      /               *
             * pm_C -------- pm_D   *
             *                      *
             */

            float u_min = x;
            u_min /= num_width_points - 1;
            float u_max = x + 1;
            u_max /= num_width_points - 1;
            float v_min = y;
            v_min /= num_height_points - 1;
            float v_max = y + 1;
            v_max /= num_height_points - 1;

            PointMass *pm_A = pm;
            PointMass *pm_B = pm + 1;
            PointMass *pm_C = pm + num_width_points;
            PointMass *pm_D = pm + num_width_points + 1;

            Vector3D uv_A = Vector3D(u_min, v_min, 0);
            Vector3D uv_B = Vector3D(u_max, v_min, 0);
            Vector3D uv_C = Vector3D(u_min, v_max, 0);
            Vector3D uv_D = Vector3D(u_max, v_max, 0);


            // Both triangles defined by vertices in counter-clockwise orientation
            triangles.push_back(new Triangle(pm_A, pm_C, pm_B,
                                             uv_A, uv_C, uv_B));
            triangles.push_back(new Triangle(pm_B, pm_C, pm_D,
                                             uv_B, uv_C, uv_D));
        }
    }

    // For each triangle in row-order, create 3 edges and 3 internal halfedges
    for (int i = 0; i < triangles.size(); i++) {
        Triangle *t = triangles[i];

        // Allocate new halfedges on heap
        Halfedge *h1 = new Halfedge();
        Halfedge *h2 = new Halfedge();
        Halfedge *h3 = new Halfedge();

        // Allocate new edges on heap
        Edge *e1 = new Edge();
        Edge *e2 = new Edge();
        Edge *e3 = new Edge();

        // Assign a halfedge pointer to the triangle
        t->halfedge = h1;

        // Assign halfedge pointers to point masses
        t->pm1->halfedge = h1;
        t->pm2->halfedge = h2;
        t->pm3->halfedge = h3;

        // Update all halfedge pointers
        h1->edge = e1;
        h1->next = h2;
        h1->pm = t->pm1;
        h1->triangle = t;

        h2->edge = e2;
        h2->next = h3;
        h2->pm = t->pm2;
        h2->triangle = t;

        h3->edge = e3;
        h3->next = h1;
        h3->pm = t->pm3;
        h3->triangle = t;
    }

    // Go back through the cloth mesh and link triangles together using halfedge
    // twin pointers

    // Convenient variables for math
    int num_height_tris = (num_height_points - 1) * 2;
    int num_width_tris = (num_width_points - 1) * 2;

    bool topLeft = true;
    for (int i = 0; i < triangles.size(); i++) {
        Triangle *t = triangles[i];

        if (topLeft) {
            // Get left triangle, if it exists
            if (i % num_width_tris != 0) { // Not a left-most triangle
                Triangle *temp = triangles[i - 1];
                t->pm1->halfedge->twin = temp->pm3->halfedge;
            } else {
                t->pm1->halfedge->twin = nullptr;
            }

            // Get triangle above, if it exists
            if (i >= num_width_tris) { // Not a top-most triangle
                Triangle *temp = triangles[i - num_width_tris + 1];
                t->pm3->halfedge->twin = temp->pm2->halfedge;
            } else {
                t->pm3->halfedge->twin = nullptr;
            }

            // Get triangle to bottom right; guaranteed to exist
            Triangle *temp = triangles[i + 1];
            t->pm2->halfedge->twin = temp->pm1->halfedge;
        } else {
            // Get right triangle, if it exists
            if (i % num_width_tris != num_width_tris - 1) { // Not a right-most triangle
                Triangle *temp = triangles[i + 1];
                t->pm3->halfedge->twin = temp->pm1->halfedge;
            } else {
                t->pm3->halfedge->twin = nullptr;
            }

            // Get triangle below, if it exists
            if (i + num_width_tris - 1 < 1.0f * num_width_tris * num_height_tris / 2.0f) { // Not a bottom-most triangle
                Triangle *temp = triangles[i + num_width_tris - 1];
                t->pm2->halfedge->twin = temp->pm3->halfedge;
            } else {
                t->pm2->halfedge->twin = nullptr;
            }

            // Get triangle to top left; guaranteed to exist
            Triangle *temp = triangles[i - 1];
            t->pm1->halfedge->twin = temp->pm2->halfedge;
        }

        topLeft = !topLeft;
    }

    clothMesh->triangles = triangles;
    this->clothMesh = clothMesh;
}
