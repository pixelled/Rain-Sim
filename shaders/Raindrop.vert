#version 330

layout (location = 0) in vec4 vertex;
out vec2 v_texcoords;
out vec4 v_position;

uniform mat4 u_rotate;
uniform mat4 u_model;
uniform mat4 u_view_projection;
uniform vec4 view_position;
//uniform mat4 view;

void main() {
  v_texcoords = vertex.zw;
  v_position = u_view_projection * view_position;
  gl_Position = u_view_projection * u_model * u_rotate * vec4(vertex.xy, 0.0, 1.0);
}