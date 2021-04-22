#include "particleSystem.h"

void ParticleSystem::init_raindrops() {
    drops = vector<Raindrop*>();

    for (int i = 0; i < 500; i += 1) {
        double x = (double(rand()) / RAND_MAX - 0.5) * 5;
        double z = (double(rand()) / RAND_MAX - 0.5) * 5;
        double y = (double(rand()) / RAND_MAX - 0.5) * 5;
        
        drops.push_back(new Raindrop(1.0, Vector3D(x, 5.0 + y, z), Vector3D(0, 0, 0)));
    }
}

void ParticleSystem::reset() {

}

void ParticleSystem::updateWind(Vector3D wind_f) {
    this->wind_f = wind_f;
}

void ParticleSystem::simulate(double frames_per_sec, double simulation_steps, vector<Vector3D> external_accelerations,
                              vector<CollisionObject *> *collision_objects) {
    double delta_t = 1.0f / frames_per_sec / simulation_steps;

    Vector3D accs = Vector3D(0, 0, 0);
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

    for (int i = 0; i < drops.size(); i += 1) {
        // Reset forces
        drops[i]->forces = accs * drops[i]->mass;

        // correct the velocity and position
        drops[i]->vel += (drops[i]->forces / drops[i]->mass) * delta_t * simulation_steps;

        drops[i]->last_pos = drops[i]->pos;
        drops[i]->pos += 0.1 * drops[i]->vel * delta_t * simulation_steps;

        for (CollisionObject *co : *collision_objects) {
            if (typeid(*co) == typeid(Plane)) {
                Plane* p = (Plane*) co;
                if (p->collide(*drops[i])) {
                    // If the droplet collided with the groud, replace it with a new droplet in the sky
                    double x = (double(rand()) / RAND_MAX - 0.5) * 5;
                    double z = (double(rand()) / RAND_MAX - 0.5) * 5;
                    double y = (double(rand()) / RAND_MAX - 0.5) * 5;
                    drops[i]->pos = Vector3D(x, 5.0 + y, z);
                    drops[i]->vel = Vector3D(0, 0, 0);
                }             
            }
            
        }
             

        // Update the sphere
        drops[i]->s->origin = drops[i]->pos;
    }
}
