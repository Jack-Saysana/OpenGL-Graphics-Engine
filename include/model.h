#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_PROPS (5)

typedef enum {
  AMB = 0,
  DIFF = 1,
  SPEC = 2,
  SPEC_EXPONENT = 3,
  BUMP = 4
} TEX_TYPE;

typedef enum chain_type {
  LOCATION = 0,
  ROTATION = 1,
  SCALE = 2
} C_TYPE;

typedef struct keyframe {
  float offset[4];
  unsigned int frame;
} KEYFRAME;

typedef struct keyframe_chain {
  KEYFRAME *chain;
  size_t num_frames;
  unsigned int b_id;
  C_TYPE type;
  unsigned int start_frame;
} K_CHAIN;

typedef struct animation {
  K_CHAIN *keyframe_chains;
  size_t num_chains;
} ANIMATION;

typedef struct bone {
  float coords[3];
  int parent;
  int num_children;
} BONE;

typedef struct model {
  ANIMATION *animations;
  BONE *bones;
  size_t num_animations;
  size_t num_bones;
  unsigned int textures[NUM_PROPS];
  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
  unsigned int num_indicies;
} MODEL;

void draw_model(unsigned int shader, MODEL *model);
void draw_bones(MODEL *model);
void free_animations(ANIMATION *animations, size_t a_len);
void free_model(MODEL *model);
