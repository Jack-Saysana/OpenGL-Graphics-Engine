#ifndef __ENGINE_SIMULATION_STR_H__
#define __ENGINE_SIMULATION_STR_H__

#include "./oct_tree_str.h"
#include "./models/entity_str.h"

typedef struct simulation_collider {
  ENTITY *entity;
  void *data;
  size_t collider_offset;
  size_t index;
  int status;
  int to_delete;
} SIM_COLLIDER;

typedef struct simulation_entity {
  ENTITY *entity;
  void *data;
  size_t index;
  int status;
  int to_delete;
} SIM_ENTITY;

typedef union simulation_item {
  SIM_COLLIDER col;
  SIM_ENTITY ent;
} SIM_ITEM;

typedef union ledger_input {
  struct c_data{
    ENTITY *ent;
    size_t col;
    void *data;
  } collider;
  struct e_data {
    ENTITY *ent;
    void *data;
  } entity;
} LEDGER_INPUT;

typedef struct simulation {
  struct simulation *linked_sims[MAX_LINKED_SIMS];
  // Oct tree used for collision detection
  OCT_TREE *oct_tree;
  SIM_ITEM *ent_ledger;
  // Hash-map of moving entities
  SIM_ITEM *ment_ledger;
  // Hash-map of moving colliders
  SIM_ITEM *mcol_ledger;
  // Hash-map of colliders which "drive" movement
  SIM_ITEM *dcol_ledger;
  size_t *ent_list;
  // List corresponding to moving entity ledger used for linear traversal
  size_t *ment_list;
  // List corresponding to moving collider ledger used for linear traversal
  size_t *mcol_list;
  // List corresponding to driving ledger used for linear traversal
  size_t *dcol_list;

  size_t num_ent;
  size_t ent_ledger_size;
  size_t ent_list_size;
  size_t num_ent_moving;
  size_t ment_ledger_size;
  size_t ment_list_size;
  size_t num_col_moving;
  size_t mcol_ledger_size;
  size_t mcol_list_size;
  size_t num_col_driving;
  size_t dcol_ledger_size;
  size_t dcol_list_size;

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
