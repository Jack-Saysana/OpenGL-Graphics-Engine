#include <physics/constraint.h>

void apply_constraints(ENTITY *entity, J_CONS *constraints,
                       size_t num_constr, vec3 gravity) {
  amat A = init_amat(NULL, num_constr * 3, num_constr * 3);
  amat B = init_amat(NULL, num_constr * 3, 1);
  amat C = init_amat(NULL, num_constr * 3, 1);
  amat def_accels = init_amat(NULL, 3, num_constr);
  vec3 temp = GLM_VEC3_ZERO_INIT;
  vec3 def_accel = GLM_VEC3_ZERO_INIT;
  size_t col_idx = 0;

  // Get default acceleration and desired net effect for each point of interest
  for (size_t i = 0; i < entity->model->num_colliders; i++) {
    glm_vec3_zero(entity->np_data[i].e_force);
  }
  featherstone_abm(entity, gravity);
  for (size_t i = 0; i < num_constr; i++) {
    calc_world_accel(entity, constraints[i].pt, constraints[i].col_idx,
                     def_accel);
    glm_vec3_copy(def_accel, def_accels.data + (i*3));
    glm_vec3_sub(constraints[i].d_accel, def_accel, temp);
    // Store desired net effect in linear system
    AMAT_GET(B, 0, (i*3)) = temp[0];
    AMAT_GET(B, 0, (i*3)+1) = temp[1];
    AMAT_GET(B, 0, (i*3)+2) = temp[2];
  }

  // Apply test forces for each constraint
  for (size_t i = 0; i < num_constr; i++) {
    for (size_t j = 0; j < 3; j++) {
      entity->np_data[constraints[i].col_idx].e_force[j] = 1.0;
      featherstone_abm(entity, gravity);
      entity->np_data[constraints[i].col_idx].e_force[j] = 0.0;
      for (size_t k = 0; k < num_constr; k++) {
        glm_vec3_copy(def_accels.data + (k*3), def_accel);
        calc_world_accel(entity, constraints[k].pt, constraints[k].col_idx,
                         temp);
        glm_vec3_sub(temp, def_accel, temp);
        AMAT_GET(A, (i*3)+j, (k*3)) = temp[0];
        AMAT_GET(A, (i*3)+j, (k*3)+1) = temp[1];
        AMAT_GET(A, (i*3)+j, (k*3)+2) = temp[2];
      }
    }
  }

  // Solve linear system
  solve_system(A, B, C);

  // Apply constraint forces
  for (size_t i = 0; i < num_constr; i++) {
    col_idx = constraints[i].col_idx;
    entity->np_data[col_idx].e_force[0] = AMAT_GET(C, 0, (i*3));
    entity->np_data[col_idx].e_force[1] = AMAT_GET(C, 0, (i*3)+1);
    entity->np_data[col_idx].e_force[2] = AMAT_GET(C, 0, (i*3)+2);
  }

  free_amat(A);
  free_amat(B);
  free_amat(C);
}

void calc_world_accel(ENTITY *entity, vec3 pt, size_t col, vec3 dest) {
  size_t bone = 0;
  mat3 bone_to_entity = GLM_MAT3_IDENTITY_INIT;
  mat3 entity_to_world = GLM_MAT3_IDENTITY_INIT;
  mat3 bone_to_world = GLM_MAT3_IDENTITY_INIT;
  mat3 world_to_bone = GLM_MAT3_IDENTITY_INIT;

  bone = entity->model->collider_bone_links[col];
  glm_mat3_copy(entity->model->bones[bone].coordinate_matrix,
                bone_to_entity);
  glm_mat4_pick3(entity->final_b_mats[bone], entity_to_world);
  glm_mat3_mul(entity_to_world, bone_to_entity, bone_to_world);
  glm_mat3_transpose_to(bone_to_world, world_to_bone);
  if (entity->model->colliders[col].type == POLY) {
    glm_mat4_mulv3(entity->final_b_mats[bone],
                   entity->model->colliders[col].data.center_of_mass, 1.0,
                   dest);
  } else {
    glm_mat4_mulv3(entity->final_b_mats[bone],
                   entity->model->colliders[col].data.center, 1.0, dest);
  }
  glm_vec3_sub(pt, dest, dest);

  // Convert constriants[i].pt to bone space
  glm_mat3_mulv(world_to_bone, dest, dest);
  // Calculate a = a_lin + (a_ang x r) for point
  glm_vec3_cross(entity->np_data[col].a_hat, dest, dest);
  glm_vec3_add(((float *)entity->np_data[col].a_hat)+3, dest, dest);
  // Convert a to world space
  glm_mat3_mulv(bone_to_world, dest, dest);
}
