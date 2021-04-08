#version 330

// (Every uniform is available here.)

uniform mat4 u_view_projection;
uniform mat4 u_model;

uniform float u_normal_scaling;
uniform float u_height_scaling;

uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;

// Feel free to add your own textures. If you need more than 4,
// you will need to modify the skeleton.
uniform sampler2D u_texture_1;
uniform sampler2D u_texture_2;
uniform sampler2D u_texture_3;
uniform sampler2D u_texture_4;

// Environment map! Take a look at GLSL documentation to see how to
// sample from this.
uniform samplerCube u_texture_cubemap;

in vec4 v_position;
in vec4 v_normal;
in vec4 v_tangent;
in vec2 v_uv;

out vec4 out_color;

void checkerboard() {
  int p = 50;
  vec3 l = u_light_pos - vec3(v_position);
  vec3 v = normalize(u_cam_pos - vec3(v_position));
  float r2 = dot(l, l);
  l = l / sqrt(r2);
  vec3 h = normalize(v + l);
  vec3 I = u_light_intensity / r2;
  vec3 ambient = 0.02 * vec3(1, 1, 1);
  vec3 color = vec3(1, 1, 1) * ((int(v_uv.x * 32) + int(v_uv.y * 32)) % 2);
  vec3 diffuse = I * vec3(color) * max(0.f, dot(vec3(v_normal), l));
  vec3 specular = I * pow(max(0.f, dot(normalize(vec3(v_normal)), h)), p);
  out_color = vec4(ambient + diffuse + specular, 1);
}

void main() {
  int p = 10;
  vec3 l = u_light_pos - vec3(v_position);
  vec3 v = normalize(u_cam_pos - vec3(v_position));
  float r2 = dot(l, l);
  l = l / sqrt(r2);
  vec3 h = normalize(v + l);
  vec3 I = u_light_intensity / r2;
  vec3 ambient = 0.02 * vec3(1, 1, 1);
  vec3 color = vec3(0.2, 0.2, 0.2);
  vec3 diffuse = I * vec3(color) * max(0.f, dot(vec3(v_normal), l));
  vec3 specular = I * pow(max(0.f, dot(normalize(vec3(v_normal)), h)), p);
  float roughness = texture(u_texture_4, v_uv).r - 0.5;
  out_color = vec4(ambient + diffuse * roughness + specular * (1 - roughness), 1);
}
