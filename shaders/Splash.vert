#version 330

in vec4 vertex;
out vec2 v_texcoords;
out float opacity_wet;

uniform vec4 u_world_pos;
uniform vec2 u_offset;
uniform mat4 u_model;
uniform mat4 u_view_projection;
uniform sampler2D u_texture_5;
uniform float avg_color;
uniform float opacity;

void main() {
  v_texcoords = vec2(vertex.z + u_offset.x, vertex.w);
  vec4 model_v = u_model * vec4(vertex.xy, 0, 1);
  vec4 view_v = u_view_projection * model_v;
  gl_Position = view_v;
  vec2 height_uv = u_world_pos.xz / 8.f;;
  float height_cutoff = min(1.f, 0.3f + 0.75f * avg_color);
  float height = texture(u_texture_5, height_uv).r;
  opacity_wet = min(1.f, opacity * min(4.f, min(1.f, 2.5f * avg_color) * 4.f * (height > height_cutoff ? 0.1f : 1.f)));

  //gl_Position = vec4(view_v.xy, 0, 1);
}