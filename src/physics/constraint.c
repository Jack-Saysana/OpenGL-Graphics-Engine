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
    vec6_zero(entity->np_data[i].e_force);
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
      //entity->np_data[constraints[i].col_idx].e_force[j] = 1.0;
      vec3 e_force = GLM_VEC3_ZERO_INIT;
      e_force[j] = 1.0;
      calc_force_vec(entity, constraints[i].pt, constraints[i].col_idx,
                     e_force, entity->np_data[constraints[i].col_idx].e_force);

      featherstone_abm(entity, gravity);
      vec6_zero(entity->np_data[constraints[i].col_idx].e_force);
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
  /*
  fprintf(stderr, "A:\n");
  print_amat2(A);
  fprintf(stderr, "\n");
  fprintf(stderr, "B:\n");
  print_amat2(B);
  fprintf(stderr, "C:\n");
  print_amat2(C);
  */
  // Check if the calculated soltuion is accurate given some error threshold
  if (!check_sol(A, B, C, CONSTRAINT_ERROR_THRESHOLD)) {
    // Solution is corrupted due to numerical instability. Better to drop the
    // constraints
    //fprintf(stderr, "DROPPING CONSTRAINTS\n");
    vec6_zero(entity->np_data[col_idx].e_force);
  } else {
    // Apply constraint forces
    for (size_t i = 0; i < num_constr; i++) {
      col_idx = constraints[i].col_idx;
      vec3 e_force = GLM_VEC3_ZERO_INIT;
      e_force[0] = AMAT_GET(C, 0, (i*3));
      e_force[1] = AMAT_GET(C, 0, (i*3)+1);
      e_force[2] = AMAT_GET(C, 0, (i*3)+2);
      calc_force_vec(entity, constraints[i].pt, constraints[i].col_idx,
                     e_force, entity->np_data[col_idx].e_force);
    }
  }

  free_amat(A);
  free_amat(B);
  free_amat(C);
}

void calc_world_accel(ENTITY *entity, vec3 pt, size_t col, vec3 dest) {
  int bone = entity->model->collider_bone_links[col];
  glm_mat4_mulv3(entity->final_b_mats[bone],
                 entity->model->bones[bone].base, 1.0, dest);


  glm_vec3_sub(pt, dest, dest);
  glm_vec3_cross(entity->np_data[col].ang_a, dest, dest);
  glm_vec3_add(entity->np_data[col].a, dest, dest);
}

void calc_force_vec(ENTITY *entity, vec3 pt, size_t col, vec3 force,
                    vec6 dest) {
  vec6 temp = VEC6_ZERO_INIT;
  int bone = entity->model->collider_bone_links[col];
  vec3 joint_base = GLM_VEC3_ZERO_INIT;
  glm_mat4_mulv3(entity->final_b_mats[bone], entity->model->bones[bone].base,
                 1.0, joint_base);
  vec3 p = GLM_VEC3_ZERO_INIT;
  glm_vec3_sub(pt, joint_base, p);

  glm_vec3_copy(force, temp);
  glm_vec3_cross(p, force, ((float *)temp)+3);

  vec6_add(temp, dest, dest);
}
