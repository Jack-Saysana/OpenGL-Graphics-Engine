#version 460

in vec4 fragPos;
in vec2 texCoords;
in vec3 normal;
in vec3 viewPos;

out vec4 FragCol;

void main() {
  FragCol = vec4(1.0, 1.0, 1.0, 1.0);
}
