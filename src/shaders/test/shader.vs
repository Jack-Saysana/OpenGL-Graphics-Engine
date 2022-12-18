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

uniform BONE bones[9];
uniform mat4 bone_mats[9][3];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//out vec4 fragPos;
out vec2 texCoords;
//out vec3 normal;
//out vec3 viewPos;

mat4 hierarchy_transform(int id);
vec4 get_bone_transformation();

void main() {
  vec4 bone_transformation = get_bone_transformation();

  gl_Position = projection * view * model * bone_transformation;

  //fragPos = model * vec4(inPos.x, inPos.y, inPos.z, 1.0);
  texCoords = in_tex_coords;
  //normal = inNormal;
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
    transformation = bone_mats[cur][ROTATION] * bone_mats[cur][LOCATION] *
                     bone_mats[cur][SCALE] * transformation;
    cur = bones[cur].parent;
  }

  return transformation;
}
