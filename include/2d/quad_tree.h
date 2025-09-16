#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <cglm/cglm.h>
#include <const.h>
#include <structs/2d/quad_tree_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

static int init_quad_node(QUAD_TREE *tree, QUAD_NODE *parent);
static int read_quad(QUAD_TREE *tree, QUAD_NODE *node, COLLISION_RES_2D *res);
static int append_quad_buffer(QUAD_TREE *tree, size_t node_offset,
                              ENTITY_2D *entity, size_t collider_offset);
static int add_to_list(QUAD_TREE *tree, size_t obj_offset,
                          size_t node_offset);
static int remove_from_list(QUAD_TREE *tree, size_t obj_offset);
static int detect_quadrant(vec2 min_extent, vec2 max_extent, float quad_len,
                           COLLIDER_2D *obj);
static size_t update_quad_extents(int quad, vec2 min_extent, vec2 max_extent,
                                  float quad_len);
static void update_node_emptiness(QUAD_TREE *tree, size_t node_offset);
COLLISION_RES_2D quad_tree_search(QUAD_TREE *tree, COLLIDER_2D *col);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
int get_lsb(int val);
int collision_check_2d(COLLIDER_2D *a, COLLIDER_2D *b, vec2 correction);
void set_vec3(char *name, vec3 vec, unsigned int shader);
void draw_square(vec2 center, float half_w, float half_h);
