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

uniform BONE bones[26];
uniform mat4 bone_mats[26][3];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 test_col;

mat4 parent_transformation();
int equiv_mats(mat4 a, mat4 b);

mat4 reflect = mat4(vec4(1.0, 0.0, 0.0, 0.0),
                    vec4(0.0, 1.0, 0.0, 0.0),
                    vec4(0.0, 0.0, -1.0, 0.0),
                    vec4(0.0, 0.0, 0.0, 1.0));

void main() {
  mat4 transformation = bone_mats[bone_id][LOCATION] /** bone_mats[bone_id][ROTATION]*/ *
                        bone_mats[bone_id][SCALE];
  transformation = parent_transformation() * transformation;

  mat4 idy = mat4(vec4(1.0, 0.0, 0.0, 0.0),
                  vec4(0.0, 1.0, 0.0, 0.0),
                  vec4(0.0, 0.0, 1.0, 0.0),
                  vec4(0.0, 0.0, 0.0, 1.0));

  mat4 zr = mat4(vec4(0.0, 0.0, 0.0, 0.0),
                 vec4(0.0, 0.0, 0.0, 0.0),
                 vec4(0.0, 0.0, 0.0, 0.0),
                 vec4(0.0, 0.0, 0.0, 0.0));

  test_col = vec3(0.0, 0.0, 1.0);

  gl_Position = projection * view * model * transformation * reflect * vec4(in_pos, 1.0);
}

int equiv_mats(mat4 a, mat4 b) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (a[i][j] != b[i][j]) {
        return -1;
      }
    }
  }

  return 0;
}

mat4 parent_transformation() {
  int cur = bones[bone_id].parent;

  mat4 transformation = mat4(vec4(1.0, 0.0, 0.0, 0.0),
                             vec4(0.0, 1.0, 0.0, 0.0),
                             vec4(0.0, 0.0, 1.0, 0.0),
                             vec4(0.0, 0.0, 0.0, 1.0));
  while (cur != -1) {
    vec4 ref_coords = reflect * vec4(bones[cur].coords, 1.0);
    mat4 to_parent = mat4(vec4(1.0, 0.0, 0.0, 0.0),
                          vec4(0.0, 1.0, 0.0, 0.0),
                          vec4(0.0, 0.0, 1.0, 0.0),
                          vec4(ref_coords.x, ref_coords.y, ref_coords.z, 1.0));
    mat4 from_parent = mat4(vec4(1.0, 0.0, 0.0, 0.0),
                            vec4(0.0, 1.0, 0.0, 0.0),
                            vec4(0.0, 0.0, 1.0, 0.0),
                            vec4(-ref_coords.x, -ref_coords.y, -ref_coords.z, 1.0));

    transformation = to_parent * bone_mats[cur][ROTATION] * from_parent *
                     bone_mats[cur][LOCATION] * bone_mats[cur][SCALE] * transformation;
    cur = bones[cur].parent;
  }

  return transformation;
}
