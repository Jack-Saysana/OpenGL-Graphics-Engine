#ifndef __ENGINE_ENTITY_STR_H__
#define __ENGINE_ENTITY_STR_H__

#include <cglm/mat4.h>
#include "../../math/spatial_algebra.h"
#include "../../const.h"
#include "model_str.h"
#include "../physics/constraint_str.h"
#include "../physics/p_data_str.h"

typedef struct entity {
  // Pointer to miscelaneous data to link entity to other information
  void *data;
  // Callback function to integrate the entity in simulations
  void (*move_cb)(struct entity *, vec3);
  // Callback function to check if the entity is moving
  int (*is_moving_cb)(struct entity *, size_t);
  MODEL *model;
  // Physics constraint buffer to be applied for the current frame
  J_CONS *p_cons;
  size_t num_cons;
  size_t cons_size;
  // Location, rotation and scale matricies for each bone
  mat4 (*bone_mats)[3];
  // Physics data for each collider
  P_DATA *np_data;
  // Physics data for zero-joints that provide additional dofs to colliders
  ZERO_JOINT *zj_data;
  // Model matrix for each bone, including those inherited by parent bones
  mat4 *final_b_mats;
  // Physics system status
  // Bit layout: 0...0[MUTABLE/IMMUTABLE][DRIVEN/DRIVING][STATIC/DYNAMIC]
  int type;
} ENTITY;

#endif
