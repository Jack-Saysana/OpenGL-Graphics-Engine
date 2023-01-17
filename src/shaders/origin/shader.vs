#version 430 core

layout (location = 0) in vec3 in_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 test_col;

void main() {
  test_col = vec3(0.0, 1.0, 1.0);
  gl_Position = projection * view * model * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
}
