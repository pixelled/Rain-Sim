#version 330


uniform vec3 u_cam_pos;
uniform vec2 u_texture_5_size;
uniform vec2 u_texture_3_size;

uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;
uniform vec4 u_color;

uniform float avg_color;

uniform samplerCube u_texture_cubemap;
uniform sampler2D u_texture_5;
uniform sampler2D u_texture_3;

in vec4 v_position;
in vec4 v_normal;
in vec4 v_tangent;
in vec2 v_uv;

out vec4 out_color;

void main() {
    vec3 n = normalize(vec3(v_normal));
    vec3 t = normalize(vec3(v_tangent));
    mat3 TBN = mat3(t, cross(n, t), n);
    float curr_h = texture(u_texture_5, v_uv).r;

    const float height_scaling = 50.0;
    vec3 color = vec3(0.3f, 0.3f, 0.3f);

    float height_cutoff = min(1.f, 0.3f + 0.75f * avg_color);
//    float height_cutoff = 0.5f;
    float wetness_raw = (texture(u_texture_3, v_uv)).r;
    float wetness = curr_h < height_cutoff ? wetness_raw * 1.2 : 0;

    float d_u = (texture(u_texture_5, v_uv + vec2(1.0 / u_texture_5_size.x, 0.0)).r - curr_h) * height_scaling;
    float d_v = (texture(u_texture_5, v_uv + vec2(0.0, 1.0 / u_texture_5_size.y)).r - curr_h) * height_scaling;
    vec3 nd = normalize(TBN * vec3(-d_u * max(1 - wetness * 2, 0), max(0, -d_v * max(1 - wetness * 2, 0)), 1));

    const int p = 80;
    vec3 l = u_light_pos - vec3(v_position);
    vec3 v = normalize(u_cam_pos - vec3(v_position));
    float r2 = dot(l, l);
    l = l / sqrt(r2);
    vec3 h = normalize(v + l);
    vec3 I = u_light_intensity / r2;

    const float diffuse_brightness = 2.0;
    const float specular_brightness = 2.0;
    const float base_wetness = 0.1;

    const vec3 ambient = 0.1 * vec3(1, 1, 1);

    const vec3 shadow_center = vec3(3.5, 0, 5);
    const float shadow_radius = 1.2;
    float shadow_factor = max(0, 1 - (max(shadow_radius - length(vec3(v_position) - shadow_center), 0) / shadow_radius));

    I *= shadow_factor;
    vec3 diffuse = I * vec3(color) * max(0.f, dot(nd, l)) * (1 - wetness - base_wetness) * diffuse_brightness;
    vec3 specular = I * pow(max(0.f, dot(nd, h)), p) * (wetness_raw + base_wetness) * specular_brightness;

    vec3 wo = normalize(u_cam_pos - vec3(v_position));
    vec3 environment = vec3(texture(u_texture_cubemap, vec3(-wo.x, wo.y, -wo.z))) * wetness * 1.2;

    float alpha = max(0, 1 - wetness) + (specular.r + environment.r) * 0.2 + 0.15;

    out_color = vec4(ambient + diffuse + specular + environment, alpha);
}
