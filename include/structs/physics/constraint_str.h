#ifndef __CONSTRAINT_STR_H__
#define __CONSTRAINT_STR_H__

typedef struct joint_constraint {
  size_t col_idx;
  vec3 pt; // World space
  vec3 d_accel; // World space
} J_CONS;

#endif
