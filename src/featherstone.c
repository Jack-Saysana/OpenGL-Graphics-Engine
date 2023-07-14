#include <featherstone.h>

int featherstone_abm(ENTITY *body) {
  size_t num_links = body->model->num_colliders;
  COLLIDER *links = body->model->colliders;

  vec3 temp = GLM_VEC3_ZERO_INIT;

  // Calculate spatial velocities from inbound to outbound
  // TODO Need to take inertial frames into account
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
      vec6_copy(body->np_data[parent_col].v_hat, parent_vel);
    }

    vec3 ang_vel = GLM_VEC3_ZERO_INIT;
    vec3 vel = GLM_VEC3_ZERO_INIT;

    P_DATA *p_data = body->np_data + i;
    vec3 center_to_joint = GLM_VEC3_ZERO_INIT;
    if (body->model->colliders[i].type == POLY) {
      glm_vec3_sub(body->model->bones[root_bone].coords,
                   body->model->colliders[i].data.center_of_mass,
                   center_to_joint);
    } else {
      glm_vec3_sub(body->model->bones[root_bone].coords,
                   body->model->colliders[i].data.center,
                   center_to_joint);
    }

    vec3 *basis_vectors = body->model->bones[root_bone].basis_vectors;

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
        glm_vec3_scale(basis_vectors[j], p_data->joint_angle_vels[j + 3],
                       temp);
        glm_vec3_add(temp, ang_vel, ang_vel);
        glm_vec3_scale(basis_vectors[j], p_data->joint_angle_vels[j + 3],
                       temp);
        glm_vec3_cross(temp, center_to_joint, temp);
        glm_vec3_add(temp, vel, vel);
      }
    }

    vec6_compose(ang_vel, vel, p_data->v_hat);
    vec6_add(p_data->v_hat, parent_vel, p_data->v_hat);
  }

  // TODO A lot of this can be precomputed and never changes (I-hat, za_linear)
  // Calculate I-hat and Z-hat from inbound to outbound
  for (int i = 0; i < num_links; i++) {
    P_DATA *p_data = body->np_data + i;

    // M
    float mass = 1.0 / p_data->inv_mass;
    mat3 mass_mat = GLM_MAT3_IDENTITY_INIT;
    glm_mat3_scale(mass_mat, mass);

    // I
    mat4 interia_tensor_m4 = GLM_MAT4_IDENTITY_INIT;
    mat3 inertia = GLM_MAT3_IDENTITY_INIT;
    glm_mat4_inv(p_data->inv_inertia, inertia_tensor_m4);
    glm_mat4_pick3(inertia_tensor_m4, inertia);

    // I-hat = [[0, M], [I, 0]];
    mat6_compose(MAT3_ZERO, mass_mat, inertia, MAT3_ZERO,
                 p_data->I_hat);

    // L
    vec3 za_linear = { 0.0, -mass * GRAVITY, 0.0};

    // A
    vec3 za_angular = GLM_VEC3_ZERO_INIT;
    vec3 ang_vel = GLM_VEC3_ZERO_INIT;
    vec3 *basis_vectors = body->model->bones[root_bone].basis_vectors;
    for (int j = 0; j < 3; j++) {
      if (p_data->dofs[j + 3]) {
        glm_vec3_scale(basis_vectors[j], p_data->joint_angle_vels[j + 3],
                       temp);
        glm_vec3_add(temp, ang_vel, ang_vel);
      }
    }
    glm_mat3_mulv(inertia, ang_vel, za_angular);
    glm_vec3_cross(ang_vel, za_angular, za_angular);

    // Z-hat = [L, A]
    vec6_compose(za_linear, za_angular, p_data->Z_hat);
  }

  // Calculate I-hat-A and Z-hat-A from outbound to inbound
  for (int i = num_links - 1; i >= 0; i--) {
    if (links[i].category != HIT_BOX) {
      continue;
    }

    root_bone = body->model->collider_bone_links[i];
    vec3 center_to_joint = GLM_VEC3_ZERO_INIT;
    if (body->model->colliders[i].type == POLY) {
      glm_vec3_sub(body->model->bones[root_bone].coords,
                   body->model->colliders[i].data.center_of_mass,
                   center_to_joint);
    } else {
      glm_vec3_sub(body->model->bones[root_bone].coords,
                   body->model->colliders[i].data.center,
                   center_to_joint);
    }

    P_DATA *p_data = body->np_data + i;
    vec3 *basis_vectors = body->model->bones[root_bone].basis_vectors;

    // Matrix rotating vectors in world coordinates to vectors in the current
    // bone's coordinates
    mat4 world_to_cur = body->bone_mats[root_bone][ROTATION];
    glm_mat4_inv(cur_from_world_rot, cur_from_world_rot);

    size_t *children = body->model->collider_children +
                      links[i].children_offset;

    size_t cur_child = -1;
    P_DATA *child_p_data = NULL;
    for (int j = 0; j < links[i].num_children; j++) {
      // Accumulate children I-hat-A and Z-hat-A
      child_data = body->np_data + children[j];
      cur_child = body->model->collider_bone_links[children[j]];

      // Spatial transformation matrix transforming vectors in the current
      // link's frame to vectors in the child link's frame
      mat6 cur_to_child = MAT6_ZERO_INIT;
      // Spatial transformation matrix transforming vectors in the child
      // link's frame to vectors in the current links' frame
      mat6 child_to_cur = MAT6_ZERO_INIT;
      compute_spatial_transformations(world_to_cur,
                                      body->model->bones[root_bone].coords,
                                      body->bone_mats[cur_child],
                                      body->model->bones[cur_child].coords,
                                      cur_to_child, child_to_cur);

      vec6 s_dot_IA = VEC6_ZERO_INIT;
      vec6_spatial_transpose_mulm(child_data[children[j]].s_hat,
                                  child_data[children[j]].I_hat_A,
                                  axis_dot_ineria);

      float ssIA = vec6_dot(child_data[children[j]].s_hat, s_dot_IA);
    }

    vec6 spatial_force = VEC6_ZERO_INIT;
    mat6_mulv(p_data->artic_spatial_inertia, p_data->spatial_accel,
              spatial_force);
    mat6_add(spatial_force, p_data->artic_spatial_zero_accel, spatial_force);

    // TODO precompute
    vec3 axis_ang = GLM_VEC3_ZERO_INIT;
    vec3 axis_lin = GLM_VEC3_ZERO_INIT;
    for (int i = 5; i >= 0; i--) {
      if (dofs[i]) {
        if (i < 3) {
          glm_vec3_copy(basis_vectors[i], axis_lin);
        } else {
          glm_vec3_copy(basis_vectors[i - 3], axis_ang);
          glm_vec3_cross(basis_vectors[i - 3], center_to_joint, axis_lin);
        }
        vec6_compose(axis_ang, axis_lin, p_data->spatial_axis);
        break;
      }
    }

    p_data->spatial_force_mag = vec6_dot(spatial_axis, spatial_force);
  }

  // Calculate q** and spatial acceleration from inbound to outbound
  for (int i = 0; i < num_links; i++) {

  }

  return 0;
}

void compute_spatial_transformations(mat4 world_to_parent_rot,
                                     vec3 parent_coords,
                                     mat4* child_mats,
                                     vec3 child_coords,
                                     mat6 parent_to_child_dest,
                                     mat6 child_to_parent_dest) {
  // Matrix rotating vectors in the child bone's coordinates to world
  // coordinates
  mat4 child_to_world = parent_mats[ROTATION];

  // Matrix rotating vectors in child's bone coordinates to vectors in the
  // current bone's coordinates
  mat3 child_to_parent_rot = GLM_MAT3_IDENTITY_INIT;
  glm_mat4_mul(world_to_parent_rot, child_to_world, child_to_world);
  glm_mat4_pick3(child_to_world, child_to_parent_rot);

  // Matrix rotating vectors in the current bone's coordinates to vectors
  // in the child's bone coordinates
  mat3 parent_to_child_rot = GLM_MAT3_IDENTITY_INIT;
  glm_mat3_inv(child_to_parent_rot, parent_to_child_rot);

  // Matrix who's diagonal is the vector from the parent bone tail to the
  // child bone tail
  mat3 parent_to_child_mat = GLM_MAT3_IDENTITY_INIT;
  vec3 parent_to_child = GLM_VEC3_ZERO_INIT;
  glm_vec3_sub(child_coords, parent_coords, parent_to_child);
  glm_mat3_mulv(parent_to_child_mat, parent_to_child, parent_to_child_mat);

  // Matrix who's diagonal is the vector from the child bone tail to the
  // parent bone tail
  mat3 child_to_parent_mat = GLM_MAT3_IDENTITY_INIT;
  vec3 child_to_parent = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(parent_to_child, -1.0, child_to_parent);
  glm_mat3_mulv(child_to_parent_mat, child_to_parent, child_to_parent_mat);

  // Spatial transformation matrix transforming vectors in the child bone's
  // coords to vectors in the current bone's coords
  mat6_spatial_transform(child_to_parent_rot, parent_to_child_mat,
                         child_to_parent_dest);

  // Spatial transformation matrix transforming vectors in the current
  // bone's coords to vectors in the child bone's coords
  mat6_spatial_transform(parent_to_child, rot, child_to_parent_mat,
                         parent_to_child_dest);
}
