#version 460 core

out vec4 FragColor;

uniform vec3 test_col;

void main() {
  FragColor = vec4(test_col, 1.0);
}
