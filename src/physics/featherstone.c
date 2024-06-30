#include <physics/featherstone.h>

int featherstone_abm(ENTITY *body, vec3 grav) {
  size_t num_links = body->model->num_colliders;

  BONE *bones = body->model->bones;
  COLLIDER *colliders = body->model->colliders;
  int *bone_from_col = body->model->collider_bone_links;
  int *col_from_bone = body->model->bone_collider_links;
  P_DATA *p_data = body->np_data;

  int root_bone = -1;
  int parent_bone = -1;
  int parent_col = -1;

  // Calculate spatial velocities from inbound to outbound
  for (int cur_col = 0; cur_col < num_links; cur_col++) {
    if (colliders[cur_col].category != HURT_BOX) {
      continue;
    }

    root_bone = bone_from_col[cur_col];

    // Transforms entity space vectors of current link to world space
    mat4 ent_to_world = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_copy(body->final_b_mats[root_bone], ent_to_world);

    // Transforms bone space vectors of current link to world space
    mat4 bone_to_world = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_ins3(bones[root_bone].coordinate_matrix, bone_to_world);
    glm_mat4_mul(ent_to_world, bone_to_world, bone_to_world);

    // Transforms entity space vectors of current link to bone space of current
    // link
    mat4 ent_to_bone = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_ins3(bones[root_bone].coordinate_matrix, ent_to_bone);
    glm_mat4_transpose(ent_to_bone);

    // Uses entity space to world space because center/center_of_mass is given
    // in entity space
    // TODO joint_to_com can be precomputed
    vec3 cur_coords = GLM_VEC3_ZERO_INIT;
    if (colliders[cur_col].type == POLY) {
      glm_vec3_sub(colliders[cur_col].data.center_of_mass,
                   bones[root_bone].base, p_data[cur_col].joint_to_com);
      glm_mat4_mulv3(ent_to_world, colliders[cur_col].data.center_of_mass,
                     1.0, cur_coords);
    } else {
      glm_vec3_sub(colliders[cur_col].data.center, bones[root_bone].base,
                   p_data[cur_col].joint_to_com);
      glm_mat4_mulv3(ent_to_world, colliders[cur_col].data.center, 1.0,
                     cur_coords);
    }

    glm_mat4_mulv3(ent_to_bone, p_data[cur_col].joint_to_com, 1.0,
                   p_data[cur_col].joint_to_com);

    parent_bone = bones[root_bone].parent;
    parent_col = -1;
    if (parent_bone != -1) {
      // Transforms entity space vectors of parent link to world space
      mat4 p_ent_to_world = GLM_MAT4_IDENTITY_INIT;
      glm_mat4_copy(body->final_b_mats[parent_bone], p_ent_to_world);

      // Transforms bone space vectors of parent link to world space
      mat4 p_bone_to_world = GLM_MAT4_IDENTITY_INIT;
      glm_mat4_ins3(bones[parent_bone].coordinate_matrix, p_bone_to_world);
      glm_mat4_mul(p_ent_to_world, p_bone_to_world, p_bone_to_world);

      parent_col = col_from_bone[parent_bone];
      // Uses entity space to world space because center/center_of_mass is
      // given in entity space
      vec3 parent_coords = GLM_VEC3_ZERO_INIT;
      if (colliders[parent_col].type == POLY) {
        glm_mat4_mulv3(p_ent_to_world,
                       colliders[parent_col].data.center_of_mass, 1.0,
                       parent_coords);
      } else {
        glm_mat4_mulv3(p_ent_to_world, colliders[parent_col].data.center,
                       1.0, parent_coords);
      }

      compute_spatial_transformations(p_bone_to_world, parent_coords,
                                      bone_to_world, cur_coords,
                                      p_data[cur_col].from_parent_lin,
                                      p_data[cur_col].ST_from_parent,
                                      p_data[cur_col].ST_to_parent);

#ifdef DEBUG_FS
      // TESTING
      printf("%dX%d:\n", cur_col, parent_col);
      print_mat6(p_data[cur_col].ST_from_parent);
      printf("\n%dX%d:\n", parent_col, cur_col);
      print_mat6(p_data[cur_col].ST_to_parent);
      printf("\nv_hat[%d]:\n", cur_col);
      print_vec6(p_data[cur_col].v_hat);
#endif
    }

    compute_spatial_velocity(cur_col, parent_col, bone_to_world, colliders,
                             p_data);

#ifdef DEBUG_FS
    // TESTING
    printf("\ns[%d]:\n", cur_col);
    print_vec6(p_data[cur_col].s_hat);

    printf("\nv[%d]:\n", cur_col);
    print_vec6(p_data[cur_col].v_hat);

    printf("\nc[%d]:\n", cur_col);
    print_vec6(p_data[cur_col].coriolis_vector);
#endif
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
    glm_mat3_mulv(world_to_bone, grav, gravity);

    // Convert external force to bone space
    vec3 e_force = GLM_VEC3_ZERO_INIT;
    glm_mat3_mulv(world_to_bone, p_data[cur_col].e_force, e_force);

    vec3 za_linear = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(gravity, -mass, za_linear);
    glm_vec3_sub(za_linear, e_force, za_linear);

    float *ang_vel = (float *) p_data[cur_col].v_hat;
    vec3 za_ang = GLM_VEC3_ZERO_INIT;
    glm_mat3_mulv(inertia_tensor, ang_vel, za_ang);

    glm_vec3_cross(ang_vel, za_ang, za_ang);

    // Z_hat
    vec6_compose(za_linear, za_ang, p_data[cur_col].Z_hat);

#ifdef DEBUG_FS
    // TESTING
    printf("\nI_hat[%d]:\n", cur_col);
    print_mat6(p_data[cur_col].I_hat);

    printf("Z_hat[%d]:\n", cur_col);
    print_vec6(p_data[cur_col].Z_hat);
#endif
  }

  // Calculate I-hat-A and Z-hat-A from outbound to inbound
  for (int cur_col = num_links - 1; cur_col >= 0; cur_col--) {
    if (colliders[cur_col].category != HURT_BOX) {
      continue;
    }

    mat6_copy(p_data[cur_col].I_hat, p_data[cur_col].I_hat_A);
    vec6_copy(p_data[cur_col].Z_hat, p_data[cur_col].Z_hat_A);

    mat6 child_i_data = MAT6_ZERO_INIT;
    vec6 child_z_data = VEC6_ZERO_INIT;
    for (int cur_child = 0; cur_child < colliders[cur_col].num_children;
         cur_child++) {
      int child_col = colliders[cur_col].children_offset + cur_child;
      compute_articulated_data(child_col, p_data, child_i_data,
                               child_z_data);
#ifdef DEBUG_FS
      printf("Z_DATA[%d]:\n", child_col);
      print_vec6(child_z_data);
#endif
      vec6_add(p_data[cur_col].Z_hat_A, child_z_data,
               p_data[cur_col].Z_hat_A);
      mat6_add(p_data[cur_col].I_hat_A, child_i_data,
               p_data[cur_col].I_hat_A);
    }

#ifdef DEBUG_FS
    // TESTING
    printf("\nI_A[%d]:\n", cur_col);
    print_mat6(p_data[cur_col].I_hat_A);
    printf("Z_A[%d]:\n", cur_col);
    print_vec6(p_data[cur_col].Z_hat_A);
#endif
  }

  // Calculate q** and spatial acceleration from inbound to outbound
  for (int cur_col = 0; cur_col < num_links; cur_col++) {
    if (colliders[cur_col].category != HURT_BOX) {
      continue;
    }

    vec6_zero(p_data[cur_col].a_hat);

    int root_bone = bone_from_col[cur_col];
    parent_bone = bones[root_bone].parent;

    vec6 temp_vec6 = VEC6_ZERO_INIT;
    if (parent_bone != -1) {
      int parent_col = col_from_bone[parent_bone];
      mat6_mulv(p_data[cur_col].ST_from_parent, p_data[parent_col].a_hat,
                temp_vec6);
      p_data[cur_col].accel_angle = vec6_dot(p_data[cur_col].s_inner_I,
                                             temp_vec6);

      // q**
      p_data[cur_col].accel_angle = (-p_data[cur_col].accel_angle -
                                     p_data[cur_col].SZI) /
                                    p_data[cur_col].s_inner_I_dot_s;

      mat6_mulv(p_data[cur_col].ST_from_parent, p_data[parent_col].a_hat,
                temp_vec6);
#ifdef DEBUG_FS
      printf("\nA_DATA[%d]:\n", cur_col);
      printf("  %dX%d * a[%d]:\n", cur_col, parent_col, parent_col);
      print_vec6(temp_vec6);
#endif
      vec6_add(temp_vec6, p_data[cur_col].coriolis_vector,
               p_data[cur_col].a_hat);
#ifdef DEBUG_FS
      printf("  c[%d]:\n", cur_col);
      print_vec6(p_data[cur_col].coriolis_vector);
#endif
      vec6_scale(p_data[cur_col].s_hat, p_data[cur_col].accel_angle,
                 temp_vec6);
#ifdef DEBUG_FS
      printf("  q[%d] * s[%d]:\n", cur_col, cur_col);
      print_vec6(temp_vec6);
#endif

      // a_hat
      vec6_add(p_data[cur_col].a_hat, temp_vec6, p_data[cur_col].a_hat);
      vec6_remove_noise(p_data[cur_col].a_hat, 0.001);

      // Calculate world-space acceleration
      mat3 bone_to_world = GLM_MAT3_IDENTITY_INIT;
      glm_mat4_pick3(body->final_b_mats[root_bone], bone_to_world);
      glm_mat3_mul(bone_to_world, bones[root_bone].coordinate_matrix,
                   bone_to_world);
      glm_mat3_mulv(bone_to_world, p_data[cur_col].a_hat,
                    p_data[cur_col].ang_a);
      glm_mat3_mulv(bone_to_world, ((float *)p_data[cur_col].a_hat)+3,
                    p_data[cur_col].a);

#ifdef DEBUG_FS
      printf("q[%d]:\n%f\n", cur_col, p_data[cur_col].accel_angles);
      printf("\na[%d]:\n", cur_col);
      print_vec6(p_data[cur_col].a_hat);
#endif
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

void compute_spatial_velocity(int cur_col, int parent_col, mat4 bone_to_world,
                              COLLIDER *colliders, P_DATA *p_data) {
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

  // Spatial Joint Axis
  glm_vec3_cross(p_data[cur_col].dof, p_data[cur_col].joint_to_com,
                 temp);
  vec6_compose(p_data[cur_col].dof, temp, p_data[cur_col].s_hat);

  // Coriolis vector
  vec3 d = GLM_VEC3_ZERO_INIT;
  glm_vec3_copy(p_data[cur_col].joint_to_com, d);

  vec3 r = GLM_VEC3_ZERO_INIT;
  glm_vec3_copy(p_data[cur_col].from_parent_lin, r);

  vec6_zero(p_data[cur_col].coriolis_vector);
  vec3 u_scaled = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(p_data[cur_col].dof, p_data[cur_col].vel_angle, u_scaled);

  vec3 coriolis_first = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(va, u_scaled, coriolis_first);

  vec3 u_scaled_cross_d = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(u_scaled, d, u_scaled_cross_d);

  vec3 va_cross_r = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(va, r, va_cross_r);

  vec3 coriolis_last = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(va, 2.0, coriolis_last);
  glm_vec3_cross(coriolis_last, u_scaled_cross_d, coriolis_last);

  glm_vec3_cross(va, va_cross_r, temp);
  glm_vec3_add(coriolis_last, temp, coriolis_last);

  glm_vec3_cross(u_scaled, u_scaled_cross_d, temp);
  glm_vec3_add(coriolis_last, temp, coriolis_last);

  vec6_compose(coriolis_first, coriolis_last,
               p_data[cur_col].coriolis_vector);

  // Accumulate velocity
  glm_vec3_scale(p_data[cur_col].dof, p_data[cur_col].vel_angle,
                 temp);
  glm_vec3_add(temp, va, va);

  glm_vec3_cross(p_data[cur_col].dof, p_data[cur_col].joint_to_com,
                 temp);
  glm_vec3_scale(temp, p_data[cur_col].vel_angle, temp);
  glm_vec3_add(temp, v, v);

  vec6_compose(va, v, p_data[cur_col].v_hat);

  // Calculate world-space velocity
  mat3 b_to_w = GLM_MAT3_IDENTITY_INIT;
  glm_mat4_pick3(bone_to_world, b_to_w);
  glm_mat3_mulv(b_to_w, v, p_data[cur_col].v);
  glm_mat3_mulv(b_to_w, va, p_data[cur_col].ang_v);
}

void compute_articulated_data(int col, P_DATA *p_data, mat6 i_dest,
                              vec6 z_dest) {
  vec6 temp_vec6 = VEC6_ZERO_INIT;
  mat6 temp_mat6 = MAT6_ZERO_INIT;

  // ========== Articulated I data ===========
  vec6 I_hat_s_hat = VEC6_ZERO_INIT;
  // Is
  mat6_mulv(p_data[col].I_hat_A, p_data[col].s_hat, I_hat_s_hat);

  // Iss'
  vec6_spatial_transpose_mulv(I_hat_s_hat, p_data[col].s_hat,
                              temp_mat6);

  // Iss'I
  mat6_mul(temp_mat6, p_data[col].I_hat_A, temp_mat6);

  // s'I
  vec6_spatial_transpose_mulm(p_data[col].s_hat, p_data[col].I_hat_A,
                              p_data[col].s_inner_I);

  // s'Is
  p_data[col].s_inner_I_dot_s = vec6_dot(p_data[col].s_inner_I,
                                         p_data[col].s_hat);

  mat6_scale(temp_mat6, 1.0 / p_data[col].s_inner_I_dot_s, temp_mat6);

  mat6_sub(p_data[col].I_hat_A, temp_mat6, i_dest);

  mat6_mul(p_data[col].ST_to_parent, i_dest, i_dest);
  mat6_mul(i_dest, p_data[col].ST_from_parent, i_dest);

  // ============ Articulated Z data ============
  vec6 I_hat_coriolis = VEC6_ZERO_INIT;
  mat6_mulv(p_data[col].I_hat_A, p_data[col].coriolis_vector, I_hat_coriolis);

  vec6_add(p_data[col].Z_hat_A, I_hat_coriolis, temp_vec6);

  // s'(Z_hat_A + I_hat_coriolis))
  p_data[col].SZI = vec6_inner_product(p_data[col].s_hat, temp_vec6);

  float scalar = -p_data[col].SZI / p_data[col].s_inner_I_dot_s;

  vec6_scale(I_hat_s_hat, scalar, temp_vec6);
  vec6_add(temp_vec6, I_hat_coriolis, temp_vec6);
  vec6_add(temp_vec6, p_data[col].Z_hat_A, z_dest);

  mat6_mulv(p_data[col].ST_to_parent, z_dest, z_dest);
}
