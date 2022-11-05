#version 460 core

out vec4 FragColor;
//in vec4 fragPos;
in vec2 texCoords;
//in vec3 normal;
//in vec3 viewPos;

uniform struct Material {
  sampler2D diff_map;
  sampler2D spec_map;
  int shine;
} material;

uniform mat4 projection;

void main() {
  FragColor = texture(material.diff_map, texCoords);
  //FragColor = vec4(texCoords.x, texCoords.y, 0.0, 1.0);
}
