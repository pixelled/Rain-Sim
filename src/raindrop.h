#ifndef CGL_RAINDROP_H
#define CGL_RAINDROP_H

#include "CGL/CGL.h"
#include "CGL/vector3D.h"
#include "collision/sphere.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

using namespace CGL;

class RaindropRenderer
{
public:
	RaindropRenderer();

	void render(GLShader& shader, Vector3D& position, Vector3D& velocity);
	void initRenderData();
	void update_view(Matrix4f& view);
	void update_proj(Matrix4f& projection, float screen_w, float screen_h);
	void update_proj(float screen_w, float screen_h);
	void update_texture_size(Vector3D& texture_size);
private:
	unsigned int quadVAO;
	Matrix4f view;
	Matrix4f projection;
	Matrix4f ortho_proj;
	double size_x;
	double size_y;
	float screen_w;
	float screen_h;
};

class Raindrop {
public:
	Raindrop(double mass, Vector3D pos, Vector3D vel);

	void render(GLShader& shader, RaindropRenderer &raindrop_renderer);
	
	double mass;
	Vector3D pos;
	Vector3D last_pos;
	Vector3D vel;
	Vector3D forces;
	Sphere* s;
};

#endif // CGL_RAINDROP_H