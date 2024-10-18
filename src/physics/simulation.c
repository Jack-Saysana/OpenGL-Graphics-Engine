#include <physics/simulation.h>

// ========================== CREATION AND DELETION ==========================

SIMULATION *init_sim(float max_extent, unsigned int max_depth) {
  SIMULATION *sim = malloc(sizeof(SIMULATION));
  if (sim == NULL) {
    fprintf(stderr, "Error: Unable to allocate simulation\n");
    return NULL;
  }

  sim->oct_tree = init_tree(max_extent, max_depth);
  if (sim->oct_tree == NULL) {
    free(sim);
    fprintf(stderr, "Error: Unable to initailize simulation oct-tree\n");
    return NULL;
  }

  int status = ledger_init(&sim->ment_ledger, &sim->ment_list,
                           &sim->num_ent_moving, &sim->ment_ledger_size,
                           &sim->ment_list_size);
  if (status) {
    free_oct_tree(sim->oct_tree);
    free(sim);
    fprintf(stderr, "Error: Unable to allocate simulation ledgers\n");
    return NULL;
  }

  status = ledger_init(&sim->mcol_ledger, &sim->mcol_list,
                       &sim->num_col_moving, &sim->mcol_ledger_size,
                       &sim->mcol_list_size);
  if (status) {
    free(sim->ment_ledger);
    free(sim->ment_list);
    free_oct_tree(sim->oct_tree);
    free(sim);
    fprintf(stderr, "Error: Unable to allocate simulation ledgers\n");
    return NULL;
  }

  status = ledger_init(&sim->dcol_ledger, &sim->dcol_list,
                       &sim->num_col_driving, &sim->dcol_ledger_size,
                       &sim->dcol_list_size);
  if (status) {
    free(sim->ment_ledger);
    free(sim->ment_list);
    free(sim->mcol_ledger);
    free(sim->mcol_list);
    free_oct_tree(sim->oct_tree);
    free(sim);
    fprintf(stderr, "Error: Unable to allocate simulation ledgers\n");
    return NULL;
  }

  glm_vec3_zero(sim->forces);

  return sim;
}

void free_sim(SIMULATION *sim) {
  if (sim == NULL) {
    return;
  }

  free_oct_tree(sim->oct_tree);
  free(sim->ment_ledger);
  free(sim->ment_list);
  free(sim->mcol_ledger);
  free(sim->mcol_list);
  free(sim->dcol_ledger);
  free(sim->dcol_list);
  free(sim);
}

// ============================ SIMULATION HELPERS ===========================

int sim_add_entity(SIMULATION *sim, ENTITY *entity, int collider_filter) {
  if (entity == NULL || sim == NULL) {
    return -1;
  }

  int status = 0;
  LEDGER_INPUT input;
  input.ent = entity;

  // Add desired colliders to the simulation
  COLLIDER *cur_col = NULL;
  input.collider.ent = entity;
  for (size_t i = 0; i < entity->model->num_colliders; i++) {
    input.collider.col = i;
    cur_col = entity->model->colliders + i;
    if (((collider_filter & ALLOW_DEFAULT) &&
         cur_col->category == DEFAULT) ||
        ((collider_filter & ALLOW_HURT_BOXES) &&
         cur_col->category == HURT_BOX) ||
        ((collider_filter & ALLOW_HIT_BOXES) &&
         cur_col->category == HIT_BOX)) {
      // Check if collider is driving
      if (entity->type & T_DRIVING) {
        status = ledger_add(&sim->dcol_ledger, &sim->dcol_list,
                            &sim->num_col_driving, &sim->dcol_ledger_size,
                            &sim->dcol_list_size, input, L_TYPE_COLLIDER);
        if (status) {
          fprintf(stderr, "Error: Unable to reallocate simulation ledger\n");
          return -1;
        }
      }

      // Check if collider is currently moving
      if (is_moving(entity, i)) {
        status = ledger_add(&sim->mcol_ledger, &sim->mcol_list,
                            &sim->num_col_moving, &sim->mcol_ledger_size,
                            &sim->mcol_list_size, input, L_TYPE_COLLIDER);
        if (status) {
          fprintf(stderr, "Error: Unable to reallocate simulation ledger\n");
          return -1;
        }

        status = ledger_add(&sim->ment_ledger, &sim->ment_list,
                            &sim->num_ent_moving, &sim->ment_ledger_size,
                            &sim->ment_list_size, input, L_TYPE_ENTITY);
        if (status) {
          fprintf(stderr, "Error: Unable to reallocate simulation ledger\n");
          return -1;
        }
      }

      // Insert collider into oct-tree
#ifdef DEBUG_OCT_TREE
      status = oct_tree_insert(sim->oct_tree, entity, i, 1);
#else
      status = oct_tree_insert(sim->oct_tree, entity, i);
#endif
      if (status) {
        fprintf(stderr,
                "Error: Unable to insert entity into simulation oct-tree\n");
        return -1;
      }
    }
  }

  return 0;
}

int sim_remove_entity(SIMULATION *sim, ENTITY *entity) {
  if (sim == NULL || entity == NULL) {
    return -1;
  }

  LEDGER_INPUT input;
  input.ent = entity;
  ledger_delete(sim->ment_ledger, sim->ment_list, sim->ment_ledger_size,
                &sim->num_ent_moving, input, L_TYPE_ENTITY);

  // TODO perhaps filter colliders better
  input.collider.ent = entity;
  for (size_t i = 0; i < entity->model->num_colliders; i++) {
    input.collider.col = i;
    ledger_delete(sim->mcol_ledger, sim->mcol_list, sim->mcol_ledger_size,
                  &sim->num_col_moving, input, L_TYPE_COLLIDER);
    ledger_delete(sim->dcol_ledger, sim->dcol_list, sim->dcol_ledger_size,
                  &sim->num_col_driving, input, L_TYPE_COLLIDER);
    oct_tree_delete(sim->oct_tree, entity, i);
  }

  return 0;
}

void sim_add_force(SIMULATION *sim, vec3 force) {
  glm_vec3_add(sim->forces, force, sim->forces);
}

void sim_clear_forces(SIMULATION *sim) {
  glm_vec3_zero(sim->forces);
}

void prep_sim_movement(SIMULATION *sim) {
  for (size_t i = 0; i < sim->num_col_moving; i++) {
#ifdef DEBUG_OCT_TREE
    fprintf(stderr, "Prep before:\n");
    for (size_t j = 0; j < sim->oct_tree->data_buff_len; j++) {
      if (sim->oct_tree->data_buffer[j].entity == sim->mcol_ledger[sim->mcol_list[i]].col.entity &&
          sim->oct_tree->data_buffer[j].collider_offset == sim->mcol_ledger[sim->mcol_list[i]].col.collider_offset) {
        fprintf(stderr, "  %p, %ld, %ld, %d, %ld\n",
                sim->oct_tree->data_buffer[j].entity,
                sim->oct_tree->data_buffer[j].collider_offset,
                sim->oct_tree->data_buffer[j].node_offset,
                sim->oct_tree->data_buffer[j].birthmark,
                j);
      }
    }
#endif
    oct_tree_delete(sim->oct_tree,
                    sim->mcol_ledger[sim->mcol_list[i]].col.entity,
                    sim->mcol_ledger[sim->mcol_list[i]].col.collider_offset);
#ifdef DEBUG_OCT_TREE
    int bad = 0;
    fprintf(stderr, "Prep after:\n");
    for (size_t j = 0; j < sim->oct_tree->data_buff_len; j++) {
      if (sim->oct_tree->data_buffer[j].entity == sim->mcol_ledger[sim->mcol_list[i]].col.entity &&
          sim->oct_tree->data_buffer[j].collider_offset == sim->mcol_ledger[sim->mcol_list[i]].col.collider_offset) {
        ENTITY *ent = sim->oct_tree->data_buffer[j].entity;
        fprintf(stderr, "  %p, %ld, %ld, %d, %ld\n", ent,
                sim->oct_tree->data_buffer[j].collider_offset,
                sim->oct_tree->data_buffer[j].node_offset,
                sim->oct_tree->data_buffer[j].birthmark,
                j);
        bad = 1;
      }
    }
    if (bad) {
      COLLIDER obj;
      memset(&obj, 0, sizeof(COLLIDER));
      global_collider(sim->mcol_ledger[sim->mcol_list[i]].col.entity,
                      sim->mcol_ledger[sim->mcol_list[i]].col.collider_offset,
                      &obj);
      fprintf(stderr, "BAD\n");
    }
#endif
  }
}

#ifdef DEBUG_OCT_TREE
void update_sim_movement(SIMULATION *sim, int birthmark) {
  for (size_t i = 0; i < sim->num_col_moving; i++) {
    oct_tree_insert(sim->oct_tree,
                    sim->mcol_ledger[sim->mcol_list[i]].col.entity,
                    sim->mcol_ledger[sim->mcol_list[i]].col.collider_offset,
                    birthmark);
  }
}
#else
void update_sim_movement(SIMULATION *sim) {
  for (size_t i = 0; i < sim->num_col_moving; i++) {
    oct_tree_insert(sim->oct_tree,
                    sim->mcol_ledger[sim->mcol_list[i]].col.entity,
                    sim->mcol_ledger[sim->mcol_list[i]].col.collider_offset);
  }
}
#endif

size_t save_sim_state(SIMULATION *sim, SIM_STATE **state) {
  SIM_STATE *sim_state = malloc(sizeof(SIM_STATE) * sim->num_ent_moving);
  if (state == NULL) {
    return 0;
  }

  ENTITY *cur_ent = NULL;
  P_DATA *p_data = NULL;
  ZERO_JOINT *z_data = NULL;
  for (size_t i = 0; i < sim->num_ent_moving; i++) {
    cur_ent = sim->ment_ledger[sim->ment_list[i]].ent.entity;
    p_data = cur_ent->np_data;
    z_data = cur_ent->zj_data;
    sim_state[i].bone_mats = malloc(sizeof(mat4) * 3 *
                                    cur_ent->model->num_bones);
    if (sim_state[i].bone_mats == NULL) {
      free_sim_state(sim_state, i);
      return INVALID_INDEX;
    }
    size_t num_joints = cur_ent->model->num_colliders;
    for (size_t j = 0; j < cur_ent->model->num_colliders; j++) {
      num_joints += p_data[j].num_z_joints;
    }
    sim_state[i].col_state = malloc(sizeof(struct collider_state) *
                                    num_joints);
    if (sim_state[i].col_state == NULL) {
      free(sim_state[i].bone_mats);
      free_sim_state(sim_state, i);
      return INVALID_INDEX;
    }

    memcpy(sim_state[i].bone_mats, cur_ent->bone_mats, sizeof(mat4) * 3 *
           cur_ent->model->num_bones);
    size_t jnt = 0;
    for (size_t j = 0; j < cur_ent->model->num_colliders; j++) {
      for (size_t k = 0; k < p_data[j].num_z_joints; k++) {
        size_t z_offset = p_data[j].zero_joint_offset + k;
        sim_state[i].col_state[jnt].joint_angle = z_data[z_offset].joint_angle;
        sim_state[i].col_state[jnt].vel_angle = z_data[z_offset].vel_angle;
        jnt++;
      }
      sim_state[i].col_state[jnt].joint_angle = p_data[j].joint_angle;
      sim_state[i].col_state[jnt].vel_angle = p_data[j].vel_angle;
      jnt++;
    }
  }

  *state = sim_state;
  return sim->num_ent_moving;
}

void restore_sim_state(SIMULATION *sim, SIM_STATE *state) {
  ENTITY *cur_ent = NULL;
  P_DATA *p_data = NULL;
  ZERO_JOINT *z_data = NULL;
  for (size_t i = 0; i < sim->num_ent_moving; i++) {
    cur_ent = sim->ment_ledger[sim->ment_list[i]].ent.entity;
    p_data = cur_ent->np_data;
    z_data = cur_ent->zj_data;
    memcpy(cur_ent->bone_mats, state[i].bone_mats, sizeof(mat4) * 3 *
           cur_ent->model->num_bones);
    size_t jnt = 0;
    for (size_t j = 0; j < cur_ent->model->num_colliders; j++) {
      for (size_t k = 0; k < p_data[j].num_z_joints; k++) {
        size_t z_offset = p_data[j].zero_joint_offset + k;
        z_data[z_offset].joint_angle = state[i].col_state[jnt].joint_angle;
        z_data[z_offset].vel_angle = state[i].col_state[jnt].vel_angle;
        jnt++;
      }
      p_data[j].joint_angle = state[i].col_state[jnt].joint_angle;
      p_data[j].vel_angle = state[i].col_state[jnt].vel_angle;
      jnt++;
    }
  }

  free_sim_state(state, sim->num_ent_moving);
}

void free_sim_state(SIM_STATE *state, size_t num_ents) {
  for (size_t i = 0; i < num_ents; i++) {
    free(state[i].bone_mats);
    free(state[i].col_state);
  }
  free(state);
}

size_t peek_integration(SIMULATION *sim, SIM_STATE **state, vec3 origin,
                        float range) {
  size_t ret = save_sim_state(sim, state);

  ENTITY *cur_ent = NULL;
  for (size_t i = 0; i < sim->num_ent_moving; i++) {
    cur_ent = sim->ment_ledger[sim->ment_list[i]].ent.entity;
    if (range == SIM_RANGE_INF ||
        entity_in_range(sim, cur_ent, origin, range)) {
      if (cur_ent->num_cons) {
        apply_constraints(cur_ent, cur_ent->p_cons, cur_ent->num_cons,
                          sim->forces);
      }
      featherstone_abm(cur_ent, sim->forces);
      integrate_ent(cur_ent);
      for (size_t j = 0; j < cur_ent->model->num_colliders; j++) {
        glm_vec3_zero(cur_ent->np_data[j].e_force);
      }
    }
  }

  return ret;
}

void integrate_sim(SIMULATION *sim, vec3 origin, float range) {
  ENTITY *cur_ent = NULL;
  for (size_t i = 0; i < sim->num_ent_moving; i++) {
    cur_ent = sim->ment_ledger[sim->ment_list[i]].ent.entity;
    if (range == SIM_RANGE_INF ||
        entity_in_range(sim, cur_ent, origin, range)) {
      if (cur_ent->num_cons) {
        apply_constraints(cur_ent, cur_ent->p_cons, cur_ent->num_cons,
                          sim->forces);
      }
      featherstone_abm(cur_ent, sim->forces);
      integrate_ent(cur_ent);
      for (size_t j = 0; j < cur_ent->model->num_colliders; j++) {
        glm_vec3_zero(cur_ent->np_data[j].e_force);
      }
      cur_ent->num_cons = 0;
    }
  }
}

void integrate_ent(ENTITY *ent) {
  float delta = 0.0;
  for (int cur_bone = 0; cur_bone < ent->model->num_bones; cur_bone++) {
    int cur_col = ent->model->bone_collider_links[cur_bone];
    if (cur_col == -1) {
      continue;
    }

    int collider_root_bone = ent->model->collider_bone_links[cur_col];
    if (collider_root_bone == cur_bone) {
      // Integrate acceleration of zero joints
      for (size_t i = 0; i < ent->np_data[cur_col].num_z_joints; i++) {
        size_t cur_zj = ent->np_data[cur_col].zero_joint_offset + i;
        delta = ent->zj_data[cur_zj].accel_angle * DELTA_TIME;
        ent->zj_data[cur_zj].vel_angle *= 0.999;
        ent->zj_data[cur_zj].vel_angle += delta;
        remove_noise(ent->zj_data[cur_zj].vel_angle, ZERO_THRESHOLD);
      }
      // Integrate acceleration of main joint
      delta = ent->np_data[cur_col].accel_angle * DELTA_TIME;
      ent->np_data[cur_col].vel_angle *= 0.999;
      ent->np_data[cur_col].vel_angle += delta;
      remove_noise(ent->np_data[cur_col].vel_angle, ZERO_THRESHOLD);

      ///*
      //fprintf(stderr, "a: %f, %f, %f\n", ent->np_data[cur_col].a[0],
      //        ent->np_data[cur_col].a[1], ent->np_data[cur_col].a[2]);
      vec3 delta_vec3 = GLM_VEC3_ZERO_INIT;
      glm_vec3_scale(ent->np_data[cur_col].a, DELTA_TIME, delta_vec3);
      //fprintf(stderr, "d_v: %f, %f, %f\n", delta_vec3[0], delta_vec3[1],
      //        delta_vec3[2]);
      //fprintf(stderr, "v before: %f, %f, %f\n", ent->np_data[cur_col].v[0],
      //        ent->np_data[cur_col].v[1], ent->np_data[cur_col].v[2]);
      glm_vec3_scale(ent->np_data[cur_col].v, 0.999,
                     ent->np_data[cur_col].v);
      glm_vec3_add(ent->np_data[cur_col].v, delta_vec3,
                   ent->np_data[cur_col].v);
      vec3_remove_noise(ent->np_data[cur_col].v, ZERO_THRESHOLD);
      //fprintf(stderr, "v after: %f, %f, %f\n", ent->np_data[cur_col].v[0],
      //        ent->np_data[cur_col].v[1], ent->np_data[cur_col].v[2]);

      //fprintf(stderr, "ang_a: %f, %f, %f\n", ent->np_data[cur_col].ang_a[0],
      //        ent->np_data[cur_col].ang_a[1], ent->np_data[cur_col].ang_a[2]);
      glm_vec3_scale(ent->np_data[cur_col].ang_a, DELTA_TIME, delta_vec3);
      //fprintf(stderr, "d_v: %f, %f, %f\n", delta_vec3[0], delta_vec3[1],
      //        delta_vec3[2]);
      //fprintf(stderr, "ang_v before: %f, %f, %f\n",
      //        ent->np_data[cur_col].ang_v[0], ent->np_data[cur_col].ang_v[1],
      //        ent->np_data[cur_col].ang_v[2]);
      glm_vec3_scale(ent->np_data[cur_col].ang_v, 0.999,
                     ent->np_data[cur_col].ang_v);
      glm_vec3_add(ent->np_data[cur_col].ang_v, delta_vec3,
                   ent->np_data[cur_col].ang_v);
      vec3_remove_noise(ent->np_data[cur_col].ang_v, ZERO_THRESHOLD);
      //fprintf(stderr, "ang_v after: %f, %f, %f\n\n",
      //        ent->np_data[cur_col].ang_v[0], ent->np_data[cur_col].ang_v[1],
      //        ent->np_data[cur_col].ang_v[2]);
      //*/

      // Integrate velocity of zero joints
      for (size_t i = 0; i < ent->np_data[cur_col].num_z_joints; i++) {
        size_t cur_zj = ent->np_data[cur_col].zero_joint_offset + i;
        delta = ent->zj_data[cur_zj].vel_angle * DELTA_TIME;
        ent->zj_data[cur_zj].joint_angle += delta;
      }

      // Integrate velocity of main joint
      delta = ent->np_data[cur_col].vel_angle * DELTA_TIME;
      ent->np_data[cur_col].joint_angle += delta;
      glm_vec3_scale(ent->np_data[cur_col].v, DELTA_TIME, delta_vec3);
      glm_translate(ent->bone_mats[cur_bone][LOCATION], delta_vec3);

      vec3 delta_rot = GLM_VEC3_ZERO_INIT;
      glm_vec3_scale(ent->np_data[cur_col].ang_v, DELTA_TIME, delta_rot);
      versor rot_quat = GLM_QUAT_IDENTITY_INIT;
      glm_quatv(rot_quat, glm_vec3_norm(delta_rot), delta_rot);
      glm_quat_normalize(rot_quat);
      versor temp_quat = GLM_QUAT_IDENTITY_INIT;
      glm_mat4_quat(ent->bone_mats[cur_bone][ROTATION], temp_quat);
      glm_quat_normalize(temp_quat);
      glm_quat_mul(rot_quat, temp_quat, temp_quat);
      glm_quat_normalize(temp_quat);
      glm_quat_mat4(temp_quat, ent->bone_mats[cur_bone][ROTATION]);

      // Combine rotation, location and scale into final bone matrix
      vec3 temp = GLM_VEC3_ZERO_INIT;
      mat4 from_center = GLM_MAT4_IDENTITY_INIT;
      mat4 to_center = GLM_MAT4_IDENTITY_INIT;
      if (ent->model->colliders[cur_col].type == SPHERE) {
        glm_vec3_copy(ent->model->colliders[cur_col].data.center, temp);
      } else {
        glm_vec3_copy(ent->model->colliders[cur_col].data.center_of_mass,
                      temp);
      }
      glm_translate(to_center, temp);
      glm_vec3_negate(temp);
      glm_translate(from_center, temp);

      glm_mat4_identity(ent->final_b_mats[cur_bone]);
      glm_mat4_mul(from_center, ent->final_b_mats[cur_bone],
                   ent->final_b_mats[cur_bone]);
      glm_mat4_mul(ent->bone_mats[cur_bone][SCALE],
                   ent->final_b_mats[cur_bone],
                   ent->final_b_mats[cur_bone]);
      glm_mat4_mul(ent->bone_mats[cur_bone][ROTATION],
                   ent->final_b_mats[cur_bone],
                   ent->final_b_mats[cur_bone]);
      glm_mat4_mul(to_center, ent->final_b_mats[cur_bone],
                   ent->final_b_mats[cur_bone]);
      glm_mat4_mul(ent->bone_mats[cur_bone][LOCATION],
                   ent->final_b_mats[cur_bone],
                   ent->final_b_mats[cur_bone]);

      // "Anchor" revolute joints so they are not affected by translational
      // drift
      int parent_bone = ent->model->bones[cur_bone].parent;
      vec3 anchor = GLM_VEC3_ZERO_INIT;
      if (parent_bone != -1 &&
          ent->np_data[cur_col].joint_type != JOINT_PRISMATIC) {
        vec3 base_loc = GLM_VEC3_ZERO_INIT;
        glm_mat4_mulv3(ent->final_b_mats[cur_bone],
                       ent->model->bones[cur_bone].base, 1.0, base_loc);
        vec3 p_head_loc = GLM_VEC3_ZERO_INIT;
        glm_mat4_mulv3(ent->final_b_mats[parent_bone],
                       ent->model->bones[parent_bone].head, 1.0,
                       p_head_loc);
        glm_vec3_sub(p_head_loc, base_loc, anchor);
      }
      mat4 anchor_mat = GLM_MAT4_IDENTITY_INIT;
      glm_translate(anchor_mat, anchor);
      glm_mat4_mul(anchor_mat, ent->final_b_mats[cur_bone],
                   ent->final_b_mats[cur_bone]);
      glm_mat4_mul(anchor_mat, ent->bone_mats[cur_bone][LOCATION],
                   ent->bone_mats[cur_bone][LOCATION]);
    } else {
      glm_mat4_copy(ent->final_b_mats[collider_root_bone],
                    ent->final_b_mats[cur_bone]);
    }
  }
}

size_t get_sim_collisions(SIMULATION *sim, COLLISION **dest, vec3 origin,
                          float range, int get_col_info) {
  pthread_mutex_t col_lock;
  pthread_mutex_init(&col_lock, NULL);
  COLLISION *collisions = malloc(sizeof(COLLISION) * BUFF_STARTING_LEN);
  size_t buf_len = 0;
  size_t buf_size = BUFF_STARTING_LEN;

  int status = 0;
  ENTITY *cur_ent = NULL;
  size_t collider_offset = 0;

  // Update placement of neccesarry driving entities
  LEDGER_INPUT input;
  for (size_t i = 0; i < sim->num_col_driving; i++) {
    cur_ent = sim->dcol_ledger[sim->dcol_list[i]].col.entity;
    collider_offset = sim->dcol_ledger[sim->dcol_list[i]].col.collider_offset;

    if (is_moving(cur_ent, collider_offset)) {
      input.collider.ent = cur_ent;
      input.collider.col = collider_offset;
      status = ledger_add(&sim->mcol_ledger, &sim->mcol_list,
                          &sim->num_col_moving, &sim->mcol_ledger_size,
                          &sim->mcol_list_size, input, L_TYPE_COLLIDER);
      if (status) {
        *dest = NULL;
        return 0;
      }
    }
  }

  // Flag all moving entities for deletion. Flags will be flipped back if any
  // collider of an entity is moving
  for (size_t i = 0; i < sim->num_ent_moving; i++) {
    sim->ment_ledger[sim->ment_list[i]].ent.to_delete = 1;
  }

  /*
  // Detect collisions for all moving entities
  pthread_t t1;
  C_ARGS t1_args;
  t1_args.col_lock = &col_lock;
  t1_args.sim = sim;
  t1_args.start = 0;
  //t1_args.end = sim->num_col_moving / 3;
  t1_args.end = sim->num_col_moving / 2;
  //t1_args.end = sim->num_col_moving;
  t1_args.collisions = &collisions;
  t1_args.buf_len = &buf_len;
  t1_args.buf_size = &buf_size;
  glm_vec3_copy(origin, t1_args.origin);
  t1_args.range = range;
  t1_args.get_col_info = get_col_info;

  pthread_t t2;
  C_ARGS t2_args;
  memcpy(&t2_args, &t1_args, sizeof(C_ARGS));
  t2_args.start = t1_args.end;
  //t2_args.end = t2_args.start + (sim->num_col_moving / 3);
  t2_args.end = sim->num_col_moving;
  */

  /*
  pthread_t t3;
  C_ARGS t3_args;
  memcpy(&t3_args, &t1_args, sizeof(C_ARGS));
  t3_args.start = t2_args.end;
  t3_args.end = sim->num_col_moving;
  */

  /*
  pthread_create(&t1, NULL, check_moving_buffer, &t1_args);
  pthread_create(&t2, NULL, check_moving_buffer, &t2_args);
  //pthread_create(&t3, NULL, check_moving_buffer, &t3_args);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  //pthread_join(t3, NULL);
  */
  C_ARGS args;
  args.col_lock = &col_lock;
  args.sim = sim;
  args.start = 0;
  args.end = sim->num_col_moving;
  args.collisions = &collisions;
  args.buf_len = &buf_len;
  args.buf_size = &buf_size;
  glm_vec3_copy(origin, args.origin);
  args.range = range;
  args.get_col_info = get_col_info;
  check_moving_buffer(&args);

  // Clean up moving collider buffer
  for (size_t i = 0; i < sim->num_col_moving; i++) {
    if (sim->mcol_ledger[sim->mcol_list[i]].col.to_delete) {
      ledger_delete_direct(sim->mcol_ledger, sim->mcol_list,
                           &sim->num_col_moving, i, L_TYPE_COLLIDER);
      i--;
    }
  }
  // Clean up moving entity buffer
  for (size_t i = 0; i < sim->num_ent_moving; i++) {
    if (sim->ment_ledger[sim->ment_list[i]].ent.to_delete) {
      ledger_delete_direct(sim->ment_ledger, sim->ment_list,
                           &sim->num_ent_moving, i, L_TYPE_ENTITY);
      i--;
    }
  }

  *dest = collisions;
  return buf_len;
}

size_t sim_get_nearby(SIMULATION *sim, COLLISION **dest, vec3 pos,
                      float range) {
  COLLISION *collisions = malloc(sizeof(COLLISION) * BUFF_STARTING_LEN);
  size_t buf_len = 0;
  size_t buf_size = BUFF_STARTING_LEN;

  int status = 0;

  // Spoof an entity which will act as our search sphere
  COLLIDER col;
  memset(&col, 0, sizeof(COLLIDER));
  glm_vec3_copy(pos, col.data.center);
  col.data.radius = range;
  col.children_offset = -1;
  col.num_children = 0;
  col.type = SPHERE;
  col.category = DEFAULT;

  int col_bone_link = -1;
  MODEL model;
  memset(&model, 0, sizeof(MODEL));
  model.colliders = &col;
  model.num_colliders = 1;
  model.collider_bone_links = &col_bone_link;

  ENTITY ent;
  memset(&ent, 0, sizeof(ENTITY));
  ent.model = &model;

  status = get_collider_collisions(sim, &ent, 0, &collisions, &buf_len,
                                   &buf_size, 0, NULL);
  if (status) {
    *dest = NULL;
    return 0;
  }

  for (size_t i = 0; i < buf_len; i++) {
    collisions[i].a_ent = NULL;
    collisions[i].a_offset = INVALID_INDEX;
  }

  *dest = collisions;
  return buf_len;
}

void impulse_resolution(SIMULATION *sim, COLLISION col, vec3 a_dest_vel,
                        vec3 b_dest_vel) {
  COL_ARGS a_args;
  a_args.vel_dest = a_dest_vel;
  a_args.velocity = col.a_ent->np_data[col.a_offset].v;
  a_args.ang_velocity = col.a_ent->np_data[col.a_offset].ang_v;
  a_args.collider = col.a_offset;
  a_args.type = col.a_ent->type;
  int bone = col.a_ent->model->collider_bone_links[col.a_offset];
  glm_mat4_quat(col.a_ent->bone_mats[bone][ROTATION], a_args.rotation);
  a_args.inv_mass = col.a_ent->np_data[col.a_offset].inv_mass;
  if (a_args.inv_mass) {
    calc_inertia_tensor(col.a_ent, col.a_offset, a_args.inv_mass,
                        a_args.inv_inertia);
    glm_mat4_inv(a_args.inv_inertia, a_args.inv_inertia);
  } else {
    glm_mat4_zero(a_args.inv_inertia);
  }
  glm_mat4_mulv3(col.a_ent->final_b_mats[bone],
                 col.a_ent->model->bones[bone].base, 1.0,
                 a_args.center_of_rotation);

  COL_ARGS b_args;
  b_args.vel_dest = b_dest_vel;
  b_args.velocity = col.b_ent->np_data[col.b_offset].v;
  b_args.ang_velocity = col.b_ent->np_data[col.b_offset].ang_v;
  b_args.collider = col.b_offset;
  b_args.type = col.b_ent->type;
  bone = col.b_ent->model->collider_bone_links[col.b_offset];
  glm_mat4_quat(col.b_ent->bone_mats[bone][ROTATION], b_args.rotation);
  b_args.inv_mass = col.b_ent->np_data[col.b_offset].inv_mass;
  if (b_args.inv_mass) {
    calc_inertia_tensor(col.b_ent, col.b_offset, b_args.inv_mass,
                        b_args.inv_inertia);
    glm_mat4_inv(b_args.inv_inertia, b_args.inv_inertia);
  } else {
    glm_mat4_zero(b_args.inv_inertia);
  }
  glm_mat4_mulv3(col.b_ent->final_b_mats[bone],
                 col.b_ent->model->bones[bone].base, 1.0,
                 b_args.center_of_rotation);

  solve_collision(&a_args, &b_args, col.col_dir, col.col_point, sim->forces);
}

// Manually refresh the status of an entity in the simulation. Must be called
// before refresh_collider()
void prep_refresh(SIMULATION *sim, ENTITY *ent) {
  LEDGER_INPUT input;
  input.ent = ent;
  ledger_delete(sim->ment_ledger, sim->ment_list, sim->ment_ledger_size,
                &sim->num_ent_moving, input, L_TYPE_ENTITY);
}

// Manually refresh the status of the collider in the simulation
void refresh_collider(SIMULATION *sim, ENTITY *ent, size_t offset) {
  LEDGER_INPUT input;
  input.collider.ent = ent;
  input.collider.col = offset;
  if (is_moving(ent, offset)) {
    ledger_add(&sim->mcol_ledger, &sim->mcol_list, &sim->num_col_moving,
               &sim->mcol_ledger_size, &sim->mcol_list_size, input,
               L_TYPE_COLLIDER);
    input.ent = ent;
    ledger_add(&sim->ment_ledger, &sim->ment_list, &sim->num_ent_moving,
               &sim->ment_ledger_size, &sim->ment_list_size, input,
               L_TYPE_ENTITY);
  } else {
    ledger_delete(sim->mcol_ledger, sim->mcol_list, sim->mcol_ledger_size,
                   &sim->num_col_moving, input, L_TYPE_COLLIDER);
  }
}

// ============================== MISC HELPERS ===============================

void global_collider(ENTITY *ent, size_t collider_offset, COLLIDER *dest) {
  COLLIDER *raw_col = ent->model->colliders + collider_offset;
  mat4 bone_to_entity = GLM_MAT4_IDENTITY_INIT;
  mat4 entity_to_world = GLM_MAT4_IDENTITY_INIT;
  int bone = ent->model->collider_bone_links[collider_offset];
  glm_mat4_copy(ent->final_b_mats[bone], entity_to_world);
  glm_mat4_ins3(ent->model->bones[bone].coordinate_matrix, bone_to_entity);
  if (raw_col->type == POLY) {
    glm_vec4(raw_col->data.center_of_mass, 1.0, bone_to_entity[3]);
  } else {
    glm_vec4(raw_col->data.center, 1.0, bone_to_entity[3]);
  }

  dest->type = raw_col->type;
  dest->category = raw_col->category;
  dest->children_offset = 0;
  dest->num_children = 0;
  if (dest->type == POLY) {
    dest->data.num_used = raw_col->data.num_used;
    for (int i = 0; i < raw_col->data.num_used; i++) {
      glm_mat4_mulv3(bone_to_entity, raw_col->data.verts[i], 1.0,
                     dest->data.verts[i]);
      glm_mat4_mulv3(entity_to_world, dest->data.verts[i], 1.0,
                     dest->data.verts[i]);
    }
    glm_mat4_mulv3(entity_to_world, raw_col->data.center_of_mass, 1.0,
                   dest->data.center_of_mass);
  } else if (dest->type == SPHERE) {
    dest->data.radius = raw_col->data.radius;
    glm_mat4_mulv3(entity_to_world, raw_col->data.center, 1.0,
                   dest->data.center);
  }
  if (ent->model->colliders[collider_offset].type == SPHERE) {
    dest->data.radius *= ent->bone_mats[bone][SCALE][0][0];
  }
}

void *check_moving_buffer(void *args) {
  C_ARGS arg_data = *((C_ARGS *) args);
  SIMULATION *sim = arg_data.sim;
  COLLISION **collisions = arg_data.collisions;
  size_t *buf_len = arg_data.buf_len;
  size_t *buf_size = arg_data.buf_size;
  vec3 origin = GLM_VEC3_ZERO_INIT;
  glm_vec3_copy(arg_data.origin, origin);
  float range = arg_data.range;
  int get_col_info = arg_data.get_col_info;

  int status = 0;
  COLLIDER cur_col;
  memset(&cur_col, 0, sizeof(COLLIDER));
  ENTITY *cur_ent = NULL;
  size_t collider_offset = 0;
  LEDGER_INPUT input;
  size_t index = 0;

  for (size_t i = arg_data.start; i < arg_data.end; i++) {
    cur_ent = sim->mcol_ledger[sim->mcol_list[i]].col.entity;
    collider_offset = sim->mcol_ledger[sim->mcol_list[i]].col.collider_offset;

    // Only consider collider if it is within range
    global_collider(cur_ent, collider_offset, &cur_col);
    if ((range != SIM_RANGE_INF && cur_col.type == POLY &&
        glm_vec3_distance(origin, cur_col.data.center_of_mass) > range) ||
        (range != SIM_RANGE_INF && cur_col.type == SPHERE &&
        glm_vec3_distance(origin, cur_col.data.center) > range)) {
      continue;
    }

    if (is_moving(cur_ent, collider_offset)) {
      // Check collisions
      status = get_collider_collisions(sim, cur_ent, collider_offset,
                                       collisions, buf_len, buf_size,
                                       get_col_info, arg_data.col_lock);
      if (status) {
        return (void *) -1;
      }

      // Collider is moving, therefore do not remove the collider's entity from
      // the simulations moving buffer
      input.ent = cur_ent;
      index = ledger_search(sim->ment_ledger, sim->ment_ledger_size, input,
                            L_TYPE_ENTITY);
      if (index != INVALID_INDEX) {
        sim->ment_ledger[index].ent.to_delete = 0;
      } else {
        fprintf(stderr, "Error: Simulation collider/entity pairity broken\n");
      }
    } else {
      sim->mcol_ledger[sim->mcol_list[i]].col.to_delete = 1;
    }
  }

  return 0;
}

int get_collider_collisions(SIMULATION *sim, ENTITY *subject,
                            size_t collider_offset, COLLISION **col,
                            size_t *col_buf_len, size_t *col_buf_size,
                            int get_col_info, pthread_mutex_t *col_lock) {
  // Calculate world space collider of subject
  COLLIDER s_world_col;
  memset(&s_world_col, 0, sizeof(COLLIDER));
  global_collider(subject, collider_offset, &s_world_col);

  COLLISION_RES col_res = oct_tree_search(sim->oct_tree, &s_world_col);
  //fprintf(stderr, "%ld\n", col_res.list_len);

  PHYS_OBJ *p_obj = NULL;
  ENTITY *candidate_ent = NULL;
  COLLIDER c_world_col;
  memset(&c_world_col, 0, sizeof(COLLIDER));

  vec3 simplex[4] = { GLM_VEC3_ZERO_INIT, GLM_VEC3_ZERO_INIT,
                      GLM_VEC3_ZERO_INIT, GLM_VEC3_ZERO_INIT };
  vec3 collision_dir = GLM_VEC3_ZERO_INIT;
  vec3 col_point = GLM_VEC3_ZERO_INIT;
  float collision_depth = 0.0;

  int collision = 0;
  int status = 0;

  size_t collisions = 0;
  for (size_t i = 0; i < col_res.list_len; i++) {
    p_obj = col_res.list[i];
    candidate_ent = p_obj->entity;

    // Calculate world space collider of candidate
    global_collider(candidate_ent, p_obj->collider_offset, &c_world_col);

    if (candidate_ent != subject ||
        ((subject->type & T_DRIVING) == 0 &&
         p_obj->collider_offset != collider_offset)) {
      collision = collision_check(&s_world_col, &c_world_col, simplex);
      if (collision) {
        collisions++;
        if (get_col_info) {
          status = epa_response(&s_world_col, &c_world_col, simplex,
                                collision_dir, &collision_depth);
          if (status) {
            free(col_res.list);
            return -1;
          }
          vec3_remove_noise(collision_dir, 0.000001);
          glm_vec3_scale_as(collision_dir, collision_depth, collision_dir);
          collision_point(&s_world_col, &c_world_col, collision_dir,
                          col_point);
        } else {
          glm_vec3_zero(collision_dir);
          glm_vec3_zero(col_point);
          collision_depth = 0.0;
        }

        if (col_lock) {
          pthread_mutex_lock(col_lock);
        }
        COLLISION *new_col = (*col) + (*col_buf_len);
        new_col->a_ent = subject;
        new_col->b_ent = candidate_ent;
        new_col->a_offset = collider_offset;
        new_col->b_offset = p_obj->collider_offset;
        glm_vec3_copy(collision_dir, new_col->col_dir);
        glm_vec3_copy(col_point, new_col->col_point);
        //glm_vec3_copy(col_point, test_col_pt);

        (*col_buf_len)++;
        if (*col_buf_len == *col_buf_size) {
          status = double_buffer((void **) col, col_buf_size,
                                 sizeof(COLLISION));
          if (status) {
            fprintf(stderr, "Error: Unable to reallocate collision buffer\n");
            free(col_res.list);
            return -1;
          }
        }
        if (col_lock) {
          pthread_mutex_unlock(col_lock);
        }
      }
    }
  }

  free(col_res.list);

  return 0;
}

int entity_in_range(SIMULATION *sim, ENTITY *ent, vec3 origin, float range) {
  // TODO Might be too out aggressive of a range enforcement
  COLLIDER cur_col;
  memset(&cur_col, 0, sizeof(COLLIDER));
  for (size_t i = 0; i < ent->model->num_colliders; i++) {
    // Only consider collider if it is within range
    global_collider(ent, i, &cur_col);
    if (!(range != SIM_RANGE_INF && cur_col.type == POLY &&
          glm_vec3_distance(origin, cur_col.data.center_of_mass) > range) &&
        !(range != SIM_RANGE_INF && cur_col.type == SPHERE &&
          glm_vec3_distance(origin, cur_col.data.center) > range)) {
      return 0;
    }
  }

  return 1;
}

int is_moving(ENTITY *ent, size_t col) {
  float mag = ent->np_data[col].vel_angle * ent->np_data[col].vel_angle;
  size_t zj_offset = ent->np_data[col].zero_joint_offset;
  size_t num_zj = ent->np_data[col].num_z_joints;
  for (size_t i = 0; i < num_zj; i++) {
    mag += (ent->zj_data[zj_offset + i].vel_angle *
            ent->zj_data[zj_offset + i].vel_angle);
  }
  mag = sqrt(mag);
  return mag > MOVING_THRESHOLD;
}

// =========================== BOOK KEEPING HELPERS ==========================

int ledger_init(SIM_ITEM **ledger, size_t **l_list, size_t *num_items,
                size_t *ledger_size, size_t *list_size) {
  *ledger = malloc(sizeof(SIM_ITEM) * HASH_MAP_STARTING_LEN);
  if ((*ledger) == NULL) {
    return -1;
  }
  memset(*ledger, 0, sizeof(SIM_ITEM) * HASH_MAP_STARTING_LEN);

  *l_list = malloc(sizeof(size_t) * BUFF_STARTING_LEN);
  if((*l_list) == NULL) {
    free(*ledger);
    return -1;
  }

  *num_items = 0;
  *ledger_size = HASH_MAP_STARTING_LEN;
  *list_size = BUFF_STARTING_LEN;
  return 0;
}

size_t hash_item(double key, size_t i, size_t size) {
  size_t ret = size * ((key * HASH_MAGIC_NUM) - floor(key * HASH_MAGIC_NUM));
  return (ret + i) % size;
}

int ledger_add(SIM_ITEM **ledger, size_t **l_list,
               size_t *num_items, size_t *ledger_size, size_t *list_size,
               LEDGER_INPUT l_data, int l_type) {
  size_t index = ledger_search(*ledger, *ledger_size, l_data, l_type);
  if (index != INVALID_INDEX) {
    return 0;
  }

  size_t i = 0;
  double key = 0.0;
  while (1) {
    if (l_type == L_TYPE_ENTITY) {
      key = (size_t) l_data.ent;
    } else {
      key = ((size_t) l_data.collider.ent) + l_data.collider.col;
    }
    index = hash_item(key, i, *ledger_size);
    if (l_type == L_TYPE_ENTITY) {
      if ((*ledger)[index].ent.status != LEDGER_OCCUPIED) {
        (*ledger)[index].ent.entity = l_data.ent;
        (*ledger)[index].ent.index = *num_items;
        (*ledger)[index].ent.status = LEDGER_OCCUPIED;
        (*ledger)[index].ent.to_delete = 0;
        break;
      }
    } else {
      if ((*ledger)[index].col.status != LEDGER_OCCUPIED) {
        (*ledger)[index].col.entity = l_data.collider.ent;
        (*ledger)[index].col.collider_offset = l_data.collider.col;
        (*ledger)[index].col.index = *num_items;
        (*ledger)[index].col.status = LEDGER_OCCUPIED;
        (*ledger)[index].col.to_delete = 0;
        break;
      }
    }
    i++;
  }

  int status = 0;
  (*l_list)[*num_items] = index;
  (*num_items)++;
  if (*num_items == *list_size) {
    status = double_buffer((void **) l_list, list_size, sizeof(size_t));
    if (status) {
      if (l_type == L_TYPE_ENTITY) {
        (*ledger)[index].ent.status = LEDGER_FREE;
      } else {
        (*ledger)[index].col.status = LEDGER_FREE;
      }
      (*num_items)--;
      return -1;
    }
  }

  double load_factor = ((double) (*num_items)) / ((double) (*ledger_size));
  if (load_factor > 0.5) {
    status = resize_ledger(ledger, *l_list, ledger_size, *num_items, l_type);
    if (status) {
      if (l_type == L_TYPE_ENTITY) {
        (*ledger)[index].ent.status = LEDGER_FREE;
      } else {
        (*ledger)[index].col.status = LEDGER_FREE;
      }
      (*num_items)--;
      return -1;
    }
  }

  return 0;
}

size_t ledger_search(SIM_ITEM *ledger, size_t ledger_size,
                     LEDGER_INPUT l_data, int l_type) {
  double key = 0;
  if (l_type == L_TYPE_ENTITY) {
    key = (size_t) l_data.ent;
  } else {
    key = ((size_t) l_data.collider.ent) + l_data.collider.col;
  }

  size_t i = 0;
  size_t index = 0;
  while (1) {
    index = hash_item(key, i, ledger_size);
    if (l_type == L_TYPE_ENTITY) {
      if (ledger[index].ent.status == LEDGER_FREE) {
        break;
      } else if (ledger[index].ent.status == LEDGER_OCCUPIED &&
                 ledger[index].ent.entity == l_data.ent) {
        return index;
      }
    } else {
      if (ledger[index].col.status == LEDGER_FREE) {
        break;
      } else if (ledger[index].col.status == LEDGER_OCCUPIED &&
                 ledger[index].col.entity == l_data.collider.ent &&
                 ledger[index].col.collider_offset == l_data.collider.col) {
        return index;
      }
    }
    i++;
  }
  return INVALID_INDEX;
}

void ledger_delete(SIM_ITEM *ledger, size_t *l_list, size_t ledger_size,
                   size_t *num_items, LEDGER_INPUT l_data, int l_type) {
  size_t index = ledger_search(ledger, ledger_size, l_data, l_type);
  if (index != INVALID_INDEX) {
    (*num_items)--;
    if (l_type == L_TYPE_ENTITY) {
      ledger[index].ent.status = LEDGER_DELETED;
      l_list[ledger[index].ent.index] = l_list[*num_items];
      ledger[l_list[*num_items]].ent.index = ledger[index].ent.index;
    } else {
      ledger[index].col.status = LEDGER_DELETED;
      l_list[ledger[index].col.index] = l_list[*num_items];
      ledger[l_list[*num_items]].col.index = ledger[index].col.index;
    }
  }
}

void ledger_delete_direct(SIM_ITEM *ledger, size_t *l_list,
                          size_t *num_items, size_t index, int l_type) {
  (*num_items)--;
  if (l_type == L_TYPE_ENTITY) {
    ledger[l_list[index]].ent.status = LEDGER_DELETED;
    l_list[index] = l_list[*num_items];
    ledger[l_list[*num_items]].ent.index = index;
  } else {
    ledger[l_list[index]].col.status = LEDGER_DELETED;
    l_list[index] = l_list[*num_items];
    ledger[l_list[*num_items]].col.index = index;
  }
}

int resize_ledger(SIM_ITEM **ledger, size_t *l_list, size_t *ledger_size,
                  size_t num_items, int l_type) {
  SIM_ITEM *new_ledger = malloc(sizeof(SIM_ITEM) * 2 * *ledger_size);
  if (new_ledger == NULL) {
    return -1;
  }
  (*ledger_size) *= 2;
  memset(new_ledger, 0, sizeof(SIM_ITEM) * *ledger_size);

  size_t j = 0;
  size_t index = 0;
  ENTITY *cur_ent = NULL;
  size_t cur_col = 0;
  int cur_del = 0;
  double key = 0.0;
  for (size_t i = 0; i < num_items; i++) {
    if (l_type == L_TYPE_ENTITY) {
      cur_ent = (*ledger)[l_list[i]].ent.entity;
      cur_del = (*ledger)[l_list[i]].ent.to_delete;
      key = (size_t) cur_ent;
    } else {
      cur_ent = (*ledger)[l_list[i]].col.entity;
      cur_col = (*ledger)[l_list[i]].col.collider_offset;
      cur_del = (*ledger)[l_list[i]].col.to_delete;
      key = ((size_t) cur_ent) + cur_col;
    }

    j = 0;
    while (1) {
      index = hash_item(key, j, *ledger_size);
      if (l_type == L_TYPE_ENTITY) {
        if (new_ledger[index].ent.status != LEDGER_OCCUPIED) {
          new_ledger[index].ent.entity = cur_ent;
          new_ledger[index].ent.index = i;
          new_ledger[index].ent.status = LEDGER_OCCUPIED;
          new_ledger[index].ent.to_delete = cur_del;
          break;
        }
      } else {
        if (new_ledger[index].col.status != LEDGER_OCCUPIED) {
          new_ledger[index].col.entity = cur_ent;
          new_ledger[index].col.collider_offset = cur_col;
          new_ledger[index].col.index = i;
          new_ledger[index].col.status = LEDGER_OCCUPIED;
          new_ledger[index].col.to_delete = cur_del;
          break;
        }
      }
      j++;
    }
    l_list[i] = index;
  }

  free(*ledger);
  *ledger = new_ledger;
  return 0;
}

