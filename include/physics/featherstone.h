#include <stdio.h>
#include <cglm/cglm.h>
#include <const.h>
#include <structs/models/entity_str.h>

static mat3 MAT3_ZERO = GLM_MAT3_ZERO_INIT;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

void compute_spatial_transformations(mat4 p_bone_to_world, vec3 p_coords,
                                     mat4 c_bone_to_world, vec3 c_coords,
                                     vec3 p_to_c_lin_dest, mat6 p_to_c_dest,
                                     mat6 c_to_p_dest);
void compute_spatial_velocity(int cur_col, int parent_col, mat4 bone_to_world,
                              COLLIDER *colliders, P_DATA *p_data);
//void compute_articulated_data(int col, P_DATA *p_data, mat6 i_dest,
//                              vec6 z_dest);
void calc_spatial_vel(mat6 from_parent, vec6 p_v_hat, vec3 u, vec3 d, vec3 r,
                      float vel_angle, int joint_type, vec6 s_hat_dest,
                      vec6 c_hat_dest, vec6 dest);
void calc_coriolis(vec3 va_parent, vec3 u, vec3 r, vec3 d, float vel_angle,
                   int joint_type, vec6 dest);
void calc_IZ_hat(mat3 ent_to_world, mat3 bone_to_ent, mat3 inv_inertia,
                 vec3 grav, vec6 e_forces, vec3 ang_vel, float inv_mass,
                 mat6 i_dest, vec6 z_dest);
void calc_articulated_child(mat6 to_parent, mat6 from_parent, mat6 I_hat_A,
                            vec6 Z_hat_A, vec6 c_hat, vec6 s_hat, float Q,
                            mat6 i_dest, vec6 z_dest, vec6 si_dest,
                            float *sis_dest, float *szi_dest);
void calc_a_hat(mat6 from_parent, vec6 p_a_hat, vec6 c_hat, vec6 s_hat,
                vec6 si, float szi, float sis, vec6 a_hat_dest,
                float *accel_angle_dest);
void calc_zj_ent_to_world(mat4 p_ent_to_world, vec3 p_head_ent, vec3 dof,
                          float joint_angle, int joint_type, mat4 dest);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

void print_vec6(vec6 v);
void print_mat6(mat6 m);
void print_vec3(vec3 v);
void print_mat3(mat3 m);
void print_mat4 (mat4 m);
