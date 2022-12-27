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

  /*if (equiv_mats(idy, bone_mats[bone_id][0]) == 0) {
    if (bone_id == 0 && bones[bone_id].parent == -1) {
      test_col = vec3(1.0, 0.0, 0.0);
      //test_col = vec3(0.0, 0.0, 0.0);
      //test_col = normalize(bones[bone_id].coords);
    } else if (bone_id == 1 && bones[bone_id].parent == 0) {
      test_col = vec3(0.0, 1.0, 0.0);
      //test_col = vec3(0.0, 0.0, 0.0);
      //test_col = normalize(bones[bone_id].coords);
    } else if (bone_id == 2 && bones[bone_id].parent == 1) {
      test_col = vec3(0.0, 0.0, 1.0);
      //test_col = vec3(0.0, 0.0, 0.0);
      //test_col = normalize(bones[bone_id].coords);
    } else if (bone_id == 3 && bones[bone_id].parent == 2) {
      test_col = normalize(vec3(1.0, 1.0, 0.0));
      //test_col = vec3(0.0, 0.0, 0.0);
      //test_col = normalize(bones[bone_id].coords);
    } else if (bone_id == 4 && bones[bone_id].parent == 1) {
      test_col = normalize(vec3(0.0, 1.0, 1.0));
      //test_col = vec3(0.0, 0.0, 0.0);
      //test_col = normalize(bones[bone_id].coords);
    } else if (bone_id == 5 && bones[bone_id].parent == 4) {
      test_col = normalize(vec3(1.0, 0.0, 1.0));
      //test_col = vec3(0.0, 0.0, 0.0);
      //test_col = normalize(bones[bone_id].coords);
    } else if (bone_id == 6 && bones[bone_id].parent == 5) {
      test_col = normalize(vec3(1.0, 0.5, 0.5));
      //test_col = vec3(0.0, 0.0, 0.0);
      //test_col = normalize(bones[bone_id].coords);
    } else if (bone_id == 7 && bones[bone_id].parent == 5) {
      test_col = normalize(vec3(0.5, 0.5, 1.0));
      //test_col = vec3(0.0, 0.0, 0.0);
      //test_col = normalize(bones[bone_id].coords);
    } else if (bone_id == 8 && bones[bone_id].parent == 1) {
      test_col = normalize(vec3(0.5, 1.0, 0.5));
      //test_col = vec3(0.0, 0.0, 0.0);
      //test_col = normalize(bones[bone_id].coords);
    } else {
      test_col = vec3(0.0, 0.0, 0.0);
    }
  } else {
    test_col = vec3(0.0, 0.0, 0.0);
  }*/
  /*if (bone_id == 8 && bones[bone_id].parent == 1) {
    test_col = vec3(1.0, 0.0, 0.0);
  } else {
    test_col = vec3(0.0, 0.0, 0.0);
  }*/

  /*if (bone_id == 0) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 1) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 2) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 3) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 4) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 5) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 6) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 7) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 8) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 9) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 10) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 11) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 12) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 13) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 14) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 15) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 16) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 17) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 18) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 19) {
    test_col = vec3(1.0, 0.0, 0.0);
  } else if (bone_id == 20) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 21) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 22) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 23) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 24) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else if (bone_id == 25) {
    test_col = vec3(0.0, 0.0, 0.0);
  } else {
    test_col = vec3(0.0, 0.0, 0.0);
  }*/

  if (equiv_mats(idy, bone_mats[bone_id][0]) == 0) {
    test_col = vec3(1.0, 0.0, 0.0);
  } else {
    test_col = vec3(0.0, 0.0, 0.0);
  }

  gl_Position = projection * view * model * transformation * reflect * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
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
    mat4 to_parent = mat4(vec4(1.0, 0.0, 0.0, bones[cur].coords.x),
                          vec4(0.0, 1.0, 0.0, bones[cur].coords.y),
                          vec4(0.0, 0.0, 1.0, bones[cur].coords.z),
                          vec4(0.0, 0.0, 0.0, 1.0));
    mat4 from_parent = mat4(vec4(1.0, 0.0, 0.0, bones[cur].coords.x * -1.0),
                            vec4(0.0, 1.0, 0.0, bones[cur].coords.y * -1.0),
                            vec4(0.0, 0.0, 1.0, bones[cur].coords.z * -1.0),
                            vec4(0.0, 0.0, 0.0, 1.0));

    transformation = to_parent * bone_mats[cur][ROTATION] * from_parent *
                     bone_mats[cur][LOCATION] * bone_mats[cur][SCALE] * transformation;
    cur = bones[cur].parent;
  }

  return transformation;
}
