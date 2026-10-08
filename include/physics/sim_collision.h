#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <const.h>
#include <globals.h>
#include <cglm/cglm.h>
#include <structs/simulation_str.h>
#include <structs/sim_ledger_str.h>
#include <structs/col_check_args_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

void *check_moving_buffer(void *args);
int get_collider_collisions(SIMULATION *sim, ENTITY *subject,
                            size_t collider_offset, COL_UPDATE **col,
                            size_t *col_buf_len, size_t *col_buf_size,
                            int get_col_info, pthread_mutex_t *col_lock);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *hit_box);
int propagate_new_mcol(SIMULATION *sim, ENTITY *ent, size_t col);
void propagate_rm_mcol(SIMULATION *sim, ENTITY *ent, size_t col);
void propagate_rm_ment(SIMULATION *sim, ENTITY *ent);

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

void global_collider(ENTITY *ent, size_t, COLLIDER *dest);

