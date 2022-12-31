#version 460 core

#define LOCATION (0)
#define ROTATION (1)
#define SCALE (2)

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;
layout (location = 3) in ivec4 in_bone_ids;
layout (location = 4) in vec4 in_weights;

struct BONE {
  vec3 coords;
  int parent;
};

uniform BONE bones[26];
uniform mat4 bone_mats[26][3];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 frag_pos;
out vec2 tex_coords;
out vec3 normal;
out vec3 view_pos;

mat4 hierarchy_transform(int id);
vec4 get_bone_transformation();

void main() {
  vec4 bone_transformation = get_bone_transformation();

  gl_Position = projection * view * model * bone_transformation;

  frag_pos = model * bone_transformation;
  tex_coords = in_tex_coords;
  normal = in_normal;
}

vec4 get_bone_transformation() {
  vec4 total = vec4(0.0);

  int i = 0;
  int cur = in_bone_ids[i];
  while (i < 4 && cur != -1) {
    total += (in_weights[i] * hierarchy_transform(cur) * vec4(in_pos, 1.0));
    i++;
    cur = in_bone_ids[i];
  }

  return total;
}

mat4 hierarchy_transform(int id) {
  int cur = id;
  mat4 transformation = mat4(vec4(1.0, 0.0, 0.0, 0.0),
                             vec4(0.0, 1.0, 0.0, 0.0),
                             vec4(0.0, 0.0, 1.0, 0.0),
                             vec4(0.0, 0.0, 0.0, 1.0));

  while (cur != -1) {
    vec4 ref_coords = vec4(bones[cur].coords, 1.0);
    mat4 to_parent = mat4(vec4(1.0, 0.0, 0.0, 0.0),
                          vec4(0.0, 1.0, 0.0, 0.0),
                          vec4(0.0, 0.0, 1.0, 0.0),
                          vec4(ref_coords.x, ref_coords.y, ref_coords.z, 1.0));
    mat4 from_parent = mat4(vec4(1.0, 0.0, 0.0, 0.0),
                            vec4(0.0, 1.0, 0.0, 0.0),
                            vec4(0.0, 0.0, 1.0, 0.0),
                            vec4(-ref_coords.x, -ref_coords.y, -ref_coords.z, 1.0));

    transformation = bone_mats[cur][LOCATION] *
                     to_parent * bone_mats[cur][ROTATION] * from_parent *
                     bone_mats[cur][SCALE] * transformation;
    cur = bones[cur].parent;
  }

  return transformation;
}
