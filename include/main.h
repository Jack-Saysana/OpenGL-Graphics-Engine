#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <math.h>
#include <stdlib.h>

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

typedef struct bone {
  float coords[3];
  int parent;
  int num_children;
} BONE;

typedef struct chain_queue {
  K_CHAIN **buffer;
  size_t queue_size;
  size_t queue_len;
} C_QUEUE;

typedef struct model {
  ANIMATION *animations;
  BONE *bones;
  size_t num_animations;
  size_t num_bones;
  unsigned int textures[NUM_PROPS];
  unsigned int VAO;
  unsigned int num_indicies;
} MODEL;

void framebuffer_size_callback(GLFWwindow *, int, int);
unsigned int init_shader_prog(char *, char *, char *);
MODEL *load_model(char *path);
C_QUEUE *begin_animation(ANIMATION *anim);
K_CHAIN *dequeue_chain(C_QUEUE *queue);
void calc_bone_mats(mat4 (*bone_mats)[3], unsigned int bone_id, C_TYPE type,
                    unsigned int frame, KEYFRAME *prev, KEYFRAME *next);
void draw_bones(MODEL *model);
void draw_model(unsigned int shader, MODEL *model);
void free_model(MODEL *model);
void free_queue(C_QUEUE *queue);
