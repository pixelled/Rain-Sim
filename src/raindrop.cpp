#include "raindrop.h"
#include "rainSimulator.h"
#include <cmath>
#include <nanogui/nanogui.h>

#include "CGL/color.h"
#include "CGL/vector3D.h"
#include "CGL/CGL.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace nanogui;

Raindrop::Raindrop(Vector3D pos, Vector3D hit) {
	this->pos = pos;
	this->hit = hit;

	this->s = new Sphere(pos, 0.3, false, 0.4, 2, 2);
}

Vector3D Raindrop::vel = Vector3D(0, 0, 0);

void Raindrop::render(GLShader &shader, RaindropRenderer &raindrop_renderer) {
	raindrop_renderer.render(shader, pos, vel);
}

SpriteRenderer::SpriteRenderer() {
	initRenderData();
}

void SpriteRenderer::update_view(Matrix4f& view) {
	this->view = view;
}

void SpriteRenderer::initRenderData() {
	/* Reference: https://learnopengl.com/In-Practice/2D-Game/Rendering-Sprites */
	unsigned int VBO;
	float x = 0.1;
	float y = 0.2;
	float vertices[] = {
		// pos      // tex
		-x, y, 0.0f, 1.0f,
		x, -y, 1.0f, 0.0f,
		-x, -y, 0.0f, 0.0f,

		-x, y, 0.0f, 1.0f,
		x, y, 1.0f, 1.0f,
		x, -y, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &this->quadVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(this->quadVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void SpriteRenderer::update_texture_size(Vector3D& texture_size) {
	size_x = texture_size.x;
	size_y = texture_size.y;
}

void RaindropRenderer::render(GLShader& shader, Vector3D& position, Vector3D& velocity) {
	Vector4f pos(position.x, position.y, position.z, 1.0);
	Vector4f vel(velocity.x, velocity.y, velocity.z, 1.0);
	// Derive positions in view space
	pos = view * pos;
	vel = view * vel;
	Vector4f origin = view * Vector4f(0.0, 0.0, 0.0, 1.0);
	vel = vel - origin;
	// Rotation matrix in raindrop space
	Matrix4f m;
	Vector4f x = view * Vector4f(1.0, 0.0, 0.0, 1.0) - origin;
	Vector3D y(-vel(0), -vel(1), -vel(2));
	y.normalize();
	Vector3D z = cross(Vector3D(x(0), x(1), x(2)), y);
	m << x(0), y.x, z.x, 0, x(1), y.y, z.y, 0, x(2), y.z, z.z, 0, 0, 0, 0, 1;

	// u_model maps quad coordinates into view space
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos(0), pos(1), pos(2)));
	// Change back to eigen matrix
	Matrix4f u_model;
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			u_model(i, j) = model[j][i];
		}
	}

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

    float dist = -pos(2); //(position - Vector3D(4.0, 4.0, 4.0)).norm();
	shader.setUniform("opacity", clamp(5.f / dist, 0.f, 1.f));

	shader.setUniform("view", view);
	shader.setUniform("u_rotate", m);
	shader.setUniform("u_model", u_model);
	glBindVertexArray(this->quadVAO);

	shader.drawArray(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

SplashRenderer::SplashRenderer(int num_frames) {
	end_idx = num_frames - 1;
}

void SplashRenderer::initRenderData() {
	unsigned int VBO;
	len_x = size_x / (end_idx + 1);
	len_y = size_y;
	cout << len_x << " " << len_y;
	float vertices[] = {
		// pos      // tex
		-0.5f, 0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, len_x, len_y,
		-0.5f, -0.5f, 0.0f, len_y,

		-0.5f, 0.5f, 0.0f, 0.0f,
		0.5f, 0.5f, len_x, 0.0f,
		0.5f, -0.5f, len_x, len_y
	};

	glGenVertexArrays(1, &this->quadVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(this->quadVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void SplashRenderer::add_splash(Vector3D& position) {
	float scale = (float)rand() / (float)RAND_MAX * 0.05 + 0.05;
	splashes.emplace_back(Vector4f(position.x, position.y, position.z, 1.0), 0, scale);
}

void SplashRenderer::render(GLShader& shader, SplashInfo &s) {
	// Derive positions in view space
	Vector4f pos = view * s.pos;

	// u_model maps quad coordinates into view space
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos(0), pos(1) + s.scale - 0.03, pos(2)));
	/*model = glm::rotate(model, (float)atan(velocity.y / velocity.x), glm::vec3(0.0f, 0.0f, 1.0f));*/
	model = glm::scale(model, glm::vec3(glm::vec2(s.scale, s.scale), 1.0f));

	// Change back to eigen matrix
	Matrix4f u_model;
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			u_model(i, j) = model[j][i];
		}
	}

	Vector2f offset = Vector2f(s.idx * len_x, 0.0);
	shader.setUniform("u_offset", offset);
	shader.setUniform("u_model", u_model);
    float dist = -pos(2); //(position - Vector3D(4.0, 4.0, 4.0)).norm();
    shader.setUniform("u_world_pos", s.pos);
    shader.setUniform("opacity", clamp(3.f / dist, 0.f, 1.f), false);
	glBindVertexArray(this->quadVAO);

	shader.drawArray(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SplashRenderer::render_all(GLShader& shader, bool is_paused) {
	int c = 0;
	for (SplashInfo &s : splashes) {
		if (s.idx >= end_idx)
			c++;
		if (!is_paused)
			s.idx += 2;
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
        SplashRenderer::render(shader, s);
	}
	for (int i = 0; i < c; i++) {
		splashes.pop_front();
	}
}
