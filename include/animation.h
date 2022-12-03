#include <stdlib.h>
#include <stdio.h>

typedef enum keyframe_type {
  LOCATION = 0,
  ROTATION = 1,
  SCALE = 2
} KEY_TYPE;

typedef struct frame {
  struct frame *next;
  float offset[4];
  unsigned int time;
} FRAME;

typedef struct anim_bone {
  unsigned int id;
  FRAME *attrib_chains[3];
  FRAME *tails[3];
} ANIM_BONE;

typedef struct animation {
  ANIM_BONE *bones;
  size_t num_bones;
} ANIMATION;
