#include "CGL/CGL.h"
#include "CGL/vector3D.h"

using namespace CGL;

struct Raindrop {
	Raindrop(double mass, Vector3D pos, Vector3D vel) : mass(mass), pos(pos), vel(vel) {}
	
	double mass;
	Vector3D pos;
	Vector3D vel;
	Vector3D forces;
};
