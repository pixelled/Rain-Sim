#version 330

in vec2 texCoords;
out vec4 out_color;

uniform sampler2D u_texture_4;

void main() {
  out_color = texture(u_texture_4, texCoords);
}
