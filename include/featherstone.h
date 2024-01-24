#include <entity_str.h>
#include <stdio.h>

//#define DEBUG_FS

/*
static vec3 BASIS_VECTORS[3] = {
  {1.0, 0.0, 0.0},
  {0.0, 1.0, 0.0},
  {0.0, 0.0, 1.0},
};
*/

static mat3 MAT3_ZERO = GLM_MAT3_ZERO_INIT;

#define GRAVITY (10.0)
static vec3 G_VEC = { 0.0, -GRAVITY, 0.0 };

int featherstone_abm(ENTITY *body);
void compute_spatial_transformations(mat4 p_bone_to_world,
                                     vec3 p_coords,
                                     mat4 c_bone_to_world,
                                     vec3 c_coords,
                                     vec3 p_to_c_lin_dest,
                                     mat6 p_to_c_dest,
                                     mat6 c_to_p_dest);
void compute_spatial_velocity(int cur_col, int parent_col, COLLIDER *colliders,
                              P_DATA *p_data);
void compute_articulated_data(int col, P_DATA *p_data, mat6 i_dest,
                              vec6 z_dest);

void get_model_mat(ENTITY *entity, mat4 model);
void print_vec6(vec6 v);
void print_mat6(mat6 m);
void print_vec3(vec3 v);
void print_mat3(mat3 m);
void print_mat4 (mat4 m);
