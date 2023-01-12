#define NUM_PROPS (5)

typedef enum chain_type {
  LOCATION = 0,
  ROTATION = 1,
  SCALE = 2
} C_TYPE;

typedef struct keyframe {
  float offset[4];
  int frame;
} KEYFRAME;

typedef struct keyframe_chain {
  KEYFRAME *chain;
  int *sled;
  size_t num_frames;
  C_TYPE type;
  unsigned int b_id;
} K_CHAIN;

typedef struct collider {
  vec3 verts[8];
  unsigned int num_used;
} COLLIDER;

typedef struct animation {
  K_CHAIN *keyframe_chains;
  size_t num_chains;
  size_t duration;
} ANIMATION;

typedef struct bone {
  float coords[3];
  int parent;
  int num_children;
} BONE;

typedef struct model {
  ANIMATION *animations;
  K_CHAIN *k_chain_block;
  KEYFRAME *keyframe_block;
  int *sled_block;
  BONE *bones;
  COLLIDER *colliders;
  int *collider_bone_links;
  size_t num_animations;
  size_t num_bones;
  size_t num_colliders;
  unsigned int textures[NUM_PROPS];
  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
  unsigned int num_indicies;
  unsigned int ref_count;
} MODEL;

