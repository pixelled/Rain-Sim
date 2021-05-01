#version 330

in vec2 v_texcoords;
out vec4 out_color;

uniform sampler2D u_texture_4;

void main() {
  vec4 color = texture(u_texture_4, v_texcoords);
  if (color.a <= 0.1) {
  	discard;
  } else {
  	out_color = color;
  }
}