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
	float vertices[] = {
		// pos      // tex
		-0.5f, 0.5f, 0.0f, 1.0f,
		0.5f, -0.5f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.0f,

		-0.5f, 0.5f, 0.0f, 1.0f,
		0.5f, 0.5f, 1.0f, 1.0f,
		0.5f, -0.5f, 1.0f, 0.0f
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

	// u_model maps quad coordinates into view space
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos(0), pos(1), pos(2)));
	model = glm::rotate(model, (float)atan(vel(0) / vel(1)), glm::vec3(0.0f, 0.0f, 1.0f));
	float scale = sqrt(sqrt(vel(0) * vel(0) + vel(1) * vel(1))) * 0.05;
	
	model = glm::scale(model, glm::vec3(glm::vec2(scale, scale), 1.0f));

	// Change back to eigen matrix
	Matrix4f u_model;
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			u_model(i, j) = model[j][i];
		}
	}

	/*shader.setUniform("view", view);*/
	shader.setUniform("u_model", u_model);
	glBindVertexArray(this->quadVAO);

	shader.drawArray(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SplashRenderer::add_splash(Vector3D& position) {
	splashes.emplace_back(Vector4f(position.x, position.y, position.z, 1.0), 0);
}

void SplashRenderer::render(GLShader& shader, SplashInfo &s) {
	// Derive positions in view space
	Vector4f pos = view * s.pos;

	// u_model maps quad coordinates into view space
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos(0), pos(1), pos(2)));
	//model = glm::rotate(model, (float)atan(velocity.y / velocity.x), glm::vec3(0.0f, 0.0f, 1.0f));
	//model = glm::scale(model, glm::vec3(glm::vec2(0.1, 0.1), 1.0f));

	// Change back to eigen matrix
	Matrix4f u_model;
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			u_model(i, j) = model[j][i];
		}
	}

	//shader.uploadAttrib("offset_x", s.idx * size_y);

	shader.setUniform("u_model", u_model);
	glBindVertexArray(this->quadVAO);

	shader.drawArray(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SplashRenderer::render_all(GLShader& shader) {
	int c = 0;
	for (SplashInfo &s : splashes) {
		if (s.idx == end_idx) {
			c++;
		}
		SplashRenderer::render(shader, s);
	}
	for (int i = 0; i < c; i++) {
		splashes.pop_front();
	}
}
