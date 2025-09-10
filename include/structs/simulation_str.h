#ifndef __SIMULATION_STR_H__
#define __SIMULATION_STR_H__

#include <structs/oct_tree_str.h>
#include <structs/sim_ledger_str.h>

typedef struct simulation {
  struct simulation *linked_sims[MAX_LINKED_SIMS];
  // Oct tree used for collision detection
  OCT_TREE *oct_tree;

  SIM_LEDGER ent_ledger;
  SIM_LEDGER ment_ledger;
  SIM_LEDGER mcol_ledger;
  SIM_LEDGER dcol_ledger;

  // Magnitude of net acceleration caused by external forces
  vec3 forces;
  int num_linked_sims;
} SIMULATION;

typedef struct sim_state {
  ENTITY *entity;
  mat4 (*bone_mats)[3];
  struct collider_state {
    float joint_angle;
    float vel_angle;
  } *col_state;
} SIM_STATE;

typedef struct collision_list {
  ENTITY *a_ent;
  ENTITY *b_ent;
  size_t a_offset;
  size_t b_offset;
  vec3 col_dir; // Penetration vector in the direction of A into B
  vec3 col_point;
} COLLISION;

typedef struct collision_args {
  float *vel_dest;
  float *ang_vel_dest;
  float *velocity;
  float *ang_velocity;
  size_t collider;
  mat4 inv_inertia;
  versor rotation;
  vec3 center_of_rotation;
  float inv_mass;
  int type;
} COL_ARGS;

#endif
