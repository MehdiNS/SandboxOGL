#version 410
in vec2 texture_coordinates;
out vec4 frag_color;
uniform sampler2D basic_texture;

void main () {
  vec4 texel = texture(basic_texture, texture_coordinates);
  frag_color = vec4 (0.5, 0.0, 0.5, 1.0);
}
