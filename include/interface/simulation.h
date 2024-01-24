#ifndef __ENGINE_SIMULATION_H__
#define __ENGINE_SIMULATION_H__

#include "./simulation_str.h"
#include "./entity_str.h"

SIMULATION *init_sim();
void free_sim(SIMULATION *sim);

int sim_add_entity(SIMULATION *sim, ENTITY *entity, int collider_filter);
int sim_remove_entity(SIMULTION *sim, ENTITY *entity);
void sim_add_force(SIMULATION *sim, vec3 force);
void sim_clear_force(SIMULATION* sim);
void integrate_sim(SIMULATION *sim);
size_t get_sim_collisions(SIMULATION *sim, COLLISION **dest);
void impulse_resolution(SIMULATION **sim, COLLISION col);

#endif
