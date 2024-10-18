#ifndef __MODEL_STR_H__
#define __MODEL_STR_H__

#include <structs/models/animation_str.h>
#include <structs/models/bone_str.h>
#include <structs/models/collider_str.h>

typedef struct model {
  ANIMATION *animations;
  // Buffer containing each keyframe "chain" that corresponds to a bone and
  // type of animation (Scale, rotation, translation)
  K_CHAIN *k_chain_block;
  // Buffer containing the keyframes that make-up keyframe chains
  KEYFRAME *keyframe_block;
  // Buffer containing "sleds" which define, for each frame of animation, the
  // index of the keyframe that corresponds to the previous keyframe. Basically
  // a resource for easy keyframe interpolation
  int *sled_block;
  BONE *bones;
  COLLIDER *colliders;
  // Each element's index corresponds to the collider of the same index.
  // Element value corresponds to root bone represneted by collider
  int *collider_bone_links;
  // Each element's index corresponds to the bone of the same index.
  // Element value corresponds to collider represneted by bone
  int *bone_collider_links;
  size_t num_animations;
  size_t num_bones;
  size_t num_colliders;
  unsigned int textures[NUM_PROPS];
  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
  unsigned int num_indicies;
} MODEL;



#endif
