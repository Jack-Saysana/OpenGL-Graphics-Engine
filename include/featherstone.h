#include <entity_str.h>

static vec3 BASIS_VECTORS[3] = {
  {1.0, 0.0, 0.0},
  {0.0, 1.0, 0.0},
  {0.0, 0.0, 1.0},
};

static mat3 MAT3_ZERO = GLM_MAT3_ZERO_INIT;

#define GRAVITY (10.0)
static vec3 G_VEC = { 0.0, -GRAVITY, 0.0 };

int featherstone_abm(ENTITY *body);
void compute_spatial_transformations(mat4* parent_mats,
                                     vec3 parent_coords,
                                     mat4* child_mats,
                                     vec3 child_coords,
                                     vec3 parent_to_child_lin_dest,
                                     mat6 parent_to_child_dest,
                                     mat6 child_to_parent_dest);
void compute_spatial_velocity(int cur_col, int parent_col, COLLIDER *colliders,
                              P_DATA *p_data);
