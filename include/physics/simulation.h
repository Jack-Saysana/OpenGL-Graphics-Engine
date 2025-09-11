#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <GLFW/glfw3.h>
#include <const.h>
#include <globals.h>
#include <structs/2d/models/entity_2d_str.h>
#include <structs/simulation_str.h>
#include <structs/sim_ledger_str.h>

typedef struct collision_update {
  COLLISION col;
  void (*move_cb)(ENTITY *, vec3);
  int (*is_moving_cb)(ENTITY *, size_t);
} COL_UPDATE;

typedef struct check_args {
  pthread_mutex_t *col_lock;
  size_t start;
  size_t end;
  SIMULATION *sim;
  COL_UPDATE **collisions;
  size_t *buf_len;
  size_t *buf_size;
  vec3 origin;
  float range;
  int get_col_info;
} C_ARGS;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

int elist_add(SIM_COLLIDER **list, size_t *len, size_t *buff_size,
              ENTITY *entity, size_t collider_offset);
void elist_delete(SIM_COLLIDER *list, size_t index, size_t *len);
void *check_moving_buffer(void *args);
int get_collider_collisions(SIMULATION *sim, ENTITY *subject,
                            size_t collider_offset, COL_UPDATE **col,
                            size_t *col_buf_len, size_t *col_buf_size,
                            int get_col_info, pthread_mutex_t *col_lock);
int propagate_new_mcol(SIMULATION *sim, ENTITY *ent, size_t col);
void propagate_rm_mcol(SIMULATION *sim, ENTITY *ent, size_t col);
void propagate_rm_ment(SIMULATION *sim, ENTITY *ent);
void global_collider(ENTITY *ent, size_t, COLLIDER *dest);
int entity_in_range(SIMULATION *, ENTITY *, vec3, float);
void free_sim_state(SIM_STATE *, size_t);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

OCT_TREE *init_tree(float max_extent, unsigned int max_depth);
void free_oct_tree(OCT_TREE *tree);
#ifdef DEBUG_OCT_TREE
int oct_tree_insert(OCT_TREE *tree, ENTITY *entity, size_t collider_offset,
                    int birthmark);
#else
int oct_tree_insert(OCT_TREE *tree, ENTITY *entity, size_t collider_offset);
#endif
int oct_tree_delete(OCT_TREE *tree, ENTITY *entity, size_t collider_offset);
COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *hit_box);
size_t get_all_colliders(OCT_TREE *tree, PHYS_OBJ **dest);

int collision_check(COLLIDER *a, COLLIDER *b, vec3 *simplex);
int epa_response(COLLIDER *a, COLLIDER *b, vec3 *simplex, vec3 p_dir,
                 float *p_depth);
void collision_point(COLLIDER *a, COLLIDER *b, vec3 p_vec, vec3 dest);
int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
int max_dot(vec3 *verts, unsigned int len, vec3 dir);
void vec3_remove_noise(vec3 vec, float threshold);

int ledger_init(SIM_LEDGER *ledger);
int ledger_add(SIM_LEDGER *ledger, LEDGER_INPUT l_data, int l_type);
size_t ledger_search(SIM_LEDGER *ledger, LEDGER_INPUT l_data, int l_type);
void ledger_delete(SIM_LEDGER *ledger, LEDGER_INPUT l_data, int l_type);
void ledger_delete_direct(SIM_LEDGER *ledger, size_t index, int l_type);
void free_ledger(SIM_LEDGER *ledger);

