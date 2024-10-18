#ifndef __ENGINE_P_DATA_STR_H__
#define __ENGINE_P_DATA_STR_H__

typedef struct zero_joint_data {
  mat6 I_hat_A;
  mat6 ST_to_parent;
  mat6 ST_from_parent;
  mat3 bone_to_world;
  mat3 rot_mat;
  vec6 s_hat;
  vec6 Z_hat;
  vec6 Z_hat_A;
  vec6 c_hat;
  vec6 a_hat;
  vec6 v_hat;
  vec6 s_inner_I;
  vec3 dof;
  vec3 joint_to_com;
  size_t next_offset;
  int joint_type;
  float s_inner_I_dot_s;
  float SZI;
  float accel_angle;
  float vel_angle;
  float joint_angle;
} ZERO_JOINT;

typedef struct p_data {
  // Spatial inertia
  mat6 I_hat;
  // Articulated spatial inertia
  mat6 I_hat_A;
  // Spatial tranformation from the current links inertial frame to its
  // parent's inertial frame
  mat6 ST_to_parent;
  // Spatial transformation to the current link's inertial frame from its
  // parent's inertial frame
  mat6 ST_from_parent;
  // Inverse inertia tensor, given in bone space
  mat4 inv_inertia;
  // Spatial joint axis
  vec6 s_hat;
  // Spatial zero acceleration
  vec6 Z_hat;
  // Articulated spatial zero acceleration
  vec6 Z_hat_A;
  // Coriolis vector
  vec6 coriolis_vector;
  // Spatial acceleration
  vec6 a_hat;
  // Spatial velocity
  vec6 v_hat;
  // Shortcut of s'I_hat_A
  vec6 s_inner_I;
  // External forces exerted on the joint
  vec6 e_force;
  // Read-only world-space linear/angular accleration/velocity
  vec3 a;
  vec3 ang_a;
  vec3 v;
  vec3 ang_v;
  // Buffer specifiying the degree of freedom for link
  vec3 dof;
  // Vector pointing from link parent's COM to current link's COM in bone space
  vec3 from_parent_lin;
  // Vector pointing from link's joint to link's COM in bone space
  vec3 joint_to_com;
  // Offset in entity zero-joint buffer to first zero joint
  size_t zero_joint_offset;
  size_t num_z_joints;
  // Specifies if the joint is revolute or prismatic
  int joint_type;
  float inv_mass;
  // Shortcut for dot(s'I_hat_A, s_hat)
  float s_inner_I_dot_s;
  // Shortcut for s'(Z_hat_A + I_hat(coriolis))
  float SZI;
  // Joint angle acceleration
  float accel_angle;
  // Joint angle velocity
  float vel_angle;
  // Joint angle
  float joint_angle;
} P_DATA;

#endif
