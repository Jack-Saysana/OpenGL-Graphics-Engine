#include <featherstone.h>

int featherstone_abm(ENTITY *body) {
  size_t num_links = body->model->num_colliders;

  BONE *bones = body->model->bones;
  COLLIDER *colliders = body->model->colliders;
  int *bone_from_col = body->model->collider_bone_links;
  int *col_from_bone = body->model->bone_collider_links;
  P_DATA *p_data = body->np_data;

  int root_bone = -1;
  int parent_bone = -1;
  int parent_col = -1;

  mat4 global_ent_to_world = GLM_MAT4_IDENTITY_INIT;
  get_model_mat(body, global_ent_to_world);

  // Calculate spatial velocities from inbound to outbound
  for (int cur_col = 0; cur_col < num_links; cur_col++) {
    if (colliders[cur_col].category != HURT_BOX) {
      continue;
    }

    root_bone = bone_from_col[cur_col];

    // Transforms entity space vectors of current link to world space
    mat4 cur_ent_to_world = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mul(body->final_b_mats[root_bone], global_ent_to_world,
                 cur_ent_to_world);

    // Transforms bone space vectors of current link to world space
    mat4 cur_bone_to_world = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_ins3(bones[root_bone].coordinate_matrix, cur_bone_to_world);
    glm_mat4_mul(cur_ent_to_world, cur_bone_to_world, cur_bone_to_world);

    // Transforms entity space vectors of current link to bone space of current
    // link
    mat4 cur_ent_to_bone = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_ins3(bones[root_bone].coordinate_matrix, cur_ent_to_bone);
    glm_mat4_inv(cur_ent_to_bone, cur_ent_to_bone);

    vec3 cur_coords = GLM_VEC3_ZERO_INIT;
    // Uses entity space to world space because center/center_of_mass is given
    // in entity space
    // TODO joint_to_com can be precomputed
    if (colliders[cur_col].type == POLY) {
      glm_vec3_sub(colliders[cur_col].data.center_of_mass,
                   bones[root_bone].base, p_data[cur_col].joint_to_com);
      glm_mat4_mulv3(cur_ent_to_world, colliders[cur_col].data.center_of_mass,
                     1.0, cur_coords);
    } else {
      glm_vec3_sub(colliders[cur_col].data.center, bones[root_bone].base,
                   p_data[cur_col].joint_to_com);
      glm_mat4_mulv3(cur_ent_to_world, colliders[cur_col].data.center, 1.0,
                     cur_coords);
    }

    glm_mat4_mulv3(cur_ent_to_bone, p_data[cur_col].joint_to_com, 1.0,
                   p_data[cur_col].joint_to_com);

    parent_bone = bones[root_bone].parent;
    parent_col = -1;
    if (parent_bone != -1) {
      // Transforms entity space vectors of parent link to world space
      mat4 parent_ent_to_world = GLM_MAT4_IDENTITY_INIT;
      glm_mat4_mul(body->final_b_mats[parent_bone], global_ent_to_world,
                   parent_ent_to_world);

      // Transforms bone space vectors of parent link to world space
      mat4 parent_bone_to_world = GLM_MAT4_IDENTITY_INIT;
      glm_mat4_ins3(bones[parent_bone].coordinate_matrix,
                    parent_bone_to_world);
      glm_mat4_mul(parent_ent_to_world, parent_bone_to_world,
                   parent_bone_to_world);

      parent_col = col_from_bone[parent_bone];
      vec3 parent_coords = GLM_VEC3_ZERO_INIT;
      // Uses entity space to world space because center/center_of_mass is
      // given in entity space
      if (colliders[parent_col].type == POLY) {
        glm_mat4_mulv3(parent_ent_to_world,
                       colliders[parent_col].data.center_of_mass, 1.0,
                       parent_coords);
      } else {
        glm_mat4_mulv3(parent_ent_to_world, colliders[parent_col].data.center,
                       1.0, parent_coords);
      }

      compute_spatial_transformations(parent_bone_to_world,
                                      parent_coords,
                                      cur_bone_to_world,
                                      cur_coords,
                                      p_data[cur_col].from_parent_lin,
                                      p_data[cur_col].ST_from_parent,
                                      p_data[cur_col].ST_to_parent);
    }

    compute_spatial_velocity(cur_col, parent_col, colliders, p_data);
  }

  // TODO I_hat can be precomputed
  // Calculate I-hat and Z-hat from inbound to outbound
  for (int cur_col = 0; cur_col < num_links; cur_col++) {
    if (colliders[cur_col].category != HURT_BOX) {
      continue;
    }

    root_bone = bone_from_col[cur_col];

    mat4 temp = GLM_MAT4_IDENTITY_INIT;

    // M
    float mass = 1.0 / p_data[cur_col].inv_mass;
    mat3 mass_mat = GLM_MAT3_IDENTITY_INIT;
    glm_mat3_scale(mass_mat, mass);

    // I
    mat3 bone_to_world = GLM_MAT3_IDENTITY_INIT;
    mat3 bone_to_world_transpose = GLM_MAT3_IDENTITY_INIT;
    mat3 world_to_bone = GLM_MAT3_IDENTITY_INIT;
    mat3 ent_to_world = GLM_MAT3_IDENTITY_INIT;
    glm_mat4_pick3(body->final_b_mats[root_bone], ent_to_world);
    glm_mat3_mul(ent_to_world, bones[root_bone].coordinate_matrix,
                 bone_to_world);
    glm_mat3_inv(bone_to_world, world_to_bone);
    glm_mat3_transpose_to(bone_to_world, bone_to_world_transpose);

    mat3 inertia_tensor = GLM_MAT3_IDENTITY_INIT;
    glm_mat4_inv(p_data[cur_col].inv_inertia, temp);
    glm_mat4_pick3(temp, inertia_tensor);
    glm_mat3_mul(bone_to_world, inertia_tensor, inertia_tensor);
    glm_mat3_mul(inertia_tensor, bone_to_world_transpose, inertia_tensor);

    // I_hat
    mat6_compose(MAT3_ZERO, mass_mat, inertia_tensor, MAT3_ZERO,
                 p_data[cur_col].I_hat);

    // Convert gravity to bone space
    vec3 gravity = GLM_VEC3_ZERO_INIT;
    glm_mat3_mulv(world_to_bone, G_VEC, gravity);

    vec3 za_linear = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(gravity, mass, za_linear);

    float *ang_vel = (float *) p_data[cur_col].v_hat;
    vec3 za_ang = GLM_VEC3_ZERO_INIT;
    glm_mat3_mulv(inertia_tensor, ang_vel, za_ang);

    glm_vec3_cross(ang_vel, za_ang, za_ang);

    // Z_hat
    vec6_compose(za_linear, za_ang, p_data[cur_col].Z_hat);

    // TODO account for different types of joints and accumulation of different
    // DOFs
    // Vector representing joint axis scaled by joint velocity magnitude
    // Coriolis Vector
    vec3 temp_vec3 = GLM_VEC3_ZERO_INIT;
    vec6_zero(p_data[cur_col].coriolis_vector);
    int parent_bone = bones[root_bone].parent;
    if (parent_bone != -1) {
      vec3 p_ang_vel = GLM_VEC3_ZERO_INIT;
      glm_vec3_copy(p_data[col_from_bone[parent_bone]].v_hat, p_ang_vel);

      vec3 v = GLM_VEC3_ZERO_INIT;
      vec3 coriolis_first = GLM_VEC3_ZERO_INIT;
      glm_vec3_scale(BASIS_VECTORS[0], p_data[cur_col].vel_angles[3], v);
      glm_vec3_cross(p_ang_vel, v, coriolis_first);

      vec3 v_cross_d = GLM_VEC3_ZERO_INIT;
      glm_vec3_cross(v, p_data[cur_col].joint_to_com, v_cross_d);

      vec3 p_ang_vel_cross_r = GLM_VEC3_ZERO_INIT;
      glm_vec3_cross(p_ang_vel, p_data[cur_col].from_parent_lin,
                     p_ang_vel_cross_r);

      vec3 coriolis_last = GLM_VEC3_ZERO_INIT;
      glm_vec3_scale(p_ang_vel, 2.0, coriolis_last);
      glm_vec3_cross(coriolis_last, v_cross_d, coriolis_last);

      glm_vec3_cross(p_ang_vel, p_ang_vel_cross_r, temp_vec3);
      glm_vec3_add(coriolis_last, temp_vec3, coriolis_last);

      glm_vec3_cross(v, v_cross_d, temp_vec3);
      glm_vec3_add(coriolis_last, temp_vec3, coriolis_last);

      vec6_compose(coriolis_first, coriolis_last,
                   p_data[cur_col].coriolis_vector);
    }

    // TODO account for different types of joints
    glm_vec3_cross(BASIS_VECTORS[0], p_data[cur_col].joint_to_com, temp_vec3);
    // Spatial Joint Axis
    vec6_compose(BASIS_VECTORS[0], temp_vec3, p_data[cur_col].s_hat);
  }

  // Calculate I-hat-A and Z-hat-A from outbound to inbound
  for (int cur_col = num_links - 1; cur_col >= 0; cur_col--) {
    if (colliders[cur_col].category != HURT_BOX) {
      continue;
    }

    mat6_copy(p_data[cur_col].I_hat, p_data[cur_col].I_hat_A);
    vec6_copy(p_data[cur_col].Z_hat, p_data[cur_col].Z_hat_A);
    vec6_zero(p_data[cur_col].coriolis_vector);

    mat6 temp_mat6 = MAT6_ZERO_INIT;
    vec6 temp_vec6 = VEC6_ZERO_INIT;
    for (int cur_child = 0; cur_child < colliders[cur_col].num_children;
         cur_child++) {
      int child_col = colliders[cur_col].children_offset + cur_child;
      vec6 I_hat_s_hat = VEC6_ZERO_INIT;
      // Is
      mat6_mulv(p_data[child_col].I_hat_A, p_data[child_col].s_hat,
                I_hat_s_hat);

      // Iss'
      vec6_spatial_transpose_mulv(I_hat_s_hat, p_data[child_col].s_hat,
                                  temp_mat6);

      // Iss'I
      mat6_mul(temp_mat6, p_data[child_col].I_hat_A, temp_mat6);

      vec6_spatial_transpose_mulm(p_data[child_col].s_hat,
                                  p_data[child_col].I_hat_A, temp_vec6);

      // s'Is
      p_data[child_col].s_inner_I_dot_s = vec6_dot(temp_vec6,
                                                   p_data[child_col].s_hat);

      mat6_scale(temp_mat6, 1.0 / p_data[child_col].s_inner_I_dot_s,
                 temp_mat6);

      mat6_sub(p_data[child_col].I_hat_A, temp_mat6, temp_mat6);
      mat6_mul(p_data[child_col].ST_to_parent, temp_mat6, temp_mat6);
      mat6_mul(temp_mat6, p_data[child_col].ST_from_parent, temp_mat6);
      // I_hat_A
      mat6_add(p_data[cur_col].I_hat_A, temp_mat6, p_data[cur_col].I_hat_A);

      vec6 I_hat_coriolis = VEC6_ZERO_INIT;
      mat6_mulv(p_data[child_col].I_hat_A, p_data[child_col].coriolis_vector,
                I_hat_coriolis);

      vec6_add(p_data[child_col].Z_hat_A, I_hat_coriolis, temp_vec6);

      // TODO calculate Q_i, which will take into account external forces
      // applied to the joint
      p_data[child_col].scalar = vec6_inner_product(p_data[child_col].s_hat,
                                                    temp_vec6);

      p_data[child_col].scalar = (p_data[child_col].Q -
                                  p_data[child_col].scalar) /
                                 p_data[child_col].s_inner_I_dot_s;

      vec6_scale(I_hat_s_hat, p_data[child_col].scalar, temp_vec6);
      vec6_add(temp_vec6, I_hat_coriolis, temp_vec6);
      vec6_add(temp_vec6, p_data[child_col].Z_hat_A, temp_vec6);
      mat6_mulv(p_data[child_col].ST_to_parent, temp_vec6, temp_vec6);
      // Z_hat_A
      vec6_add(p_data[cur_col].Z_hat_A, temp_vec6, p_data[cur_col].Z_hat_A);
    }
  }

  // Calculate q** and spatial acceleration from inbound to outbound
  for (int cur_col = 0; cur_col < num_links; cur_col++) {
    if (colliders[cur_col].category != HURT_BOX) {
      continue;
    }

    vec6_zero(p_data[cur_col].a_hat);

    int root_bone = bone_from_col[cur_col];
    parent_bone = bones[root_bone].parent;
    if (parent_bone != -1) {
      vec6 temp_vec6 = VEC6_ZERO_INIT;
      int parent_col = col_from_bone[parent_bone];
      vec6_spatial_transpose_mulm(p_data[cur_col].s_hat,
                                  p_data[cur_col].I_hat_A, temp_vec6);
      mat6_mulv(p_data[cur_col].ST_from_parent, temp_vec6, temp_vec6);
      // TODO take into account multiple dofs
      p_data[cur_col].accel_angles[3] = vec6_dot(temp_vec6,
                                                 p_data[parent_col].a_hat);
      // q**
      p_data[cur_col].accel_angles[3] = (p_data[cur_col].Q -
                                         p_data[cur_col].accel_angles[3] -
                                         p_data[cur_col].scalar) /
                                         p_data[cur_col].s_inner_I_dot_s;

      mat6_mulv(p_data[cur_col].ST_from_parent, p_data[parent_col].a_hat,
                temp_vec6);
      vec6_add(temp_vec6, p_data[cur_col].coriolis_vector,
               p_data[cur_col].a_hat);
      vec6_scale(p_data[cur_col].s_hat, p_data[cur_col].accel_angles[3],
                 temp_vec6);

      // a_hat
      vec6_add(p_data[cur_col].a_hat, temp_vec6, p_data[cur_col].a_hat);
      for (int i = 0; i < 6; i++) {
        if (p_data[cur_col].a_hat[i] < 0.001 &&
            p_data[cur_col].a_hat[i] > -0.001) {
          p_data[cur_col].a_hat[i] = 0.0;
        }
      }
    }
  }

  return 0;
}

void compute_spatial_transformations(mat4 p_bone_to_world,
                                     vec3 p_coords,
                                     mat4 c_bone_to_world,
                                     vec3 c_coords,
                                     vec3 p_to_c_lin_dest,
                                     mat6 p_to_c_dest,
                                     mat6 c_to_p_dest) {
  // Matrix rotating vectors in world space to the parent bone space
  mat3 p_world_to_bone = GLM_MAT3_IDENTITY_INIT;
  // Remove translational component of mat4 since we only want rot matrix
  glm_mat4_pick3(p_bone_to_world, p_world_to_bone);
  glm_mat3_inv(p_world_to_bone, p_world_to_bone);

  // Matrix rotating vectors in world space to the child bone space
  mat3 c_world_to_bone = GLM_MAT3_IDENTITY_INIT;
  // Remove translational component of mat4 since we only want rot matrix
  glm_mat4_pick3(c_bone_to_world, c_world_to_bone);
  glm_mat3_inv(c_world_to_bone, c_world_to_bone);

  // Matrix rotating vectors in the child bone space to the parent bone space
  mat3 c_to_p_rot = GLM_MAT3_IDENTITY_INIT;
  glm_mat4_pick3(c_bone_to_world, c_to_p_rot);
  glm_mat3_mul(p_world_to_bone, c_to_p_rot, c_to_p_rot);

  // Inverse of above
  mat3 p_to_c_rot = GLM_MAT3_IDENTITY_INIT;
  glm_mat3_inv(c_to_p_rot, p_to_c_rot);

  // Linear component of the spatial transformation matrix transforming vectors
  // in the child bone space to the parent bone space
  mat3 p_to_c_mat = GLM_MAT3_IDENTITY_INIT;
  vec3 p_to_c_lin = GLM_VEC3_ZERO_INIT;
  // Parent to child in world coords
  glm_vec3_sub(c_coords, p_coords, p_to_c_lin);
  // Rotate to bone coords of parent
  glm_mat3_mulv(p_world_to_bone, p_to_c_lin, p_to_c_lin);
  vec3_singular_cross(p_to_c_lin, p_to_c_mat);

  // Linear component of the spatial transformation matrix transforming vectors
  // in the parent bone space to the child bone space
  mat3 c_to_p_mat = GLM_MAT3_IDENTITY_INIT;
  vec3 c_to_p_lin = GLM_VEC3_ZERO_INIT;
  // Child to parent in world coords
  glm_vec3_sub(p_coords, c_coords, c_to_p_lin);
  // Rotate to bone coords of child
  glm_mat3_mulv(c_world_to_bone, c_to_p_lin, c_to_p_lin);
  glm_vec3_negate_to(c_to_p_lin, p_to_c_lin_dest);
  vec3_singular_cross(c_to_p_lin, c_to_p_mat);

  // Spatial transformation matrix transforming vectors in the child bone's
  // coords to vectors in the current bone's coords
  mat6_spatial_transform(c_to_p_rot, p_to_c_mat, c_to_p_dest);

  // Spatial transformation matrix transforming vectors in the current
  // bone's coords to vectors in the child bone's coords
  mat6_spatial_transform(p_to_c_rot, c_to_p_mat, p_to_c_dest);
}

void compute_spatial_velocity(int cur_col, int parent_col, COLLIDER *colliders,
                              P_DATA *p_data) {
  // NOTATION NOTES
  // "v" denotes velocity
  // "va" denotes angular velocity
  // "v_hat" denotes spatial (six dimentional) velocity
  vec3 temp = GLM_VEC3_ZERO_INIT;

  vec6 parent_v_hat = VEC6_ZERO_INIT;
  vec3 v = GLM_VEC3_ZERO_INIT;
  vec3 va = GLM_VEC3_ZERO_INIT;
  if (parent_col != -1) {
    mat6_mulv(p_data[cur_col].ST_from_parent, p_data[parent_col].v_hat,
              parent_v_hat);

    // Because the angular component of a link is represented by the first half
    // of v_hat, we can just pass v_hat in as a vec3 to use the angular
    // velocity
    glm_vec3_copy(parent_v_hat, va);

    // The linear component is represented by the second half of v_hat, so we
    // extract the last half of v_hat when using linear velocity
    glm_vec3_copy(((float *) parent_v_hat) + 3, v);
  }

  // Accumulate velocities from prismatic DOFS
  // TODO Make this more flexible so more than just having the X-Axis as the
  // only degree of freedom
  glm_vec3_scale(BASIS_VECTORS[0], p_data[cur_col].vel_angles[3], temp);
  glm_vec3_add(temp, va, va);

  glm_vec3_cross(BASIS_VECTORS[0], p_data[cur_col].joint_to_com, temp);
  glm_vec3_scale(temp, p_data[cur_col].vel_angles[3], temp);
  glm_vec3_add(temp, v, v);

  vec6_compose(va, v, p_data[cur_col].v_hat);
}
