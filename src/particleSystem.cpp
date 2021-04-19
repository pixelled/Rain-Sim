//
// Created by Richard Yan on 4/20/21.
//

#include "particleSystem.h"

void ParticleSystem::reset() {

}

void ParticleSystem::updateWind(Vector3D velocity) {
    this->wind = velocity;
}

void ParticleSystem::simulate(double frames_per_sec, double simulation_steps, vector<Vector3D> external_accelerations,
                              vector<CollisionObject *> *collision_objects) {
    for (int i = 0; i < 3 * this->width * this->height; i += 3) {
        int x = rand() % 256;
        this->wetMap[i] = x;
        this->wetMap[i + 1] = x;
        this->wetMap[i + 2] = x;
    }
}
