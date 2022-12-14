#version 460 core

in vec3 test_col;

out vec4 FragColor;

uniform vec4 col;

void main() {
  FragColor = vec4(test_col, 1.0);
}
