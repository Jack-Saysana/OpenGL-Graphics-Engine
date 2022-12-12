#include <stdlib.h>
#include <stdio.h>
#include <cglm/mat4.h>

#define NUM_PROPS (5)

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

typedef struct chain_queue {
  K_CHAIN **buffer;
  size_t queue_size;
  size_t queue_len;
} C_QUEUE;

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
  unsigned int num_indicies;
} MODEL;

C_QUEUE *begin_animation(ANIMATION *anim);
int enqueue_chain(C_QUEUE *queue, K_CHAIN *chain);
K_CHAIN *dequeue_chain(C_QUEUE *queue);
void calc_bone_mats(MODEL *model, mat4 *bone_mats, unsigned int bone_id,
                    unsigned int frame, KEYFRAME *prev, KEYFRAME *next);
void free_queue(C_QUEUE *queue);
void free_animations(ANIMATION *animations, size_t a_len);
int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
