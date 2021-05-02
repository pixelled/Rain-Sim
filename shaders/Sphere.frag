#version 330

uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;

uniform samplerCube u_texture_cubemap;
uniform sampler2D u_texture_1;

in vec4 v_position;
in vec4 v_normal;
in vec2 v_uv;

out vec4 out_color;

vec3 reflected(vec4 in_vec) {
  return vec3(in_vec.x, -in_vec.y, in_vec.z);
}

void main() {
  vec3 v_pos = vec3(v_position);
  vec3 v_n = vec3(v_normal);
  vec3 wo = normalize(u_cam_pos - v_pos);
  vec3 wi = reflect(wo, v_n);

  int p = 20;
  vec3 l = u_light_pos - v_pos;
  vec3 v = normalize(u_cam_pos - v_pos);
  float r2 = dot(l, l);
  l = l / sqrt(r2);
  vec3 h = normalize(v + l);
  vec3 I = u_light_intensity / r2;
  vec3 ambient = 0.05 * vec3(1, 1, 1);
  vec3 diffuse = I * vec3(1, 1, 1) * max(0.f, dot(v_n, l));
  vec3 specular = 0.5 * I * pow(max(0.f, dot(normalize(v_n), h)), p);
  vec3 environment = vec3(texture(u_texture_cubemap, -wi)) * 0.5 * (diffuse.r * 0.5 + 0.6);
  out_color = vec4(ambient + diffuse + specular + environment, 1);
}
