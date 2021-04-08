#version 330

uniform vec4 u_color;
uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;

in vec4 v_position;
in vec4 v_normal;
in vec2 v_uv;

out vec4 out_color;

void main() {
  int p = 50;
  vec3 l = u_light_pos - vec3(v_position);
  vec3 v = normalize(u_cam_pos - vec3(v_position));
  float r2 = dot(l, l);
  l = l / sqrt(r2);
  vec3 h = normalize(v + l);
  vec3 I = u_light_intensity / r2;
  vec3 ambient = 0.02 * vec3(1, 1, 1);
  vec3 diffuse = I * vec3(u_color) * max(0.f, dot(vec3(v_normal), l));
  vec3 specular = I * pow(max(0.f, dot(normalize(vec3(v_normal)), h)), p);
  out_color = vec4(ambient + diffuse + specular, 1);
}

