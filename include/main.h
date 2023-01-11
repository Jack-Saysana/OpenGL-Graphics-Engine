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

typedef struct collider {
  vec3 verts[8];
  unsigned int num_used;
} COLLIDER;

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
  mat4 (*bone_mats)[3];
  size_t num_animations;
  size_t num_bones;
  size_t num_colliders;
  unsigned int textures[NUM_PROPS];
  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
  unsigned int num_indicies;
} MODEL;

typedef struct physics_object {
  MODEL *model;
  COLLIDER *collider;
  size_t model_offset;
  size_t node_offset;
  size_t next_offset;
  size_t prev_offset;
} PHYS_OBJ;

typedef struct oct_tree_node {
  size_t head_offset;
  size_t tail_offset;
  int next_offset;
  int empty;
} OCT_NODE;

typedef struct oct_tree {
  OCT_NODE *node_buffer;
  PHYS_OBJ *data_buffer;
  size_t node_buff_len;
  size_t node_buff_size;
  size_t data_buff_len;
  size_t data_buff_size;
} OCT_TREE;

typedef struct collision_result {
  PHYS_OBJ **list;
  size_t list_len;
  size_t list_buff_size;
} COLLISION_RES;

void keyboard_input(GLFWwindow *window);
void mouse_input(GLFWwindow *widnow, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow *, int, int);
void draw_oct_tree(MODEL *cube, OCT_TREE *tree, vec3 pos, float scale,
                   unsigned int shader, size_t offset, int depth);

unsigned int init_shader_prog(char *, char *, char *);
MODEL *load_model(char *path);
int animate(MODEL *model, unsigned int animation_index, unsigned int frame);
void draw_bones(MODEL *model);
void draw_model(unsigned int shader, MODEL *model);
void free_model(MODEL *model);



OCT_TREE *init_tree();
int oct_tree_insert(OCT_TREE *tree, COLLIDER *obj, MODEL *model,
                    size_t model_offset);
int oct_tree_delete(OCT_TREE *tree, size_t node_offset, size_t obj_offset);
COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *hit_box);
void free_oct_tree(OCT_TREE *tree);
int collision_check(COLLIDER *a, COLLIDER *b);
