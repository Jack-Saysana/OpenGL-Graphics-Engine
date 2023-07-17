#include <featherstone.h>
/*
int featherstone_abm(ENTITY *body) {
  size_t num_links = body->model->num_colliders;
  COLLIDER *links = body->model->colliders;

  vec3 temp = GLM_VEC3_ZERO_INIT;

  BONE *bones = body->model->bones;
  COLLIDER *colliders = body->model->colliders;
  int *bone_from_col = body->model->collider_bone_links;
  int *col_from_bone = body->model->bone_collider_links;
  P_DATA *p_data = body->np_data;

  int root_bone = -1;
  int parent_bone = -1;
  int parent_col = -1;

  // TODO joint_to_com can be precomputed
  // Calculate spatial velocities from inbound to outbound
  for (int cur_col = 0; cur_col < num_links; cur_col++) {
    if (links[cur_col].category != HIT_BOX) {
      continue;
    }

    root_bone = bone_from_col[i];
    vec3 cur_coords = GLM_VEC3_ZERO_INIT;
    if (colliders[cur_col].type == POLY) {
      glm_vec3_copy(colliders[cur_col].data.center_of_mass, cur_coords);
    } else {
      glm_vec3_copy(colliders[cur_col].data.center, cur_coords);
    }
    glm_vec3_sub(cur_coords, bones[root_bone].coords,
                 p_data[cur_col].joint_to_com);

    parent_bone = bones[root_bone].parent;
    parent_col = -1;
    if (parent_bone != -1) {
      parent_col = col_from_bone[parent_bone];
      vec3 parent_coords = GLM_VEC3_ZERO_INIT;
      if (colliders[parent_col].type == POLY) {
        glm_vec3_copy(colliders[parent_col].data.center_of_mass,
                      parent_coords);
      } else {
        glm_vec3_copy(colliders[parent_col].data.center, parent_coords);
      }

      compute_spatial_transformations(body->bone_mats[parent_bone],
                                      parent_coords,
                                      body->bone_mats[root_bone],
                                      cur_coords,
                                      p_data[cur_col].from_parent_lin,
                                      p_data[cur_col].from_parent,
                                      p_data[cur_col].to_parent);
    }
    compute_spatial_velocity(cur_col, parent_col, colliders, p_data);
  }

  // TODO A lot of this can be precomputed and never changes (I-hat, za_linear)
  // Calculate I-hat and Z-hat from inbound to outbound
  for (int i = 0; i < num_links; i++) {
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

void compute_spatial_transformations(mat4* parent_mats,
                                     vec3 parent_coords,
                                     mat4* child_mats,
                                     vec3 child_coords,
                                     vec3 parent_to_child_lin_dest,
                                     mat6 parent_to_child_dest,
                                     mat6 child_to_parent_dest) {
  // Matrix rotating vectors in the parent bone's coordinates to world coords
  mat4 parent_to_world = parent_mats[ROTATION];
  // Inverse of above
  mat4 world_to_parent = GLM_MAT4_IDENTITY_INIT;
  glm_vec4_inv(parent_to_world, world_to_parent);

  // Matrix rotating vectors in the child bone's coordinates to world
  // coordinates
  mat4 child_to_world = child_mats[ROTATION];

  // Matrix rotating vectors in child's bone coordinates to vectors in the
  // current bone's coordinates
  mat3 child_to_parent_rot = GLM_MAT3_IDENTITY_INIT;
  glm_mat4_mul(world_to_parent, child_to_world, child_to_world);
  glm_mat4_pick3(child_to_world, child_to_parent_rot);

  // Matrix rotating vectors in the current bone's coordinates to vectors
  // in the child's bone coordinates
  mat3 parent_to_child_rot = GLM_MAT3_IDENTITY_INIT;
  glm_mat3_inv(child_to_parent_rot, parent_to_child_rot);

  // Matrix who's diagonal is the vector from the parent bone tail to the
  // child bone tail
  mat3 parent_to_child_mat = GLM_MAT3_IDENTITY_INIT;
  glm_vec3_sub(child_coords, parent_coords, parent_to_child_lin_dest);
  vec3_singular_cross(parent_to_child_lin_dest, parent_to_child_mat);

  // Matrix who's diagonal is the vector from the child bone tail to the
  // parent bone tail
  mat3 child_to_parent_mat = GLM_MAT3_IDENTITY_INIT;
  vec3 child_to_parent_lin = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(parent_to_child_lin, -1.0, child_to_parent_lin);
  vec3_singular_cross(child_to_parent_lin, child_to_parent_mat);

  // Spatial transformation matrix transforming vectors in the child bone's
  // coords to vectors in the current bone's coords
  mat6_spatial_transform(child_to_parent_rot, parent_to_child_mat,
                         child_to_parent_dest);

  // Spatial transformation matrix transforming vectors in the current
  // bone's coords to vectors in the child bone's coords
  mat6_spatial_transform(parent_to_child, rot, child_to_parent_mat,
                         parent_to_child_dest);
}

void compute_spatial_velocity(int cur_col, int parent_col,
                              COLLIDERS *colliders, P_DATA *p_data) {
  // NOTATION NOTES
  // "v" denotes velocity
  // "va" denotes angular velocity
  // "v_hat" denotes spatial (six dimentional) velocity
  vec3 temp = GLM_VEC3_ZERO_INIT;

  vec3 v = GLM_VEC3_ZERO_INIT;
  vec3 va = GLM_VEC3_ZERO_INIT;
  if (parent_col != -1) {
    glm_mat3_mulv(p_data[cur_col].ST_from_parent, p_data[parent_col].v_hat, v);

    glm_mat3_mulv(p_data[cur_col].ST_from_parent,
                  ((float *) p_data[parent_col].v_hat) + 3, va);
    glm_vec3_cross(p_data[cur_col].v_hat, p_data[cur_col].from_parent_lin,
                   temp);
    glm_vec3_add(va, temp, va);
  }

  // Accumulate velocities from prismatic DOFS
  for (int j = 0; j < 3; j++) {
    if (p_data->dofs[j]) {
      glm_vec3_scale(BASIS_VECTORS[j], p_data[cur_col]->joint_angle_vels[j],
                     temp);
      glm_vec3_add(temp, v, v);
    }
  }
  // TODO Make this more flexible so more than just having the X-Axis as the
  // only degree of freedom
  glm_vec3_scale(BASIS_VECTORS[0],
                 p_data[cur_col]->joint_angle_vels[3], temp);
  glm_vec3_add(temp, va, va);

  glm_vec3_cross(BASIS_VECTORS[0], p_data[cur_col]->joint_to_com, temp);
  glm_vec3_scale(temp, p_data[cur_col]->joint_angle_vels[3], temp);
  glm_vec3_add(temp, v, v);

  vec6_compose(va, v, p_data[cur_col]->v_hat);
}

void compute_I_hat_and_Z_hat(int cur_col, int cur_bone, P_DATA *p_data,
                             mat4 **bone_mats) {
  mat4 temp_mat4 = GLM_MAT4_ZERO_INIT;
  vec3 temp_vec3 = GLM_VEC3_ZERO_INIT;

  // M
  float mass = 1.0 / p_data[cur_col]->inv_mass;
  mat3 mass_mat = GLM_MAT3_IDENTITY_INIT;
  glm_mat3_scale(mass_mat, mass);

  // I
  mat3 inertia_tensor = GLM_MAT3_IDENTITY_INIT;
  glm_mat4_inv(p_data[cur_col]->inv_inertia, temp_mat4);
  glm_mat4_pick3(temp_mat4, inertia_tensor);

  // I-hat = [[0, M], [I, 0]];
  mat6_compose(MAT3_ZERO, mass_mat, inertia_tensor, MAT3_ZERO,
               p_data->I_hat);

  // L
  vec3 za_linear = GLM_VEC3_ZERO_INIT;
  mat4 world_to_cur = GLM_MAT3_ZERO_INIT;
  glm_mat4_inv(bone_mats[cur_bone][ROTATION], world_to_cur);
  glm_mat4_mulv3(world_to_cur, GRAVITY, 1.0, za_linear);
  glm_vec3_scale(za_linear, -mass, za_linear);

  // A
  vec3 za_angular = GLM_VEC3_ZERO_INIT;
  glm_mat3_mulv(inertia, p_data[cur_col]->v_hat, za_angular);
  glm_vec3_cross(ang_vel, za_angular, za_angular);

  // Z-hat = [L, A]
  vec6_compose(za_linear, za_angular, p_data[cur_col]->Z_hat);
}*/
