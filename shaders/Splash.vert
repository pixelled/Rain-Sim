#version 330

layout (location = 0) in vec4 vertex;
out vec2 v_texcoords;
out vec4 v_position;
out vec4 v_normal;

uniform mat4 u_model;
uniform mat4 u_view_projection;

void main() {
  v_texcoords = vertex.zw;
  v_normal = normalize(u_model * vec4(1.0, 0.0, 0.0, 0.0));
  v_position = u_model * vec4(vertex.xy, 0.0, 1.0);
  gl_Position = u_view_projection * u_model * vec4(vertex.xy, 0.0, 1.0);
}