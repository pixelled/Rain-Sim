#include <vector>
#include "CGL/CGL.h"
#include "raindrop.h"
#include "collision/collisionObject.h"
#include "collision/plane.h"


#ifndef CLOTHSIM_PARTICLESYSTEM_H
#define CLOTHSIM_PARTICLESYSTEM_H


class ParticleSystem {
public:
    ParticleSystem(unsigned int width, unsigned int height, int count) {
        this->width = width;
        this->height = height;
        this->wind_f = Vector3D(0, 0, 0);
        this->count = count;
        this->collisionMapRes = width * height;
        wetMap = (char*) calloc((width * height * 3 + 3) / 4 * 4, sizeof(char));
        collisionMap = (unsigned char*)calloc(3 * collisionMapRes, sizeof(unsigned char));
    }

    ~ParticleSystem() {
        free(wetMap);
        free(collisionMap);
    }

    void init_raindrops();
    void reset();

    char* wetMap;
    unsigned char* collisionMap;
    unsigned int width;
    unsigned int height;
    int collisionMapRes;
    int count;
    Vector3D wind_f;
    double sky_midpoint;

    void updateWind(Vector3D wind_f);

    void simulate(double frames_per_sec, double simulation_steps,
                  vector<Vector3D> external_accelerations,
                  vector<CollisionObject *> *collision_objects);

    vector<Raindrop> raindrops;

    vector<Raindrop*> drops;

    // Spatial hashing
    unordered_map<float, vector<Raindrop *>*> map;
};


#endif //CLOTHSIM_PARTICLESYSTEM_H
