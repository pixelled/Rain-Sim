#version 330

in vec2 v_texcoords;
in vec4 v_position;
out vec4 out_color;

uniform sampler2D u_texture_4;
uniform sampler2D u_texture_9;
uniform float opacity;
uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;

void main() {
  vec4 color = texture(u_texture_4, v_texcoords);
  float alpha = color.a * opacity;

  if (alpha <= 0.1) {
  	discard;
  } else {
  //vec3 v_n = normalize(2.0 * texture(u_texture_9, v_texcoords).rgb - 1.0);
  vec3 v_n = vec3(0.0, 1.0, 0.0);
  float ambientStrength = 0.1;
  vec3 lightDir = normalize(u_light_pos.xyz - v_position.xyz); 
    
//  int p = 50;
    vec3 l = u_light_pos - vec3(v_position);
//  vec3 v = normalize(u_cam_pos - vec3(v_position));
    float r2 = dot(l, l);
//  l = l / sqrt(r2);
//  vec3 h = normalize(v + l);
    vec3 I = u_light_intensity / r2;
    vec3 ambient = 0.02 * vec3(1, 1, 1);
    float diff = max(dot(v_n, lightDir), 0.0);
    vec3 diffuse = diff * I;
    vec3 result = (ambient + diffuse) * color.xyz;
    //out_color = vec4(result, 1.0);
//  vec3 diffuse = I * max(0.f, dot(vec3(v_n), l));
//  vec3 specular = I * pow(max(0.f, dot(normalize(vec3(v_n)), h)), p);
//  out_color = vec4((ambient + diffuse + specular) * vec3(color), 1);
    out_color = color * vec4(1, 1, 1, alpha);
  }
}