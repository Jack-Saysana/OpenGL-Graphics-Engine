#include <featherstone.h>

int featherstone_abm(ENTITY *body) {
  size_t num_links = body->model->num_colliders;
  COLLIDER *links = body->model->colliders;
  // Calculate spatial velocities from inbound to outbound
  int root_bone = -1;
  int parent_col = -1;
  for (int i = 0; i < num_links; i++) {
    if (links[i].category != HIT_BOX) {
      continue;
    }

    root_bone = body->model->collider_bone_links[i];
    parent_col = -1;
    if (root_bone != -1) {
      int parent_bone = body->model->bones[root_bone].parent;
      parent_col = body->model->bone_collider_links[parent_bone];
    }

    vec6 parent_vel = VEC6_ZERO_INIT;
    if (parent_col != -1) {
      vec6_copy(body->np_data[parent_col].spatial_vel, parent_vel);
    }

    vec3 ang_vel = GLM_VEC3_ZERO_INIT;
    vec3 vel = GLM_VEC3_ZERO_INIT;

    P_DATA *p_data = body->np_data + i;
    vec3 center_to_joint = GLM_VEC3_ZERO_INIT;
    if (root_bone) {
      if (body->model->colliders[i].type == POLY) {
        glm_vec3_sub(body->model->bones[root_bone].coords,
                     body->model->colliders[i].data.center_of_mass,
                     center_to_joint);
      } else {
        glm_vec3_sub(body->model->bones[root_bone].coords,
                     body->model->colliders[i].data.center,
                     center_to_joint);
      }
    }

    vec3 *basis_vectors = body->model->bones[root_bone].basis_vectors;

    vec3 temp = GLM_VEC3_ZERO_INIT;
    // Accumulate velocities from prismatic DOFS
    for (int j = 0; j < 3; j++) {
      if (p_data->dofs[j]) {
        glm_vec3_scale(basis_vectors[j], p_data->joint_angle_vels[j], temp);
        glm_vec3_add(temp, vel, vel);
      }
    }
    // Accumulate velocities from revolute DOFS
    for (int j = 0; j < 3; j++) {
      if (p_data->dofs[j + 3]) {
        glm_vec3_scale(basis_vectors[j], p_data->joint_angle_vels[j], temp);
        glm_vec3_add(temp, ang_vel, ang_vel);
        glm_vec3_scale(basis_vectors[j], p_data->joint_angle_vels[j], temp);
        glm_vec3_cross(temp, center_to_joint, temp);
        glm_vec3_add(temp, vel, vel);
      }
    }

    vec6_compose(ang_vel, vel, p_data->spatial_vel);
    vec6_add(p_data->spatial_vel, parent_vel, p_data->spatial_vel);
  }

  // Calculate I-hat and Z-hat from inbound to outbound
  for (int i = 0; i < num_links; i++) {
    P_DATA *p_data = body->np_data + i;
    mat3 mass_mat = GLM_MAT3_IDENTITY_INIT;
    glm_mat3_scale(mass_mat, 1.0 / p_data->inv_mass);
  }

  // Calculate I-hat-A and Z-hat-A from outbound to inbound
  for (int i = num_links - 1; i >= 0; i--) {

  }

  // Calculate q** and spatial acceleration from inbound to outbound
  for (int i = 0; i < num_links; i++) {

  }

  return 0;
}
