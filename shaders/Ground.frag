#version 330


uniform vec3 u_cam_pos;

uniform samplerCube u_texture_cubemap;
uniform sampler2D u_texture_3;

in vec4 v_position;
in vec4 v_normal;
in vec4 v_tangent;
in vec2 v_texcoords;

out vec4 out_color;

void main() {
  vec3 wo = normalize(u_cam_pos - vec3(v_position));
  vec3 wi = reflect(wo, vec3(v_normal));
  out_color = texture(u_texture_cubemap, wi);

  vec4 tex = texture(u_texture_3, v_texcoords);
  out_color.a = tex.r;
}
