#ifndef CLOTHSIM_RAINDROP_H
#define CLOTHSIM_RAINDROP_H

#include "CGL/CGL.h"
#include "CGL/vector3D.h"
#include "collision/sphere.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "deque"

using namespace CGL;

class SpriteRenderer {
public:
	SpriteRenderer();
	void initRenderData();
	void update_view(Matrix4f& view);
	void update_texture_size(Vector3D& texture_size);
protected:
	unsigned int quadVAO;
	Matrix4f view;
	float size_x;
	float size_y;
};

class RaindropRenderer : public SpriteRenderer {
public:
	using SpriteRenderer::SpriteRenderer;
	void render(GLShader& shader, Vector3D& position, Vector3D& velocity);
};

class Raindrop {
public:
	Raindrop(Vector3D pos, Vector3D hit);

	void render(GLShader& shader, RaindropRenderer &raindrop_renderer);

	static Vector3D vel;
	Vector3D pos;
	Vector3D hit;
	Sphere* s;
};

struct SplashInfo {
	SplashInfo(Vector4f pos, unsigned int idx) : pos(pos), idx(idx) {}

	Vector4f pos;
	unsigned int idx;
};

class SplashRenderer : public SpriteRenderer {
public:
	SplashRenderer(int num_frames=20);

	void initRenderData();

	void render(GLShader& shader, SplashInfo &s);
	void render_all(GLShader& shader, bool is_paused);
	void add_splash(Vector3D& position);
	deque<SplashInfo> splashes;
	unsigned int end_idx;
	float len_x;
	float len_y;
};

#endif // CLOTHSIM_RAINDROP_H