#ifndef __ENGINE_ANIMATION_STR_H__
#define __ENGINE_ANIMATION_STR_H__

#define A_TYPE_SINGLE (0)
#define A_TYPE_BLEND  (1)

typedef struct animation_input {
  union {
    struct {
      unsigned int index;
      unsigned int frame;
    };
    struct {
      unsigned int a1;
      unsigned int a2;
      unsigned int f1;
      unsigned int f2;
      float ratio;
    };
  };
  int type;
} A_INPUT;

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

typedef struct animation {
  K_CHAIN *keyframe_chains;
  size_t num_chains;
  size_t duration;
} ANIMATION;

#endif
