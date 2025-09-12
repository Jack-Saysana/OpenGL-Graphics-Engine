#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <cglm/cglm.h>
#include <structs/2d/models/entity_2d_str.h>
#include <structs/simulation_str.h>
#include <structs/sim_ledger_str.h>

typedef struct collision_update_2d {
  COLLISION_2D col;
  void (*move_cb)(ENTITY_2D *, vec3);
  int (*is_moving_cb)(ENTITY_2D *, size_t);
} COL_UPDATE_2D;

typedef struct check_args_2d {
  pthread_mutex_t *col_lock;
  size_t start;
  size_t end;
  SIMULATION *sim;
  COL_UPDATE_2D **collisions;
  size_t *buf_len;
  size_t *buf_size;
  vec2 origin;
  float range;
} C_ARGS_2D;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

int is_moving_2d(ENTITY_2D *ent, size_t col);
int integrate_ent_2d(ENTITY_2D *ent, vec2 forces);
size_t sim_get_nearby_2d(SIMULATION *sim, COLLISION_2D **dest, vec2 pos,
                         float range, int get_col_info);
void *check_moving_buffer_2d(void *args);
int get_collider_collisions_2d(SIMULATION *sim, ENTITY_2D *subject,
                               size_t collider_offset, COL_UPDATE_2D **col,
                               size_t *col_buf_len, size_t *col_buf_size,
                               pthread_mutex_t *col_lock);
int entity_in_range_2d(SIMULATION *sim, ENTITY_2D *ent, vec2 origin,
                       float range);
int propagate_new_mcol_2d(SIMULATION *sim, ENTITY_2D *ent, size_t col);
void propagate_rm_mcol_2d(SIMULATION *sim, ENTITY_2D *ent, size_t col);
void propagate_rm_ment_2d(SIMULATION *sim, ENTITY_2D *ent);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int collision_check_2d(COLLIDER_2D *a, COLLIDER_2D *b, vec2 correction);

QUAD_TREE *init_quad_tree(float max_extent, unsigned int max_depth);
void free_quad_tree(QUAD_TREE *tree);
int quad_tree_insert(QUAD_TREE *tree, ENTITY_2D *entity,
                     size_t collider_offset);
int quad_tree_delete(QUAD_TREE *tree, ENTITY_2D *entity,
                     size_t collider_offset);
COLLISION_RES_2D quad_tree_search(QUAD_TREE *tree, COLLIDER_2D *col);
size_t get_all_quad_colliders(QUAD_TREE *tree, PHYS_OBJ_2D **dest);

int ledger_init(SIM_LEDGER *ledger);
int ledger_add(SIM_LEDGER *ledger, LEDGER_INPUT l_data, int l_type);
size_t ledger_search(SIM_LEDGER *ledger, LEDGER_INPUT l_data, int l_type);
void ledger_delete(SIM_LEDGER *ledger, LEDGER_INPUT l_data, int l_type);
void ledger_delete_direct(SIM_LEDGER *ledger, size_t index, int l_type);
void free_ledger(SIM_LEDGER *ledger);

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
