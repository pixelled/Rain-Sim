#include "raindrop.h"

Raindrop::Raindrop(double mass, Vector3D pos, Vector3D vel) {
	this->mass = mass;
	this->pos = pos;
	this->last_pos = pos;
	this->vel = vel;

	this->s = new Sphere(pos, 0.05, 0.4); 
}