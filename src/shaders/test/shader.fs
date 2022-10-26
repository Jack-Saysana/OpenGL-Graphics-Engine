#version 460 core

out vec4 FragColor;
/*in vec4 fragPos;
in vec2 texCoords;
in vec3 normal;
in vec3 viewPos;*/

uniform mat4 projection;

void main() {
  FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
