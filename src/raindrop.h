#ifndef CGL_RAINDROP_H
#define CGL_RAINDROP_H

#include "CGL/CGL.h"
#include "CGL/vector3D.h"
#include "collision/sphere.h"

using namespace CGL;

class Raindrop {
public:
	Raindrop(double mass, Vector3D pos, Vector3D vel);
	
	double mass;
	Vector3D pos;
	Vector3D last_pos;
	Vector3D vel;
	Vector3D forces;
	Sphere* s;
};

#endif // CGL_RAINDROP_H