#include <stdlib.h>
#include <stdio.h>
#include <cglm/mat4.h>
#include <cglm/quat.h>
#include <cglm/affine.h>

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

C_QUEUE *begin_animation(ANIMATION *anim);
int enqueue_chain(C_QUEUE *queue, K_CHAIN *chain);
K_CHAIN *dequeue_chain(C_QUEUE *queue);
void calc_bone_mats(mat4 (*bone_mats)[3], unsigned int bone_id, C_TYPE type,
                    unsigned int frame, KEYFRAME *prev, KEYFRAME *next);
void free_queue(C_QUEUE *queue);
void free_animations(ANIMATION *animations, size_t a_len);
int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
