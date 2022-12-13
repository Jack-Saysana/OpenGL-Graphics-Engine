#version 460 core

#define LOCATION (0)
#define ROTATION (1)
#define SCALE (2)

layout (location = 0) in vec3 in_pos;
layout (location = 1) in int bone_id;

struct BONE {
  float coords[3];
  int parent;
};

uniform BONE bones[9];
uniform mat4 bone_mats[3][9];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  mat4 transformation = bone_mats[bone_id][LOCATION] * bone_mats[bone_id][ROTATION] * bone_mats[bone_id][SCALE];
  gl_Position = projection * view * model * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
}
