#include <stdio.h>
#include <float.h>
#include <cglm/vec3.h>
#include <model_str.h>

#define MAX_DEPTH (5)
#define OCT_TREE_STARTING_LEN (25)
#define BUFF_STARTING_LEN (10)

typedef enum {
  LINE = 2,
  TRIANGLE = 3,
  TETRAHEDRON = 4
} SIMPLEX;

typedef enum {
  X_Y_Z = 0,
  X_Y_negZ = 1,
  X_negY_Z = 2,
  X_negY_negZ = 3,
  negX_Y_Z = 4,
  negX_Y_negZ = 5,
  negX_negY_Z = 6,
  negX_negY_negZ = 7,
  MULTIPLE = -1
} OCTANT;

typedef struct entity {
  MODEL *model;
  mat4 (*bone_mats)[3];
  mat4 model_mat;
} ENTITY;

typedef struct physics_object {
  ENTITY *entity;
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

vec3 X = { 1.0, 0.0, 0.0 };
vec3 NEG_X = { -1.0, 0.0, 0.0 };
vec3 Y = { 0.0, 1.0, 0.0 };
vec3 NEG_Y = { 0.0, -1.0, 0.0 };
vec3 Z = { 0.0, 0.0, 1.0 };
vec3 NEG_Z = { 0.0, 0.0, -1.0 };

// FRONT FACING

OCT_TREE *init_tree();
int oct_tree_insert(OCT_TREE *tree, COLLIDER *obj, ENTITY *entity,
                    size_t model_offset);
int oct_tree_delete(OCT_TREE *tree, size_t node_offset, size_t obj_offset);
COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *hit_box);
void free_oct_tree(OCT_TREE *tree);
int collision_check(COLLIDER *a, COLLIDER *b);

// BACK FACING

void triangle_check(vec3 a, vec3 b, vec3 c, unsigned int *num_used);
void support_func(COLLIDER *a, COLLIDER *b, vec3 dir, vec3 dest);
int init_node(OCT_TREE *tree, OCT_NODE *parent);
int read_oct(OCT_TREE *tree, OCT_NODE *node, COLLISION_RES *res);
int read_all_children(OCT_TREE *tree, OCT_NODE *node, COLLISION_RES *res);
int append_list(OCT_TREE *tree, size_t node_offset, COLLIDER *obj,
                ENTITY *entity, size_t model_offset);
OCTANT detect_octant(vec3 min_extent, vec3 max_extent, float *ebj_extents,
                     float *oct_len);
int max_dot(COLLIDER *a, vec3 dir);
int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
