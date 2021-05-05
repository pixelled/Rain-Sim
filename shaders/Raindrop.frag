#version 330

in vec2 v_texcoords;
in vec4 v_position;
in vec4 v_normal;
out vec4 out_color;

uniform sampler2D u_texture_4;
uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;

void main() {
  vec4 color = texture(u_texture_4, v_texcoords);
  if (color.a <= 0.1) {
  	discard;
  } else {
  vec3 l = normalize(u_light_pos - v_position.xyz);
  vec3 vn = normalize(v_normal.xyz);
  float r = length(v_position - vec4(u_light_pos, 1.0));
  float dist = length(vec4(u_cam_pos, 1.0) - v_position);

  // change opacity distance
  float opacity = clamp(dist / 23.0, 0 , 1);

  vec3 v_normal_pos = normalize(u_cam_pos - v_position.xyz);
  vec3 h = normalize(v_normal_pos + l);
  vec3 light_diffuse = u_light_intensity / (r * r);
  vec3 Ia = vec3(1.0, 1.0, 1.0);
  //vec4 final_color = vec4((1.0* light_diffuse * pow(max(0.0, dot(vn, h)), 100)) + (0.6 * Ia) + (color.xyz * light_diffuse * max(0.0, dot(vn, l))), 1);
  vec4 final_color = color;
  final_color.a = final_color.a - opacity;

  out_color = final_color;
  }
}