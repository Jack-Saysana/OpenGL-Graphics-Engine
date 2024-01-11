#version 430 core

in vec2 tex_coords;

out vec4 frag_col;

uniform struct Material {
  sampler2D amb_map;
} material;

void main() {
  frag_col = vec4(1.0, 1.0, 1.0, 1.0);//texture(material.amb_map, tex_coords);
}
