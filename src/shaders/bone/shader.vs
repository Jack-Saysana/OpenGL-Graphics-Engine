#version 460 core

#define LOCATION (0)
#define ROTATION (1)
#define SCALE (2)

layout (location = 0) in vec3 in_pos;
layout (location = 1) in int bone_id;

struct BONE {
  vec3 coords;
  int parent;
};

uniform BONE bones[9];
uniform mat4 bone_mats[3][9];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 test_col;

mat4 parent_transformation();

void main() {
  mat4 transformation = bone_mats[bone_id][LOCATION] * bone_mats[bone_id][ROTATION] * bone_mats[bone_id][SCALE];

  transformation = parent_transformation() * transformation;

  test_col = vec3(0.0, 0.0, 1.0);
  gl_Position = projection * view * model * transformation * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
}

mat4 parent_transformation() {
  int cur = bones[bone_id].parent;

  mat4 transformation = mat4(vec4(1.0, 0.0, 0.0, 0.0),
                             vec4(0.0, 1.0, 0.0, 0.0),
                             vec4(0.0, 0.0, 1.0, 0.0),
                             vec4(0.0, 0.0, 0.0, 1.0));
  vec3 pos_offset;
  mat4 to_parent;
  mat4 from_parent;
  mat4 rotation;

  while (cur != -1) {
    pos_offset = in_pos - bones[cur].coords;

    to_parent = mat4(vec4(1.0, 0.0, 0.0, pos_offset.x),
                     vec4(0.0, 1.0, 0.0, pos_offset.y),
                     vec4(0.0, 0.0, 1.0, pos_offset.z),
                     vec4(0.0, 0.0, 0.0, 1.0));

    from_parent = mat4(vec4(1.0, 0.0, 0.0, -1.0 * pos_offset.x),
                       vec4(0.0, 1.0, 0.0, -1.0 * pos_offset.y),
                       vec4(0.0, 0.0, 1.0, -1.0 * pos_offset.z),
                       vec4(0.0, 0.0, 0.0, 1.0));

    rotation = from_parent * bone_mats[cur][ROTATION] * to_parent;

    transformation = bone_mats[cur][LOCATION] * transformation;
    cur = bones[cur].parent;
  }

  return transformation;
}
