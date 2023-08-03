#include <featherstone.h>

int featherstone_abm(ENTITY *body) {
  size_t num_links = body->model->num_colliders;
  COLLIDER *links = body->model->colliders;

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
    if (links[cur_col].category != HURT_BOX) {
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

    printf("joint_to_com[%d] (ent): %f %f %f\n", cur_col,
           p_data[cur_col].joint_to_com[0],
           p_data[cur_col].joint_to_com[1],
           p_data[cur_col].joint_to_com[2]);
    glm_mat4_mulv3(cur_ent_to_bone, p_data[cur_col].joint_to_com, 1.0,
                   p_data[cur_col].joint_to_com);
    printf("joint_to_com[%d] (bone): %f %f %f\n", cur_col,
           p_data[cur_col].joint_to_com[0],
           p_data[cur_col].joint_to_com[1],
           p_data[cur_col].joint_to_com[2]);

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

      printf("\n\np: %d\nc: %d\n", parent_col, cur_col);
      printf("\np_bone_to_world:\n\n");
      print_mat4(parent_bone_to_world);
      printf("\np_ent_to_bone:\n");
      print_mat3(bones[parent_bone].coordinate_matrix);
      printf("\nc_bone_to_world:\n\n");
      print_mat4(cur_bone_to_world);
      printf("\nc_ent_to_bone:\n\n");
      print_mat3(bones[root_bone].coordinate_matrix);
      printf("\n");
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
    if (links[cur_col].category != HURT_BOX) {
      continue;
    }

    root_bone = bone_from_col[cur_col];

    mat4 temp = GLM_MAT4_IDENTITY_INIT;

    // M
    float mass = 1.0 / p_data[cur_col].inv_mass;
    mat3 mass_mat = GLM_MAT3_IDENTITY_INIT;
    glm_mat3_scale(mass_mat, mass);

    // I
    mat3 inertia_tensor = GLM_MAT3_IDENTITY_INIT;
    glm_mat4_inv(p_data[cur_col].inv_inertia, temp);
    glm_mat4_pick3(temp, inertia_tensor);

    // I_hat
    mat6_compose(MAT3_ZERO, mass_mat, inertia_tensor, MAT3_ZERO,
                 p_data[cur_col].I_hat);

    // Convert gravity to bone space
    mat3 world_to_ent = GLM_MAT3_IDENTITY_INIT;
    glm_mat4_inv(body->final_b_mats[root_bone], temp);
    glm_mat4_pick3(temp, world_to_ent);
    mat3 ent_to_bone = GLM_MAT3_IDENTITY_INIT;
    glm_mat3_inv(bones[root_bone].coordinate_matrix, ent_to_bone);
    mat3 world_to_bone = GLM_MAT3_IDENTITY_INIT;
    glm_mat3_mul(ent_to_bone, world_to_ent, world_to_bone);
    vec3 gravity = { 0.0, -1.0, 0.0 };
    glm_mat3_mulv(world_to_bone, gravity, gravity);

    vec3 za_linear = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(gravity, -mass, za_linear);

    float *ang_vel = (float *) p_data[cur_col].v_hat;
    vec3 za_ang = GLM_VEC3_ZERO_INIT;
    glm_mat3_mulv(inertia_tensor, ang_vel, za_ang);
    glm_vec3_cross(ang_vel, za_ang, za_ang);

    // Z_hat
    vec6_compose(za_linear, za_ang, p_data[cur_col].Z_hat);
  }

  /*
  // Calculate I-hat-A and Z-hat-A from outbound to inbound
  for (int i = num_links - 1; i >= 0; i--) {
    if (links[i].category != HURT_BOX) {
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
  // Parent to child in world coords
  glm_vec3_sub(c_coords, p_coords, p_to_c_lin_dest);
  printf("c_coords: (%f, %f, %f)\n", c_coords[0], c_coords[1], c_coords[2]);
  printf("p_coords: (%f, %f, %f)\n", p_coords[0], p_coords[1], p_coords[2]);
  // Rotate to bone coords of parent
  glm_mat3_mulv(p_world_to_bone, p_to_c_lin_dest, p_to_c_lin_dest);
  printf("p->c: (%f, %f, %f)\n", p_to_c_lin_dest[0], p_to_c_lin_dest[1],
         p_to_c_lin_dest[2]);
  vec3_singular_cross(p_to_c_lin_dest, p_to_c_mat);

  // Linear component of the spatial transformation matrix transforming vectors
  // in the parent bone space to the child bone space
  mat3 c_to_p_mat = GLM_MAT3_IDENTITY_INIT;
  vec3 c_to_p_lin = GLM_VEC3_ZERO_INIT;
  // Child to parent in world coords
  glm_vec3_sub(p_coords, c_coords, c_to_p_lin);
  // Rotate to bone coords of child
  glm_mat3_mulv(c_world_to_bone, c_to_p_lin, c_to_p_lin);
  printf("c->p: (%f, %f, %f)\n", c_to_p_lin[0], c_to_p_lin[1], c_to_p_lin[2]);
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
    printf("\n%d -> %d\n", parent_col, cur_col);
    print_mat6(p_data[cur_col].ST_from_parent);
    printf("\n%d <- %d\n", parent_col, cur_col);
    print_mat6(p_data[cur_col].ST_to_parent);
    printf("\n");
    printf("v_hat:\n");
    print_vec6(p_data[parent_col].v_hat);

    mat6_mulv(p_data[cur_col].ST_from_parent, p_data[parent_col].v_hat,
              parent_v_hat);
    // Because the angular component of a link is represented by the first half
    // of v_hat, we can just pass v_hat in as a vec3 to use the angular
    // velocity
    glm_vec3_copy(parent_v_hat, va);
    // The linear component is represented by the second half of v_hat, so we
    // extract the last half of v_hat when using linear velocity
    glm_vec3_copy(((float *) parent_v_hat) + 3, v);
    glm_vec3_cross(p_data[cur_col].v_hat, p_data[cur_col].from_parent_lin,
                   temp);
    glm_vec3_add(va, temp, va);
  }

  // Accumulate velocities from prismatic DOFS
  /*for (int j = 0; j < 3; j++) {
    if (p_data->dofs[j]) {
      glm_vec3_scale(BASIS_VECTORS[j], p_data[cur_col].joint_angle_vels[j],
                     temp);
      glm_vec3_add(temp, v, v);
    }
  }*/
  // TODO Make this more flexible so more than just having the X-Axis as the
  // only degree of freedom
  glm_vec3_scale(BASIS_VECTORS[0], p_data[cur_col].joint_angle_vels[3], temp);
  glm_vec3_add(temp, va, va);

  glm_vec3_cross(BASIS_VECTORS[0], p_data[cur_col].joint_to_com, temp);
  glm_vec3_scale(temp, p_data[cur_col].joint_angle_vels[3], temp);
  glm_vec3_add(temp, v, v);

  vec6_compose(va, v, p_data[cur_col].v_hat);
}

void compute_I_hat_and_Z_hat(int cur_col, int cur_bone, P_DATA *p_data,
                             mat4 **bone_mats) {
  mat4 temp = GLM_MAT4_ZERO_INIT;

  // M
  float mass = 1.0 / p_data[cur_col].inv_mass;
  mat3 mass_mat = GLM_MAT3_IDENTITY_INIT;
  glm_mat3_scale(mass_mat, mass);

  // I
  mat3 inertia_tensor = GLM_MAT3_IDENTITY_INIT;
  glm_mat4_inv(p_data[cur_col].inv_inertia, temp);
  glm_mat4_pick3(temp, inertia_tensor);

  // I-hat = [[0, M], [I, 0]];
  mat6_compose(MAT3_ZERO, mass_mat, inertia_tensor, MAT3_ZERO,
               p_data->I_hat);

  // L
  vec3 za_linear = GLM_VEC3_ZERO_INIT;
  mat4 world_to_cur = GLM_MAT3_ZERO_INIT;
  glm_mat4_inv(bone_mats[cur_bone][ROTATION], world_to_cur);
  glm_mat4_mulv3(world_to_cur, G_VEC, 1.0, za_linear);
  glm_vec3_scale(za_linear, -mass, za_linear);

  // A
  vec3 za_angular = GLM_VEC3_ZERO_INIT;
  // Because the angular velocity of a link is represented by the first half
  // of v_hat, we can just pass v_hat in as a vec3 to use the angular velocity
  glm_mat3_mulv(inertia_tensor, p_data[cur_col].v_hat, za_angular);
  glm_vec3_cross(p_data[cur_col].v_hat, za_angular, za_angular);

  // Z-hat = [L, A]
  vec6_compose(za_linear, za_angular, p_data[cur_col].Z_hat);
}
