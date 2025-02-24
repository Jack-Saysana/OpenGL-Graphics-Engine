#ifndef __ENGINE_MODEL_DATA_STR_H__
#define __ENGINE_MODEL_DATA_STR_H__

#include "entity_str.h"

typedef struct vbo {
  float vertex[3];
  float normal[3];
  float tex_coord[2];
  int bone_ids[4];
  float weights[4];
} VBO;

typedef struct model_data {
  ANIMATION *animations;
  K_CHAIN *k_chain_block;
  KEYFRAME *keyframe_block;
  int *sled_block;
  BONE *bones;
  int *bone_collider_links;
  COLLIDER *colliders;
  int *collider_bone_links;
  char *mat_paths[NUM_PROPS];
  VBO *vertices;
  int *indices;
  size_t num_animations;
  size_t num_bones;
  size_t num_colliders;
  size_t num_indices;
  size_t num_vertices;
} MODEL_DATA;

#endif
