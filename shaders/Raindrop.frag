#version 330

in vec2 v_texcoords;
out vec4 out_color;

uniform sampler2D u_texture_4;
uniform float opacity;
uniform float lighting_distance;

void main() {
  vec4 color = texture(u_texture_4, v_texcoords);
  float alpha = color.a * ((opacity + lighting_distance) / 2);

  if (alpha <= 0.1) {
  	discard;
  } else {
    out_color = color * vec4(1, 1, 1, alpha);
  }
}