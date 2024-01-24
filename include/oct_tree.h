#include <stdio.h>
#include <float.h>
#include <cglm/affine.h>
#include <const.h>
#include <entity_str.h>
#include <oct_tree_str.h>

static vec3 X_DIR = { 1.0, 0.0, 0.0 };
static vec3 NEG_X_DIR = { -1.0, 0.0, 0.0 };
static vec3 Y_DIR = { 0.0, 1.0, 0.0 };
static vec3 NEG_Y_DIR = { 0.0, -1.0, 0.0 };
static vec3 Z_DIR = { 0.0, 0.0, 1.0 };
static vec3 NEG_Z_DIR = { 0.0, 0.0, -1.0 };

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

int init_node(OCT_TREE *tree, OCT_NODE *parent);
COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *col);
int read_oct(OCT_TREE *tree, OCT_NODE *node, COLLISION_RES *res);
int read_all_children(OCT_TREE *tree, OCT_NODE *node, COLLISION_RES *res);
int append_buffer(OCT_TREE *tree, size_t node_offset, ENTITY *entity,
                  size_t collider_offset);
int add_to_list(OCT_TREE *tree, size_t obj_offset, size_t node_offset);
int remove_from_list(OCT_TREE *tree, size_t obj_offset);
OCTANT detect_octant(vec3 min_extent, vec3 max_extent, float *ebj_extents,
                     float *oct_len);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
void get_model_mat(ENTITY *entity, mat4 model);
int max_dot(vec3 *verts, unsigned int len, vec3 dir);
void global_collider(mat4 bone_to_entity, mat4 entity_to_world,
                     COLLIDER *source, COLLIDER *dest);
