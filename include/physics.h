#include <stdio.h>
#include <float.h>
#include <cglm/vec3.h>

#define MAX_DEPTH (5);

typedef enum {
  LINE = 2,
  TRIANGLE = 3,
  TETRAHEDRON = 4
} SIMPLEX;

typedef struct collider {
  vec3 verts[8];
  unsigned int num_used;
} COLLIDER;

typedef struct physics_object {
//  MODEL *model;
  COLLIDER collider;
  int offset;
} PHYS_OBJ;

typedef struct oct_tree_node {
  PHYS_OBJ *data;
  size_t data_buff_len;
  size_t data_len;
  int next_offset;
} OCT_NODE;

typedef struct oct_tree {
  OCT_NODE *root;
  OCT_NODE *node_buffer;
  size_t buff_len;
  size_t buff_size;
} OCT_TREE;

typedef struct collision_result {
  PHYS_OBJ *list;
  int list_len;
} COLLISION_RES;

//int oct_tree_insert(OCT_TREE *tree, PHYS_OBJ *obj);
//PHYS_OBJ oct_tree_delete(OCT_TREE *tree, PHYS_OBJ *obj);
//COLLISION_RES oct_tree_search(OCT_TREE *tree, COL_PLANE *hit_box,
//                              unsigned int num_planes);
int collision_check(COLLIDER *a, COLLIDER *b);
void triangle_check(vec3 a, vec3 b, vec3 c, unsigned int *num_used);
void support_func(COLLIDER *a, COLLIDER *b, vec3 dir, vec3 dest);
