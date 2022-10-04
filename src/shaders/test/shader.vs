#version 460

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 fragPos;
out vec2 texCoords;
out vec3 normal;
out vec3 viewPos;

void main() {
  gl_Position = projection * view * model *
                vec4(inPos.x, inPos.y, inPos.z, 1.0);
  fragPos = model * vec4(inPos.x, inPos.y, inPos.z, 1.0);
  texCoords = inTexCoords;
  normal = inNormal;
  viewPos = vec3(0.0, 0.0, 0.0);
}
