#include <stdlib.h>
#include <stdio.h>

typedef enum chain_type {
  LOCATION = 0,
  ROTATION = 1,
  SCALE = 2
} C_TYPE;

typedef struct animation {
  K_CHAIN *keyframe_chains;
  size_t num_chains;
} ANIMATION;

typedef struct keyframe_chain {
  KEYFRAME *chain;
  size_t num_frames;
  unsigned int b_id;
  C_TYPE type;
  unsigned int start_frame;
} K_CHAIN;

typedef struct keyframe {
  float offset[4];
  unsigned int frame;
} KEYFRAME;

void free_animations(ANIMATION *animations, size_t a_len);
