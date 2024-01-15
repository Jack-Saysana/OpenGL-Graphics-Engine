#version 430 core

in vec2 tex_coords;

out vec4 frag_col;

uniform sampler2D font_map;

void main() {
  vec4 tex_col = texture(font_map, tex_coords);
  if (tex_col.r > 0.1) {
    frag_col = vec4(tex_col.r);
  } else {
    discard;
  }
}
