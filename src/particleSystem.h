//
// Created by Richard Yan on 4/20/21.
//
#include <vector>
#include "CGL/CGL.h"
#include "collision/collisionObject.h"


#ifndef CLOTHSIM_PARTICLESYSTEM_H
#define CLOTHSIM_PARTICLESYSTEM_H


class ParticleSystem {
public:
    ParticleSystem(unsigned int width, unsigned int height, int count) {
        this->width = width;
        this->height = height;
        this->count = count;
        this->wind = Vector3D(0, 0, 0);
        wetMap = (char*) calloc((width * height * 3 + 3) / 4 * 4, sizeof(char));
    }

    ~ParticleSystem() {
        free(wetMap);
    }

    void reset();
    char* wetMap;
    unsigned int width;
    unsigned int height;
    int count;
    Vector3D wind;

    void updateWind(Vector3D velocity);

    void simulate(double frames_per_sec, double simulation_steps,
                  vector<Vector3D> external_accelerations,
                  vector<CollisionObject *> *collision_objects);


};


#endif //CLOTHSIM_PARTICLESYSTEM_H
