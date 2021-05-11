#version 330

in vec2 in_uv;
out vec4 out_color;

uniform sampler2D u_texture_8;
uniform vec2 u_texture_8_size;

vec2 clamp(vec2 uv) {
  return vec2(min(1.f, max(0.f, uv.x)), min(1.f, max(0.f, uv.y)));
}

void main() {
//  out_color = vec4(vec3(texture(u_texture_8, in_uv)), 1);
//  return;
  float h_offset = 1.f / 320.f;
  float v_offset = 1.f / 240.f;
  float h_kernel[3] = float[](1, 3, 1);
  float v_kernel[5] = float[](1.3, 1.5, 2, 1.2, 1);
  float h_fac = 1.f / 5.f;
  float v_fac = 1.f / 7.f;

  vec3 v_vals[5];
  // cannot use a loop because god knows why
  int i;
  float v;
  i = -2;
  v = v_offset * i; v_vals[i + 2] = (h_kernel[0] * vec3(texture(u_texture_8, in_uv + vec2(-h_offset, v))) +
  h_kernel[1] * vec3(texture(u_texture_8, in_uv + vec2(0, v))) +
  h_kernel[2] * vec3(texture(u_texture_8, in_uv + vec2( h_offset, v)))) * h_fac;
  i = -1;
  v = v_offset * i; v_vals[i + 2] = (h_kernel[0] * vec3(texture(u_texture_8, in_uv + vec2(-h_offset, v))) +
  h_kernel[1] * vec3(texture(u_texture_8, in_uv + vec2(0, v))) +
  h_kernel[2] * vec3(texture(u_texture_8, in_uv + vec2( h_offset, v)))) * h_fac;
  i = 0;
  v = v_offset * i; v_vals[i + 2] = (h_kernel[0] * vec3(texture(u_texture_8, in_uv + vec2(-h_offset, v))) +
  h_kernel[1] * vec3(texture(u_texture_8, in_uv + vec2(0, v))) +
  h_kernel[2] * vec3(texture(u_texture_8, in_uv + vec2( h_offset, v)))) * h_fac;
  i = 1;
  v = v_offset * i; v_vals[i + 2] = (h_kernel[0] * vec3(texture(u_texture_8, in_uv + vec2(-h_offset, v))) +
  h_kernel[1] * vec3(texture(u_texture_8, in_uv + vec2(0, v))) +
  h_kernel[2] * vec3(texture(u_texture_8, in_uv + vec2( h_offset, v)))) * h_fac;
  i = 2;
  v = v_offset * i; v_vals[i + 2] = (h_kernel[0] * vec3(texture(u_texture_8, in_uv + vec2(-h_offset, v))) +
  h_kernel[1] * vec3(texture(u_texture_8, in_uv + vec2(0, v))) +
  h_kernel[2] * vec3(texture(u_texture_8, in_uv + vec2( h_offset, v)))) * h_fac;

  vec3 col = vec3(0.0);
  for (int i = 0; i < 5; i++) {
    col += v_vals[i] * v_kernel[i];
  }
  out_color = vec4(col * v_fac, 1);
}