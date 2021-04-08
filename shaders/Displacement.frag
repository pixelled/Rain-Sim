#version 330

uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;

uniform vec4 u_color;

uniform sampler2D u_texture_2;
uniform vec2 u_texture_2_size;

uniform float u_normal_scaling;
uniform float u_height_scaling;

in vec4 v_position;
in vec4 v_normal;
in vec4 v_tangent;
in vec2 v_uv;

out vec4 out_color;

void main() {
  vec3 n = normalize(vec3(v_normal));
  vec3 t = normalize(vec3(v_tangent));
  mat3 TBN = mat3(t, cross(n, t), n);
  float curr_h = texture(u_texture_2, v_uv).r;

  float d_u = (texture(u_texture_2, v_uv + vec2(1.0 / u_texture_2_size.x, 0.0)).r - curr_h) * u_normal_scaling * u_height_scaling;
  float d_v = (texture(u_texture_2, v_uv + vec2(0.0, 1.0 / u_texture_2_size.y)).r - curr_h) * u_normal_scaling * u_height_scaling;
  vec3 nd = normalize(TBN * vec3(-d_u, -d_v, 1));

  int p = 50;
  vec3 l = u_light_pos - vec3(v_position);
  vec3 v = normalize(u_cam_pos - vec3(v_position));
  float r2 = dot(l, l);
  l = l / sqrt(r2);
  vec3 h = normalize(v + l);
  vec3 I = u_light_intensity / r2;
  vec3 ambient = 0.02 * vec3(1, 1, 1);
  vec3 diffuse = I * vec3(u_color) * max(0.f, dot(nd, l));
  vec3 specular = I * pow(max(0.f, dot(nd, h)), p);
  out_color = vec4(ambient + diffuse + specular, 1);
}

