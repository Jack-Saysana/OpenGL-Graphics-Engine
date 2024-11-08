#include <physics/featherstone.h>

int featherstone_abm(ENTITY *body, vec3 grav) {
  size_t num_links = body->model->num_colliders;

  BONE *bones = body->model->bones;
  COLLIDER *colliders = body->model->colliders;
  int *bone_from_col = body->model->collider_bone_links;
  int *col_from_bone = body->model->bone_collider_links;
  P_DATA *p_data = body->np_data;
  ZERO_JOINT *z_data = body->zj_data;

  int root_bone = -1;
  int parent_bone = -1;
  int parent_col = -1;

  // Calculate spatial velocities from inbound to outbound
  for (int cur_col = 0; cur_col < num_links; cur_col++) {
    root_bone = bone_from_col[cur_col];
    parent_bone = bones[root_bone].parent;

    mat4 c_ent_to_world = GLM_MAT4_IDENTITY_INIT;
    mat4 p_ent_to_world = GLM_MAT4_IDENTITY_INIT;
    mat4 p_bone_to_world = GLM_MAT4_IDENTITY_INIT;
    mat4 c_bone_to_world = GLM_MAT4_IDENTITY_INIT;
    vec3 p_coords = GLM_VEC3_ZERO_INIT;
    vec3 c_coords = GLM_VEC3_ZERO_INIT;
    vec3 placeholder = GLM_VEC3_ZERO_INIT;
    vec3 ent_zj_dof = GLM_VEC3_ZERO_INIT;

    if (parent_bone != -1) {
      // Calculate first p_bone_to_world
      parent_col = col_from_bone[parent_bone];
      glm_mat4_copy(body->final_b_mats[parent_bone], p_ent_to_world);
      glm_mat4_ins3(bones[parent_bone].coordinate_matrix, p_bone_to_world);
      glm_mat4_mul(p_ent_to_world, p_bone_to_world, p_bone_to_world);

      if (colliders[parent_col].type == POLY) {
        glm_mat4_mulv3(p_ent_to_world,
                       colliders[parent_col].data.center_of_mass, 1.0,
                       p_coords);
      } else {
        glm_mat4_mulv3(p_ent_to_world, colliders[parent_col].data.center, 1.0,
                       p_coords);
      }
    }

    // Calculate bone_to_world for current joint
    glm_mat4_copy(body->final_b_mats[root_bone], c_ent_to_world);
    glm_mat4_ins3(bones[root_bone].coordinate_matrix, c_bone_to_world);
    glm_mat4_mul(c_ent_to_world, c_bone_to_world, c_bone_to_world);

    if (colliders[cur_col].type == POLY) {
      glm_vec3_sub(colliders[cur_col].data.center_of_mass,
                   bones[root_bone].base, p_data[cur_col].joint_to_com);
      glm_mat4_mulv3(c_ent_to_world, colliders[cur_col].data.center_of_mass,
                     1.0, c_coords);
    } else {
      glm_vec3_sub(colliders[cur_col].data.center, bones[root_bone].base,
                   p_data[cur_col].joint_to_com);
      glm_mat4_mulv3(c_ent_to_world, colliders[cur_col].data.center, 1.0,
                     c_coords);
    }

    mat3 c_ent_to_bone = GLM_MAT3_IDENTITY_INIT;
    glm_mat3_transpose_to(bones[root_bone].coordinate_matrix, c_ent_to_bone);
    glm_mat3_mulv(c_ent_to_bone, p_data[cur_col].joint_to_com,
                  p_data[cur_col].joint_to_com);

    if (p_data[cur_col].num_z_joints) {
      // TODO Scale joints joint_to_com small to be the z joints joint_to_com
      mat4 joint_bone_to_world = GLM_MAT4_IDENTITY_INIT;
      vec3 joint_coords = GLM_VEC3_ZERO_INIT;
      vec3 p_head = GLM_VEC3_ZERO_INIT;
      glm_mat4_copy(c_bone_to_world, joint_bone_to_world);
      glm_vec3_copy(c_coords, joint_coords);
      size_t cur_zj = p_data[cur_col].zero_joint_offset;

      // First zero joint velocity calculated from parent joint
      if (parent_bone != -1) {
        glm_mat4_mulv3(body->final_b_mats[parent_bone],
                       bones[parent_bone].head, 1.0, p_head);
        glm_mat3_mulv(bones[root_bone].coordinate_matrix, z_data[cur_zj].dof,
                      ent_zj_dof);
        calc_zj_ent_to_world(body->final_b_mats[parent_bone], p_head,
                             ent_zj_dof, z_data[cur_zj].joint_angle,
                             z_data[cur_zj].joint_type, c_ent_to_world);
        glm_mat4_mulv3(c_ent_to_world, bones[parent_bone].head, 1.0, c_coords);
        glm_mat4_ins3(bones[root_bone].coordinate_matrix, c_bone_to_world);
        glm_mat4_mul(c_ent_to_world, c_bone_to_world, c_bone_to_world);
        glm_mat4_pick3(c_bone_to_world, z_data[cur_zj].bone_to_world);

        // Set zero joint's joint_to_com to a scaled down version of the main
        // joint's joint_to_com
        glm_vec3_scale_as(p_data[cur_col].joint_to_com, 0.01,
                          z_data[cur_zj].joint_to_com);

        // First zero_joint located at head of parent joint
        parent_col = col_from_bone[parent_bone];
        compute_spatial_transformations(p_bone_to_world, p_coords,
                                        c_bone_to_world, c_coords,
                                        placeholder,
                                        z_data[cur_zj].ST_from_parent,
                                        z_data[cur_zj].ST_to_parent);
        calc_spatial_vel(z_data[cur_zj].ST_from_parent,
                         p_data[parent_col].v_hat, z_data[cur_zj].dof,
                         z_data[cur_zj].joint_to_com, GLM_VEC3_ZERO,
                         z_data[cur_zj].vel_angle, z_data[cur_zj].joint_type,
                         z_data[cur_zj].s_hat, z_data[cur_zj].c_hat,
                         z_data[cur_zj].v_hat);
      } else {
        glm_mat4_identity(c_ent_to_world);
        glm_mat4_ins3(bones[root_bone].coordinate_matrix, c_bone_to_world);
        glm_mat3_copy(bones[root_bone].coordinate_matrix,
                      z_data[cur_zj].bone_to_world);
        glm_mat4_mulv3(body->final_b_mats[root_bone], bones[root_bone].base,
                       1.0, c_coords);
      }

      // Accumulate spatial velocity across intermediate zero joints
      for (int i = 1; i < p_data[cur_col].num_z_joints; i++) {
        cur_zj = p_data[cur_col].zero_joint_offset + i;
        glm_mat4_copy(c_ent_to_world, p_ent_to_world);
        glm_mat4_copy(c_bone_to_world, p_bone_to_world);
        glm_vec3_copy(c_coords, p_coords);
        glm_mat3_mulv(bones[root_bone].coordinate_matrix, z_data[cur_zj].dof,
                      ent_zj_dof);
        calc_zj_ent_to_world(p_ent_to_world, p_coords,
                             ent_zj_dof, z_data[cur_zj].joint_angle,
                             z_data[cur_zj].joint_type, c_ent_to_world);
        glm_mat4_mulv3(c_ent_to_world, p_coords, 1.0, c_coords);
        glm_mat4_ins3(bones[root_bone].coordinate_matrix, c_bone_to_world);
        glm_mat4_mul(c_ent_to_world, c_bone_to_world, c_bone_to_world);
        glm_mat4_pick3(c_bone_to_world, z_data[cur_zj].bone_to_world);

        glm_vec3_scale_as(p_data[cur_col].joint_to_com, 0.01,
                          z_data[cur_zj].joint_to_com);

        compute_spatial_transformations(p_bone_to_world, p_coords,
                                        c_bone_to_world, c_coords,
                                        placeholder,
                                        z_data[cur_zj].ST_from_parent,
                                        z_data[cur_zj].ST_to_parent);
        calc_spatial_vel(z_data[cur_zj].ST_from_parent,
                         z_data[cur_zj - 1].v_hat, z_data[cur_zj].dof,
                         z_data[cur_zj].joint_to_com, GLM_VEC3_ZERO,
                         z_data[cur_zj].vel_angle, z_data[cur_zj].joint_type,
                         z_data[cur_zj].s_hat, z_data[cur_zj].c_hat,
                         z_data[cur_zj].v_hat);
      }

      // Calc spatial velocity from last zero joint
      glm_mat4_copy(c_bone_to_world, p_bone_to_world);
      glm_vec3_copy(c_coords, p_coords);
      cur_zj = p_data[cur_col].zero_joint_offset +
               (p_data[cur_col].num_z_joints - 1);
      compute_spatial_transformations(p_bone_to_world, p_coords,
                                      joint_bone_to_world, joint_coords,
                                      p_data[cur_col].from_parent_lin,
                                      p_data[cur_col].ST_from_parent,
                                      p_data[cur_col].ST_to_parent);
      calc_spatial_vel(p_data[cur_col].ST_from_parent,
                       z_data[cur_zj].v_hat, p_data[cur_col].dof,
                       p_data[cur_col].joint_to_com,
                       p_data[cur_col].from_parent_lin,
                       p_data[cur_col].vel_angle, p_data[cur_col].joint_type,
                       p_data[cur_col].s_hat, p_data[cur_col].coriolis_vector,
                       p_data[cur_col].v_hat);
      glm_mat4_copy(joint_bone_to_world, c_bone_to_world);
    } else if (parent_bone != -1) {
      // Calc spatial velocity directly from parent joint
      parent_col = col_from_bone[parent_bone];
      compute_spatial_transformations(p_bone_to_world, p_coords,
                                      c_bone_to_world, c_coords,
                                      p_data[cur_col].from_parent_lin,
                                      p_data[cur_col].ST_from_parent,
                                      p_data[cur_col].ST_to_parent);
      calc_spatial_vel(p_data[cur_col].ST_from_parent,
                       p_data[parent_col].v_hat, p_data[cur_col].dof,
                       p_data[cur_col].joint_to_com,
                       p_data[cur_col].from_parent_lin,
                       p_data[cur_col].vel_angle, p_data[cur_col].joint_type,
                       p_data[cur_col].s_hat, p_data[cur_col].coriolis_vector,
                       p_data[cur_col].v_hat);
    }

    // Calculate world-space velocity
    mat3 b_to_w = GLM_MAT3_IDENTITY_INIT;
    glm_mat4_pick3(c_bone_to_world, b_to_w);
    glm_mat3_mulv(b_to_w, p_data[cur_col].v_hat, p_data[cur_col].ang_v);
    glm_mat3_mulv(b_to_w, ((float *)p_data[cur_col].v_hat)+3,
                  p_data[cur_col].v);
  }

  // TODO I_hat can be precomputed
  // Calculate I-hat and Z-hat from inbound to outbound
  for (int cur_col = 0; cur_col < num_links; cur_col++) {
    // Calculate Z-hat of zero joints, leave I-hat zero
    if (p_data[cur_col].num_z_joints) {
      int cur_zj = 0;
      for (int i = 0; i < p_data[cur_col].num_z_joints; i++) {
        cur_zj = p_data[cur_col].zero_joint_offset + i;

        mat3 world_to_bone = GLM_MAT3_IDENTITY_INIT;
        glm_mat3_transpose_to(z_data[cur_zj].bone_to_world, world_to_bone);

        // Convert gravity to bone space
        vec3 gravity = GLM_VEC3_ZERO_INIT;
        glm_mat3_mulv(world_to_bone, grav, gravity);
        glm_vec3_scale(gravity, -0.01, z_data[cur_zj].Z_hat);
      }
    }

    root_bone = bone_from_col[cur_col];

    mat3 ent_to_world = GLM_MAT3_IDENTITY_INIT;
    mat3 bone_to_ent = GLM_MAT3_IDENTITY_INIT;
    mat3 inv_inertia = GLM_MAT3_IDENTITY_INIT;

    glm_mat3_copy(bones[root_bone].coordinate_matrix, bone_to_ent);
    glm_mat4_pick3(body->final_b_mats[root_bone], ent_to_world);
    glm_mat4_pick3(p_data[cur_col].inv_inertia, inv_inertia);
    calc_IZ_hat(ent_to_world, bone_to_ent, inv_inertia, grav,
                p_data[cur_col].e_force, (float *) p_data[cur_col].v_hat,
                p_data[cur_col].inv_mass, p_data[cur_col].I_hat,
                p_data[cur_col].Z_hat);
  }

  // Calculate I-hat-A and Z-hat-A from outbound to inbound
  for (int cur_col = num_links - 1; cur_col >= 0; cur_col--) {
    mat6_copy(p_data[cur_col].I_hat, p_data[cur_col].I_hat_A);
    vec6_copy(p_data[cur_col].Z_hat, p_data[cur_col].Z_hat_A);

    mat6 child_i_data = MAT6_ZERO_INIT;
    vec6 child_z_data = VEC6_ZERO_INIT;
    for (int cur_child = 0; cur_child < colliders[cur_col].num_children;
         cur_child++) {
      // If child has zero-joints, then use the first zero-joint as the child
      // joint
      int child_col = colliders[cur_col].children_offset + cur_child;
      if (p_data[child_col].num_z_joints) {
        child_col = p_data[child_col].zero_joint_offset;
        calc_articulated_child(z_data[child_col].ST_to_parent,
                               z_data[child_col].ST_from_parent,
                               z_data[child_col].I_hat_A,
                               z_data[child_col].Z_hat_A,
                               z_data[child_col].c_hat,
                               z_data[child_col].s_hat, 0.0, child_i_data,
                               child_z_data, z_data[child_col].s_inner_I,
                               &z_data[child_col].s_inner_I_dot_s,
                               &z_data[child_col].SZI);
      } else {
        calc_articulated_child(p_data[child_col].ST_to_parent,
                               p_data[child_col].ST_from_parent,
                               p_data[child_col].I_hat_A,
                               p_data[child_col].Z_hat_A,
                               p_data[child_col].coriolis_vector,
                               p_data[child_col].s_hat, 0.0, child_i_data,
                               child_z_data, p_data[child_col].s_inner_I,
                               &p_data[child_col].s_inner_I_dot_s,
                               &p_data[child_col].SZI);
      }

      vec6_add(p_data[cur_col].Z_hat_A, child_z_data,
               p_data[cur_col].Z_hat_A);
      mat6_add(p_data[cur_col].I_hat_A, child_i_data,
               p_data[cur_col].I_hat_A);
    }

    // Accumulated articulated data for zero joints
    for (int i = p_data[cur_col].num_z_joints - 1; i >= 0; i--) {
      int cur_zj = p_data[cur_col].zero_joint_offset + i;
      if (i == p_data[cur_col].num_z_joints - 1) {
        calc_articulated_child(p_data[cur_col].ST_to_parent,
                               p_data[cur_col].ST_from_parent,
                               p_data[cur_col].I_hat_A,
                               p_data[cur_col].Z_hat_A,
                               p_data[cur_col].coriolis_vector,
                               p_data[cur_col].s_hat, 0.0,
                               z_data[cur_zj].I_hat_A,
                               z_data[cur_zj].Z_hat_A,
                               p_data[cur_col].s_inner_I,
                               &p_data[cur_col].s_inner_I_dot_s,
                               &p_data[cur_col].SZI);
        vec6_add(z_data[cur_zj].Z_hat_A, z_data[cur_zj].Z_hat,
                 z_data[cur_zj].Z_hat_A);
      } else {
        ZERO_JOINT *zj = z_data + (cur_zj + 1);
        calc_articulated_child(zj->ST_to_parent, zj->ST_from_parent,
                               zj->I_hat_A, zj->Z_hat_A, zj->c_hat, zj->s_hat,
                               0.0, z_data[cur_zj].I_hat_A,
                               z_data[cur_zj].Z_hat_A, zj->s_inner_I,
                               &zj->s_inner_I_dot_s, &zj->SZI);
        vec6_add(z_data[cur_zj].Z_hat_A, z_data[cur_zj].Z_hat,
                 z_data[cur_zj].Z_hat_A);
      }
    }
  }

  // Calculate q** and spatial acceleration from inbound to outbound
  for (int cur_col = 0; cur_col < num_links; cur_col++) {
    vec6_zero(p_data[cur_col].a_hat);
    int root_bone = bone_from_col[cur_col];
    parent_bone = bones[root_bone].parent;
    parent_col = -1;

    // Accumulate acceleration of zero joints
    int cur_zj = 0;
    if (p_data[cur_col].num_z_joints) {
      cur_zj = p_data[cur_col].zero_joint_offset;
      if (parent_bone != -1) {
        parent_col = col_from_bone[parent_bone];
        calc_a_hat(z_data[cur_zj].ST_from_parent, p_data[parent_col].a_hat,
                   z_data[cur_zj].c_hat, z_data[cur_zj].s_hat,
                   z_data[cur_zj].s_inner_I, z_data[cur_zj].SZI,
                   z_data[cur_zj].s_inner_I_dot_s, z_data[cur_zj].a_hat,
                   &z_data[cur_zj].accel_angle);
      }
      for (size_t i = 1; i < p_data[cur_col].num_z_joints; i++) {
        cur_zj = p_data[cur_col].zero_joint_offset + i;
        ZERO_JOINT *zj = z_data + (cur_zj - 1);
        calc_a_hat(z_data[cur_zj].ST_from_parent, zj->a_hat,
                   z_data[cur_zj].c_hat, z_data[cur_zj].s_hat,
                   z_data[cur_zj].s_inner_I, z_data[cur_zj].SZI,
                   z_data[cur_zj].s_inner_I_dot_s, z_data[cur_zj].a_hat,
                   &z_data[cur_zj].accel_angle);
      }

      // Calculate acceleration for current joint
      cur_zj = p_data[cur_col].zero_joint_offset +
               p_data[cur_col].num_z_joints - 1;
      calc_a_hat(p_data[cur_col].ST_from_parent, z_data[cur_zj].a_hat,
                 p_data[cur_col].coriolis_vector, p_data[cur_col].s_hat,
                 p_data[cur_col].s_inner_I, p_data[cur_col].SZI,
                 p_data[cur_col].s_inner_I_dot_s, p_data[cur_col].a_hat,
                 &p_data[cur_col].accel_angle);
    } else if (parent_bone != -1) {
      parent_col = col_from_bone[parent_bone];
      calc_a_hat(p_data[cur_col].ST_from_parent, p_data[parent_col].a_hat,
                 p_data[cur_col].coriolis_vector, p_data[cur_col].s_hat,
                 p_data[cur_col].s_inner_I, p_data[cur_col].SZI,
                 p_data[cur_col].s_inner_I_dot_s, p_data[cur_col].a_hat,
                 &p_data[cur_col].accel_angle);
    }

    // Calculate world-space acceleration
    mat3 bone_to_world = GLM_MAT3_IDENTITY_INIT;
    glm_mat4_pick3(body->final_b_mats[root_bone], bone_to_world);
    glm_mat3_mul(bone_to_world, bones[root_bone].coordinate_matrix,
                 bone_to_world);
    glm_mat3_mulv(bone_to_world, p_data[cur_col].a_hat,
                  p_data[cur_col].ang_a);
    vec3 lin_accel = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(p_data[cur_col].a_hat, p_data[cur_col].joint_to_com,
                   lin_accel);
    glm_vec3_add(lin_accel, ((float *)p_data[cur_col].a_hat)+3, lin_accel);
    glm_mat3_mulv(bone_to_world, lin_accel, p_data[cur_col].a);
  }

  /*
  fprintf(stderr, "a: %f, %f, %f\n", p_data[0].a[0], p_data[0].a[1],
          p_data[0].a[2]);
  fprintf(stderr, "ang_a: %f, %f, %f\n", p_data[0].ang_a[0],
          p_data[0].ang_a[1], p_data[0].ang_a[2]);
  */
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
  // TODO Calculate s_hat for all zero joints
  // TODO Compound calculating coriolis vector and velocities of zero joints

  // Spatial Joint Axis
  if (p_data[cur_col].joint_type == JOINT_REVOLUTE) {
    glm_vec3_cross(p_data[cur_col].dof, p_data[cur_col].joint_to_com,
                   temp);
    vec6_compose(p_data[cur_col].dof, temp, p_data[cur_col].s_hat);
  } else {
    vec6_compose(GLM_VEC3_ZERO, p_data[cur_col].dof, p_data[cur_col].s_hat);
  }

  // Coriolis vector
  vec6_zero(p_data[cur_col].coriolis_vector);
  vec3 coriolis_first = GLM_VEC3_ZERO_INIT;
  vec3 coriolis_last = GLM_VEC3_ZERO_INIT;

  vec3 r = GLM_VEC3_ZERO_INIT;
  glm_vec3_copy(p_data[cur_col].from_parent_lin, r);

  vec3 u_scaled = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(p_data[cur_col].dof, p_data[cur_col].vel_angle, u_scaled);

  vec3 va_cross_r = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(va, r, va_cross_r);
  if (p_data[cur_col].joint_type == JOINT_REVOLUTE) {
    vec3 d = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(p_data[cur_col].joint_to_com, d);
    glm_vec3_cross(va, u_scaled, coriolis_first);

    vec3 u_scaled_cross_d = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(u_scaled, d, u_scaled_cross_d);
    glm_vec3_scale(va, 2.0, coriolis_last);
    glm_vec3_cross(coriolis_last, u_scaled_cross_d, coriolis_last);
    glm_vec3_cross(va, va_cross_r, temp);
    glm_vec3_add(coriolis_last, temp, coriolis_last);
    glm_vec3_cross(u_scaled, u_scaled_cross_d, temp);
    glm_vec3_add(coriolis_last, temp, coriolis_last);
  } else {
    glm_vec3_scale(va, 2.0, coriolis_last);
    glm_vec3_cross(coriolis_last, u_scaled, coriolis_last);
    glm_vec3_cross(va, va_cross_r, temp);
    glm_vec3_add(coriolis_last, temp, coriolis_last);
  }
  vec6_compose(coriolis_first, coriolis_last,
               p_data[cur_col].coriolis_vector);

  // Accumulate velocity
  if (p_data[cur_col].joint_type == JOINT_REVOLUTE) {
    glm_vec3_scale(p_data[cur_col].dof, p_data[cur_col].vel_angle,
                   temp);
    glm_vec3_add(temp, va, va);

    glm_vec3_cross(temp, p_data[cur_col].joint_to_com,
                   temp);
    glm_vec3_add(temp, v, v);
  } else {
    glm_vec3_scale(p_data[cur_col].dof, p_data[cur_col].vel_angle,
                   temp);
    glm_vec3_add(temp, v, v);
  }

  vec6_compose(va, v, p_data[cur_col].v_hat);

  // Calculate world-space velocity
  mat3 b_to_w = GLM_MAT3_IDENTITY_INIT;
  glm_mat4_pick3(bone_to_world, b_to_w);
  glm_mat3_mulv(b_to_w, p_data[cur_col].v_hat, p_data[cur_col].ang_v);
  vec3 lin_accel = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(p_data[cur_col].v_hat, p_data[cur_col].joint_to_com,
                 lin_accel);
  glm_vec3_add(lin_accel, ((float *)p_data[cur_col].v_hat)+3, lin_accel);
  glm_mat3_mulv(b_to_w, lin_accel, p_data[cur_col].v);
}

void calc_spatial_vel(mat6 from_parent, vec6 p_v_hat, vec3 u, vec3 d, vec3 r,
                      float vel_angle, int joint_type, vec6 s_hat_dest,
                      vec6 c_hat_dest, vec6 dest) {
  vec3 temp = GLM_VEC3_ZERO_INIT;

  vec6 parent_v_hat = VEC6_ZERO_INIT;
  vec3 v = GLM_VEC3_ZERO_INIT;
  vec3 va = GLM_VEC3_ZERO_INIT;

  mat6_mulv(from_parent, p_v_hat, parent_v_hat);

  // Because the angular component of a link is represented by the first half
  // of v_hat, we can just pass v_hat in as a vec3 to use the angular
  // velocity
  glm_vec3_copy(parent_v_hat, va);

  // The linear component is represented by the second half of v_hat, so we
  // extract the last half of v_hat when using linear velocity
  glm_vec3_copy(((float *) parent_v_hat) + 3, v);

  // Spatial Joint Axis
  if (joint_type == JOINT_REVOLUTE) {
    glm_vec3_cross(u, d, temp);
    vec6_compose(u, temp, s_hat_dest);
  } else {
    vec6_compose(GLM_VEC3_ZERO, u, s_hat_dest);
  }

  // Coriolis vector
  vec6_zero(c_hat_dest);
  vec3 coriolis_first = GLM_VEC3_ZERO_INIT;
  vec3 coriolis_last = GLM_VEC3_ZERO_INIT;

  vec3 u_scaled = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(u, vel_angle, u_scaled);

  vec3 va_cross_r = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(va, r, va_cross_r);
  if (joint_type == JOINT_REVOLUTE) {
    glm_vec3_cross(va, u_scaled, coriolis_first);

    vec3 u_scaled_cross_d = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(u_scaled, d, u_scaled_cross_d);
    glm_vec3_scale(va, 2.0, coriolis_last);
    glm_vec3_cross(coriolis_last, u_scaled_cross_d, coriolis_last);
    glm_vec3_cross(va, va_cross_r, temp);
    glm_vec3_add(coriolis_last, temp, coriolis_last);
    glm_vec3_cross(u_scaled, u_scaled_cross_d, temp);
    glm_vec3_add(coriolis_last, temp, coriolis_last);
  } else {
    glm_vec3_scale(va, 2.0, coriolis_last);
    glm_vec3_cross(coriolis_last, u_scaled, coriolis_last);
    glm_vec3_cross(va, va_cross_r, temp);
    glm_vec3_add(coriolis_last, temp, coriolis_last);
  }
  vec6_compose(coriolis_first, coriolis_last, c_hat_dest);

  // Accumulate velocity
  if (joint_type == JOINT_REVOLUTE) {
    glm_vec3_scale(u, vel_angle, temp);
    glm_vec3_add(temp, va, va);

    glm_vec3_cross(temp, d, temp);
    glm_vec3_add(temp, v, v);
  } else {
    glm_vec3_scale(u, vel_angle, temp);
    glm_vec3_add(temp, v, v);
  }

  vec6_compose(va, v, dest);
}

void calc_coriolis(vec3 va, vec3 u, vec3 r, vec3 d, float vel_angle,
                   int joint_type, vec6 dest) {
  vec6_zero(dest);
  vec3 coriolis_first = GLM_VEC3_ZERO_INIT;
  vec3 coriolis_last = GLM_VEC3_ZERO_INIT;
  vec3 temp = GLM_VEC3_ZERO_INIT;

  vec3 u_scaled = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(u, vel_angle, u_scaled);

  vec3 va_cross_r = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(va, r, va_cross_r);
  if (joint_type == JOINT_REVOLUTE) {
    glm_vec3_cross(va, u_scaled, coriolis_first);

    vec3 u_scaled_cross_d = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(u_scaled, d, u_scaled_cross_d);
    glm_vec3_scale(va, 2.0, coriolis_last);
    glm_vec3_cross(coriolis_last, u_scaled_cross_d, coriolis_last);
    glm_vec3_cross(va, va_cross_r, temp);
    glm_vec3_add(coriolis_last, temp, coriolis_last);
    glm_vec3_cross(u_scaled, u_scaled_cross_d, temp);
    glm_vec3_add(coriolis_last, temp, coriolis_last);
  } else {
    glm_vec3_scale(va, 2.0, coriolis_last);
    glm_vec3_cross(coriolis_last, u_scaled, coriolis_last);
    glm_vec3_cross(va, va_cross_r, temp);
    glm_vec3_add(coriolis_last, temp, coriolis_last);
  }
  vec6_compose(coriolis_first, coriolis_last, dest);
}

void calc_IZ_hat(mat3 ent_to_world, mat3 bone_to_ent, mat3 inv_inertia,
                 vec3 grav, vec6 e_forces, vec3 ang_vel, float inv_mass,
                 mat6 i_dest, vec6 z_dest) {
  // M
  float mass = 1.0 / inv_mass;
  mat3 mass_mat = GLM_MAT3_IDENTITY_INIT;
  glm_mat3_scale(mass_mat, mass);

  // I
  mat3 bone_to_world = GLM_MAT3_IDENTITY_INIT;
  mat3 world_to_bone = GLM_MAT3_IDENTITY_INIT;
  glm_mat3_mul(ent_to_world, bone_to_ent, bone_to_world);
  glm_mat3_transpose_to(bone_to_world, world_to_bone);

  mat3 inertia_tensor = GLM_MAT3_IDENTITY_INIT;
  glm_mat3_inv(inv_inertia, inertia_tensor);
  glm_mat3_mul(bone_to_world, inertia_tensor, inertia_tensor);
  glm_mat3_mul(inertia_tensor, world_to_bone, inertia_tensor);

  // I_hat
  mat6_compose(MAT3_ZERO, mass_mat, inertia_tensor, MAT3_ZERO, i_dest);

  // Convert gravity to bone space
  vec3 gravity = GLM_VEC3_ZERO_INIT;
  glm_mat3_mulv(world_to_bone, grav, gravity);

  vec3 za_linear = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(gravity, -mass, za_linear);

  vec3 za_ang = GLM_VEC3_ZERO_INIT;
  glm_mat3_mulv(inertia_tensor, ang_vel, za_ang);

  glm_vec3_cross(ang_vel, za_ang, za_ang);

  // Z_hat
  vec6_compose(za_linear, za_ang, z_dest);

  // Convert external force to bone space
  vec6 e_force = GLM_VEC3_ZERO_INIT;
  mat6 mat6_to_bone = MAT6_ZERO_INIT;
  mat6_compose(world_to_bone, GLM_MAT3_ZERO, GLM_MAT3_ZERO, world_to_bone,
               mat6_to_bone);
  mat6_mulv(mat6_to_bone, e_forces, e_force);

  // Apply external force
  vec6_sub(z_dest, e_force, z_dest);
}

void calc_articulated_child(mat6 to_parent, mat6 from_parent, mat6 I_hat_A,
                            vec6 Z_hat_A, vec6 c_hat, vec6 s_hat, float Q,
                            mat6 i_dest, vec6 z_dest, vec6 si_dest,
                            float *sis_dest, float *szi_dest) {
  vec6 temp_vec6 = VEC6_ZERO_INIT;
  mat6 temp_mat6 = MAT6_ZERO_INIT;

  // ========== Articulated I data ===========
  vec6 I_hat_s_hat = VEC6_ZERO_INIT;
  // Is
  mat6_mulv(I_hat_A, s_hat, I_hat_s_hat);

  // Iss'
  vec6_spatial_transpose_mulv(I_hat_s_hat, s_hat, temp_mat6);

  // Iss'I
  mat6_mul(temp_mat6, I_hat_A, temp_mat6);

  // s'I
  vec6_spatial_transpose_mulm(s_hat, I_hat_A, si_dest);

  // s'Is
  *sis_dest = vec6_dot(si_dest, s_hat);
  mat6_scale(temp_mat6, 1.0 / *sis_dest, temp_mat6);

  mat6_sub(I_hat_A, temp_mat6, i_dest);

  mat6_mul(to_parent, i_dest, i_dest);
  mat6_mul(i_dest, from_parent, i_dest);

  // ============ Articulated Z data ============
  vec6 I_hat_coriolis = VEC6_ZERO_INIT;
  mat6_mulv(I_hat_A, c_hat, I_hat_coriolis);

  vec6_add(Z_hat_A, I_hat_coriolis, temp_vec6);

  // s'(Z_hat_A + I_hat_coriolis))
  *szi_dest = vec6_inner_product(s_hat, temp_vec6);

  float scalar = -(*szi_dest) / (*sis_dest);

  vec6_scale(I_hat_s_hat, scalar, temp_vec6);
  vec6_add(temp_vec6, I_hat_coriolis, temp_vec6);
  vec6_add(temp_vec6, Z_hat_A, z_dest);

  mat6_mulv(to_parent, z_dest, z_dest);
}

void calc_a_hat(mat6 from_parent, vec6 p_a_hat, vec6 c_hat, vec6 s_hat,
                vec6 si, float szi, float sis, vec6 a_hat_dest,
                float *accel_angle_dest) {
  vec6_zero(a_hat_dest);

  vec6 temp_vec6 = VEC6_ZERO_INIT;
  mat6_mulv(from_parent, p_a_hat, temp_vec6);
  vec6_add(temp_vec6, c_hat, a_hat_dest);

  // q**
  float temp = vec6_dot(si, temp_vec6);
  *accel_angle_dest = (-temp - szi) / sis;
  vec6_scale(s_hat, *accel_angle_dest, temp_vec6);

  // a_hat
  vec6_add(a_hat_dest, temp_vec6, a_hat_dest);
  vec6_remove_noise(a_hat_dest, 0.001);
}

void calc_zj_ent_to_world(mat4 p_ent_to_world, vec3 p_head_ent, vec3 dof,
                          float joint_angle, int joint_type, mat4 dest) {
  glm_mat4_identity(dest);
  vec3 temp = GLM_VEC3_ZERO_INIT;
  mat4 from_p_head = GLM_MAT4_IDENTITY_INIT;
  mat4 to_p_head = GLM_MAT4_IDENTITY_INIT;
  mat4 joint_mat = GLM_MAT4_IDENTITY_INIT;

  glm_translate(to_p_head, p_head_ent);
  glm_vec3_negate_to(p_head_ent, temp);
  glm_translate(from_p_head, temp);

  if (joint_type == JOINT_REVOLUTE) {
    glm_rotate(joint_mat, joint_angle, dof);
  } else {
    glm_vec3_scale_as(dof, joint_angle, temp);
    glm_translate(joint_mat, temp);
  }

  glm_mat4_mul(joint_mat, from_p_head, dest);
  glm_mat4_mul(to_p_head, dest, dest);
  glm_mat4_mul(p_ent_to_world, dest, dest);
}
