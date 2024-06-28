#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <GLFW/glfw3.h>
#include <const.h>
#include <globals.h>
#include <structs/simulation_str.h>

typedef struct check_args {
  pthread_mutex_t *col_lock;
  size_t start;
  size_t end;
  SIMULATION *sim;
  COLLISION **collisions;
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
                            size_t collider_offset, COLLISION **col,
                            size_t *col_buf_len, size_t *col_buf_size,
                            int get_col_info, pthread_mutex_t *col_lock);
void global_collider(ENTITY *, size_t, COLLIDER *dest);
void integrate_ent(ENTITY *);
int ledger_init(SIM_ITEM **, size_t **, size_t *, size_t *, size_t *);
int ledger_add(SIM_ITEM **, size_t **, size_t *, size_t *, size_t *,
               LEDGER_INPUT, int);
size_t ledger_search(SIM_ITEM *, size_t, LEDGER_INPUT, int);
void ledger_delete(SIM_ITEM *, size_t *, size_t, size_t *, LEDGER_INPUT, int);
void ledger_delete_direct(SIM_ITEM *, size_t *, size_t *, size_t, int);
int resize_ledger(SIM_ITEM **, size_t *, size_t *, size_t, int);
int entity_in_range(SIMULATION *, ENTITY *, vec3, float);

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
int featherstone_abm(ENTITY *ent, vec3 grav);
void apply_constraints(ENTITY *ent, J_CONS *cons, size_t num_constr,
                       vec3 gravity);
void collision_point(COLLIDER *a, COLLIDER *b, vec3 p_vec, vec3 dest);
void solve_collision(COL_ARGS *a_args, COL_ARGS *b_args, vec3 p_dir,
                     vec3 p_loc, vec3 gravity);
//void calc_inertia_tensor(ENTITY *ent, size_t col_offset, COLLIDER *collider,
//                         float inv_mass, mat4 dest);
void calc_inertia_tensor(ENTITY *ent, size_t col_offset, float inv_mass,
                         mat4 dest);
int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
int max_dot(vec3 *verts, unsigned int len, vec3 dir);
void vec3_remove_noise(vec3 vec, float threshold);
float remove_noise(float val, float threshold);

// ================================= MACROS ==================================

#define is_moving(e,c) (fabs((e)->np_data[(c)].vel_angle) > ZERO_THRESHOLD)
