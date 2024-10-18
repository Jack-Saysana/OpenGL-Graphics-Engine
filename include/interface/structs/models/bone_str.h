#ifndef __ENGINE_BONE_STR_H__
#define __ENGINE_BONE_STR_H__

typedef struct bone {
  // Rotates vectors in bone space to entity space
  mat3 coordinate_matrix;
  // in entity space
  vec3 head;
  vec3 base;
  int parent;
  int num_children;
} BONE;

#endif
