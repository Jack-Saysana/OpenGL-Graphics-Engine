#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GLFW/glfw3.h>
#include <const.h>
#include <globals.h>
#include <simulation_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

int elist_add(SIM_COLLIDER **list, size_t *len, size_t *buff_size,
              ENTITY *entity, size_t collider_offset);
void elist_delete(SIM_COLLIDER *list, size_t index, size_t *len);
void integrate_collider(ENTITY *entity, size_t offset, vec3 force);
int get_collider_collisions(SIMULATION *sim, ENTITY *subject,
                            size_t collider_offset, COLLISION **col,
                            size_t *col_buf_len, size_t *col_buf_size);
void get_collider_velocity(ENTITY *entity, size_t collider_offset, vec3 vel,
                           vec3 ang_vel);
int is_moving(vec3 vel, vec3 ang_vel);
void global_collider(mat4 bone_to_entity, mat4 entity_to_world,
                     COLLIDER *source, COLLIDER *dest);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

OCT_TREE *init_tree(float max_extent, unsigned int max_depth);
void free_oct_tree(OCT_TREE *tree);
int oct_tree_insert(OCT_TREE *tree, ENTITY *entity, size_t collider_offset);
int oct_tree_delete(OCT_TREE *tree, ENTITY *entity, size_t collider_offset);
COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *hit_box);
size_t get_all_colliders(OCT_TREE *tree, PHYS_OBJ **dest);

int collision_check(COLLIDER *a, COLLIDER *b, vec3 *simplex);
int epa_response(COLLIDER *a, COLLIDER *b, vec3 *simplex, vec3 p_dir,
                 float *p_depth);
void collision_point(COLLIDER *a, COLLIDER *b, vec3 p_vec, vec3 dest);
void solve_collision(COL_ARGS *a_args, COL_ARGS *b_args, vec3 p_dir,
                     vec3 p_loc);
void calc_inertia_tensor(ENTITY *ent, size_t col_offset, COLLIDER *collider,
                         float inv_mass, mat4 dest);

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
void get_model_mat(ENTITY *entity, mat4 model);
int max_dot(vec3 *verts, unsigned int len, vec3 dir);
void vec3_remove_noise(vec3 vec, float threshold);
