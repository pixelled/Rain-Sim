#version 330

in vec2 v_uv;
out vec2 in_uv;

// This is where the final pixel color is output.
// Here, we are only interested in the first 3 dimensions (xyz).
// The 4th entry in this vector is for "alpha blending" which we
// do not require you to know about. For now, just set the alpha
// to 1.
out vec4 out_color;

void main() {
  in_uv = v_uv;
  gl_Position = vec4(2 * v_uv - 1, -1, 1);
}
