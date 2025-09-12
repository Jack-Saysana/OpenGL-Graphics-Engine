#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <glad/glad.h>
#include <float.h>
#include <cglm/affine.h>
#include <const.h>
#include <structs/models/entity_str.h>
#include <structs/oct_tree_str.h>

static vec3 X_DIR = { 1.0, 0.0, 0.0 };
static vec3 NEG_X_DIR = { -1.0, 0.0, 0.0 };
static vec3 Y_DIR = { 0.0, 1.0, 0.0 };
static vec3 NEG_Y_DIR = { 0.0, -1.0, 0.0 };
static vec3 Z_DIR = { 0.0, 0.0, 1.0 };
static vec3 NEG_Z_DIR = { 0.0, 0.0, -1.0 };

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

static int init_node(OCT_TREE *tree, OCT_NODE *parent);
COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *col);
static int read_oct(OCT_TREE *tree, OCT_NODE *node, COLLISION_RES *res);
#ifdef DEBUG_OCT_TREE
static int append_buffer(OCT_TREE *tree, size_t node_offset, ENTITY *entity,
                         size_t collider_offset, int birthmark, COLLIDER col);
#else
static int append_buffer(OCT_TREE *tree, size_t node_offset, ENTITY *entity,
                         size_t collider_offset);
#endif
static int add_to_list(OCT_TREE *tree, size_t obj_offset, size_t node_offset);
static int remove_from_list(OCT_TREE *tree, size_t obj_offset);
//OCTANT detect_octant(vec3 min_extent, vec3 max_extent, float *ebj_extents,
//                     float *oct_len);
static size_t update_extents(int, vec3, vec3, float);
static int detect_octant(vec3 min_extent, vec3 max_extent, float *obj_extents,
                         float oct_len);
static void update_node_emptiness(OCT_TREE *tree, size_t node_offset);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
int max_dot(vec3 *verts, unsigned int len, vec3 dir);
void global_collider(ENTITY *entity, size_t col, COLLIDER *dest);
void draw_model(unsigned int shader, MODEL *model);
int get_lsb(int val);
