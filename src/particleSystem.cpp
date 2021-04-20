#include "particleSystem.h"

void ParticleSystem::init_raindrops() {

}

void ParticleSystem::reset() {

}

void ParticleSystem::updateWind(Vector3D wind_f) {
    this->wind_f = wind_f;
}

void ParticleSystem::simulate(double frames_per_sec, double simulation_steps, vector<Vector3D> external_accelerations,
                              vector<CollisionObject *> *collision_objects) {
    double delta_t = 1.0f / frames_per_sec / simulation_steps;

    Vector3D accs = 0.;
    for (Vector3D& a : external_accelerations) {
        accs += a;
    }

    for (Raindrop& r : raindrops) {
        Vector3D air_resistance = -r.mass * Vector3D(r.vel.x * r.vel.x, r.vel.y * r.vel.y, r.vel.z * r.vel.z);
        r.forces = wind_f + air_resistance;
        Vector3D acc = r.forces / r.mass + accs;
        Vector3D pos_temp = r.pos;
        Vector3D vel_temp = r.vel;
        r.vel = r.vel + acc * delta_t;
        r.pos = r.pos + (r.vel + vel_temp) * delta_t / 2.;
    }

    for (int i = 0; i < 3 * this->width * this->height; i += 3) {
        int x = rand() % 256;
        this->wetMap[i] = x;
        this->wetMap[i + 1] = x;
        this->wetMap[i + 2] = x;
    }
}
