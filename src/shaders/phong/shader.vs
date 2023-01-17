#version 430 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;
layout (location = 3) in ivec4 in_bone_ids;
layout (location = 4) in vec4 in_weights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPosition;

out vec4 fragPos;
out vec2 texCoords;
out vec3 normal;
out vec3 viewPos;

void main() {
  gl_Position = projection * view * model *
                vec4(in_pos, 1.0);
  fragPos = model * vec4(in_pos, 1.0);
  texCoords = in_tex_coords;
  normal = in_normal;
  viewPos = viewPos;
}
