#ifndef __ENGINE_SIMULATION_H__
#define __ENGINE_SIMULATION_H__

#include "./structs/simulation_str.h"
#include "./structs/models/entity_str.h"

SIMULATION *init_sim(float max_extent, unsigned int max_depth);
void free_sim(SIMULATION *sim);

int link_sim(SIMULATION *sim, SIMULATION *target);
int unlink_sim(SIMULATION *sim, SIMULATION *target);
int sim_add_entity(SIMULATION *sim, ENTITY *entity, size_t collider_filter);
int sim_remove_entity(SIMULATION *sim, ENTITY *entity);
void sim_add_force(SIMULATION *sim, vec3 force);
void sim_clear_force(SIMULATION* sim);
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
                      float range);
size_t save_sim_state(SIMULATION *sim, SIM_STATE **state);
void restore_sim_state(SIMULATION *sim, SIM_STATE *state, size_t state_size);
void free_sim_state(SIM_STATE *state, size_t num_ents);
void impulse_collision(COL_ARGS *a_args, COL_ARGS *b_args, vec3 p_dir,
                       vec3 p_loc, vec3 gravity);

void draw_oct_tree(MODEL *cube, OCT_TREE *tree, vec3 pos, float scale,
                   unsigned int shader, size_t offset, int depth);
#endif
