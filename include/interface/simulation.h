#ifndef __ENGINE_SIMULATION_H__
#define __ENGINE_SIMULATION_H__

#include "./structs/simulation_str.h"
#include "./structs/models/entity_str.h"
#include "./structs/2d/models/entity_2d_str.h"

SIMULATION *init_sim(float max_extent, unsigned int max_depth, SIM_TYPE type);
void free_sim(SIMULATION *sim);
int link_sim(SIMULATION *sim, SIMULATION *target);
int unlink_sim(SIMULATION *sim, SIMULATION *target);
void sim_clear_forces(SIMULATION* sim);

// 3D Interface
int sim_add_entity(SIMULATION *sim, ENTITY *entity, size_t collider_filter);
int sim_remove_entity(SIMULATION *sim, ENTITY *entity);
void sim_add_force(SIMULATION *sim, vec3 force);
void prep_sim_movement(SIMULATION *sim);
//void update_sim_movement(SIMULATION *, int);
void update_sim_movement(SIMULATION *sim);
size_t peek_integration(SIMULATION *sim, SIM_STATE **state, vec3 origin,
                        float range);
void integrate_sim(SIMULATION *sim, vec3 origin, float range);
void integrate_sim_collider(SIMULATION *sim, ENTITY *ent, size_t col);
size_t get_sim_collisions(SIMULATION *sim, COLLISION **dest, vec3 origin,
                          float range, int get_col_info);
size_t sim_get_nearby(SIMULATION *sim, COLLISION **dest, vec3 pos,
                      float range, int get_col_info);
size_t save_sim_state(SIMULATION *sim, SIM_STATE **state);
void restore_sim_state(SIMULATION *sim, SIM_STATE *state, size_t state_size);
void free_sim_state(SIM_STATE *state, size_t num_ents);
void impulse_collision(COL_ARGS *a_args, COL_ARGS *b_args, vec3 p_dir,
                       vec3 p_loc, vec3 gravity);
void draw_oct_tree(MODEL *cube, OCT_TREE *tree, vec3 pos, float scale,
                   unsigned int shader, size_t offset, int depth);

// 2D Interface
int sim_add_entity_2d(SIMULATION *sim, ENTITY_2D *entity,
                      size_t collider_filter);
int sim_remove_entity_2d(SIMULATION *sim, ENTITY_2D *entity);
void sim_add_force_2d(SIMULATION *sim, vec2 force);
void prep_sim_movement_2d(SIMULATION *sim);
void update_sim_movement_2d(SIMULATION *sim);
void integrate_sim_2d(SIMULATION *sim, vec2 origin, float range);
size_t get_sim_collisions_2d(SIMULATION *sim, COLLISION_2D **dest, vec2 origin,
                             float range, int get_col_info);
size_t sim_get_nearby_2d(SIMULATION *sim, COLLISION_2D **dest, vec2 pos,
                         float range, int get_col_info);
void draw_quad_tree(QUAD_TREE *tree, unsigned int shader);

#endif
