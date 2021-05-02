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

Raindrop::Raindrop(double mass, Vector3D pos, Vector3D vel) {
	this->mass = mass;
	this->pos = pos;
	this->last_pos = pos;
	this->vel = vel;

	this->s = new Sphere(pos, 0.3, false, 0.4, 2, 2);
}

void Raindrop::render(GLShader &shader, RaindropRenderer &raindrop_renderer) {
	raindrop_renderer.render(shader, pos, vel);
}

RaindropRenderer::RaindropRenderer() {
	initRenderData();
}

void RaindropRenderer::update_view(Matrix4f& view) {
	this->view = view;
}

void RaindropRenderer::update_proj(Matrix4f &projection, float screen_w, float screen_h) {
	this->projection = projection;
	this->screen_w = screen_w;
	this->screen_h = screen_h;
}

void RaindropRenderer::update_texture_size(Vector3D &texture_size) {
	size_x = texture_size.x;
	size_y = texture_size.y;
}

void RaindropRenderer::initRenderData() {
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

void RaindropRenderer::render(GLShader& shader, Vector3D &position, Vector3D &velocity) {
	Vector4f pos(position.x, position.y, position.z, 1.0);
	Vector4f vel(velocity.x, velocity.y, velocity.z, 1.0);
	// Derive positions in view space
	pos = view * pos;
	vel = view * vel;

	// u_model maps quad coordinates into view space
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos(0), pos(1), pos(2)));
	model = glm::rotate(model, (float)atan(velocity.y / velocity.x), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(glm::vec2(0.1, 0.1), 1.0f));

	// Change back to eigen matrix
	Matrix4f u_model;
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			u_model(i, j) = model[j][i];
		}
	}

	shader.setUniform("u_model", u_model);
	shader.setUniform("u_view_projection", projection);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(this->quadVAO);

	shader.drawArray(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
