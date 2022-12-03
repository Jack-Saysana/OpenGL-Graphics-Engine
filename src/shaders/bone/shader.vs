#version 460 core

layout (location = 0) in vec3 inPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * model * vec4(inPos.x, inPos.y, inPos.z, 1.0);
}
