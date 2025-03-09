#include <physics/simulation.h>

// ========================== CREATION AND DELETION ==========================

SIMULATION *init_sim(float max_extent, unsigned int max_depth) {
  SIMULATION *sim = malloc(sizeof(SIMULATION));
  if (sim == NULL) {
    fprintf(stderr, "Error: Unable to allocate simulation\n");
    goto err_init;
  }
  memset(sim->linked_sims, 0, sizeof(SIMULATION *) * MAX_LINKED_SIMS);
  sim->num_linked_sims = 0;

  sim->oct_tree = init_tree(max_extent, max_depth);
  if (sim->oct_tree == NULL) {
    fprintf(stderr, "Error: Unable to initailize simulation oct-tree\n");
    goto err_oct_tree;
  }

  int status = ledger_init(&sim->ment_ledger, &sim->ment_list,
                           &sim->num_ent_moving, &sim->ment_ledger_size,
                           &sim->ment_list_size);
  if (status) {
    fprintf(stderr, "Error: Unable to allocate simulation ledgers\n");
    goto err_ment;
  }

  status = ledger_init(&sim->mcol_ledger, &sim->mcol_list,
                       &sim->num_col_moving, &sim->mcol_ledger_size,
                       &sim->mcol_list_size);
  if (status) {
    fprintf(stderr, "Error: Unable to allocate simulation ledgers\n");
    goto err_mcol;
  }

  status = ledger_init(&sim->dcol_ledger, &sim->dcol_list,
                       &sim->num_col_driving, &sim->dcol_ledger_size,
                       &sim->dcol_list_size);
  if (status) {
    fprintf(stderr, "Error: Unable to allocate simulation ledgers\n");
    goto err_dcol;
  }

  status = ledger_init(&sim->ent_ledger, &sim->ent_list, &sim->num_ent,
                       &sim->ent_ledger_size, &sim->ent_list_size);
  if (status) {
    fprintf(stderr, "Error: Unable to allocate simulation ledgers\n");
    goto err_ent;
  }

  glm_vec3_zero(sim->forces);
  goto ret;

err_ent:
  free(sim->dcol_ledger);
  free(sim->dcol_list);
err_dcol:
  free(sim->mcol_ledger);
  free(sim->mcol_list);
err_mcol:
  free(sim->ment_ledger);
  free(sim->ment_list);
err_ment:
  free_oct_tree(sim->oct_tree);
err_oct_tree:
  free(sim);
err_init:
  return NULL;
ret:
  return sim;
}

void free_sim(SIMULATION *sim) {
  if (sim == NULL) {
    return;
  }

  free_oct_tree(sim->oct_tree);
  free(sim->ent_ledger);
  free(sim->ent_list);
  free(sim->ment_ledger);
  free(sim->ment_list);
  free(sim->mcol_ledger);
  free(sim->mcol_list);
  free(sim->dcol_ledger);
  free(sim->dcol_list);
  free(sim);
}

// ============================ SIMULATION HELPERS ===========================

int link_sim(SIMULATION *sim, SIMULATION *target) {
  if (!sim || sim->num_linked_sims >= MAX_LINKED_SIMS) {
    return -1;
  }

  sim->linked_sims[sim->num_linked_sims] = target;
  sim->num_linked_sims++;

  return 0;
}

int unlink_sim(SIMULATION *sim, SIMULATION *target) {
  if (!sim) {
    return -1;
  }

  for (int i = 0; i < sim->num_linked_sims; i++) {
    if (sim->linked_sims[i] == target) {
      sim->num_linked_sims--;
      sim->linked_sims[i] = sim->linked_sims[sim->num_linked_sims];
      sim->linked_sims[sim->num_linked_sims] = NULL;
      return 0;
    }
  }

  return 1;
}

int sim_add_entity(SIMULATION *sim, ENTITY *entity, size_t collider_filter) {
  if (entity == NULL || sim == NULL) {
    return -1;
  }

  int status = 0;
  LEDGER_INPUT input;

  input.entity.ent = entity;
  input.entity.data = (void *) collider_filter;
  status = ledger_add(&sim->ent_ledger, &sim->ent_list, &sim->num_ent,
                      &sim->ent_ledger_size, &sim->ent_list_size, input,
                      L_TYPE_ENTITY);
  if (status) {
    fprintf(stderr, "Error: Unable to reallocate simulation ledger\n");
    return -1;
  }

  // Add desired colliders to the simulation
  COLLIDER *cur_col = NULL;
  for (size_t i = 0; i < entity->model->num_colliders; i++) {
    input.collider.ent = entity;
    input.collider.col = i;
    input.collider.data = NULL;

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
      if (entity->is_moving_cb(entity, i)) {
        status = ledger_add(&sim->mcol_ledger, &sim->mcol_list,
                            &sim->num_col_moving, &sim->mcol_ledger_size,
                            &sim->mcol_list_size, input, L_TYPE_COLLIDER);
        if (status) {
          fprintf(stderr, "Error: Unable to reallocate simulation ledger\n");
          return -1;
        }

        input.entity.ent = entity;
        input.entity.data = NULL;
        status = ledger_add(&sim->ment_ledger, &sim->ment_list,
                            &sim->num_ent_moving, &sim->ment_ledger_size,
                            &sim->ment_list_size, input, L_TYPE_ENTITY);
        if (status) {
          fprintf(stderr, "Error: Unable to reallocate simulation ledger\n");
          return -1;
        }

        status = propagate_new_mcol(sim, entity, i);
        if (status) {
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
  input.entity.ent = entity;
  ledger_delete(sim->ment_ledger, sim->ment_list, sim->ment_ledger_size,
                &sim->num_ent_moving, input, L_TYPE_ENTITY);
  ledger_delete(sim->ent_ledger, sim->ent_list, sim->ent_ledger_size,
                &sim->num_ent, input, L_TYPE_ENTITY);

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
  SIM_ITEM *item = NULL;
  for (size_t i = 0; i < sim->num_col_moving; i++) {
    item = sim->mcol_ledger + sim->mcol_list[i];
    oct_tree_insert(sim->oct_tree, item->col.entity, item->col.collider_offset,
                    birthmark);
  }
}
#else
void update_sim_movement(SIMULATION *sim) {
  SIM_ITEM *item = NULL;
  for (size_t i = 0; i < sim->num_col_moving; i++) {
    item = sim->mcol_ledger + sim->mcol_list[i];
    oct_tree_insert(sim->oct_tree, item->col.entity,
                    item->col.collider_offset);
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
    sim_state[i].entity = cur_ent;
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

void restore_sim_state(SIMULATION *sim, SIM_STATE *state, size_t state_size) {
  ENTITY *cur_ent = NULL;
  P_DATA *p_data = NULL;
  ZERO_JOINT *z_data = NULL;
  for (size_t i = 0; i < state_size; i++) {
    cur_ent = state[i].entity;
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

  free_sim_state(state, state_size);
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
  void (*move_cb)(ENTITY *, vec3) = NULL;
  for (size_t i = 0; i < sim->num_ent_moving; i++) {
    cur_ent = sim->ment_ledger[sim->ment_list[i]].ent.entity;
    move_cb = cur_ent->move_cb;
    if (range == SIM_RANGE_INF ||
        entity_in_range(sim, cur_ent, origin, range)) {
      move_cb(cur_ent, sim->forces);
    }
  }

  return ret;
}

void integrate_sim(SIMULATION *sim, vec3 origin, float range) {
  ENTITY *cur_ent = NULL;
  void (*move_cb)(ENTITY *, vec3) = NULL;
  for (size_t i = 0; i < sim->num_ent_moving; i++) {
    cur_ent = sim->ment_ledger[sim->ment_list[i]].ent.entity;
    move_cb = cur_ent->move_cb;
    if (range == SIM_RANGE_INF ||
        entity_in_range(sim, cur_ent, origin, range)) {
      move_cb(cur_ent, sim->forces);
      cur_ent->num_cons = 0;
    }
  }
}

size_t get_sim_collisions(SIMULATION *sim, COLLISION **dest, vec3 origin,
                          float range, int get_col_info) {
  pthread_mutex_t col_lock;
  pthread_mutex_init(&col_lock, NULL);
  COL_UPDATE *collisions = malloc(sizeof(COL_UPDATE) * BUFF_STARTING_LEN);
  size_t buf_len = 0;
  size_t buf_size = BUFF_STARTING_LEN;

  int status = 0;
  ENTITY *cur_ent = NULL;
  size_t collider_offset = 0;
  int (*is_moving_cb)(ENTITY *, size_t) = NULL;

  // Update placement of neccesarry driving entities
  LEDGER_INPUT input;
  for (size_t i = 0; i < sim->num_col_driving; i++) {
    cur_ent = sim->dcol_ledger[sim->dcol_list[i]].col.entity;
    collider_offset = sim->dcol_ledger[sim->dcol_list[i]].col.collider_offset;
    is_moving_cb = cur_ent->is_moving_cb;

    if (is_moving_cb(cur_ent, collider_offset)) {
      input.collider.ent = cur_ent;
      input.collider.col = collider_offset;
      input.collider.data = NULL;
      status = ledger_add(&sim->mcol_ledger, &sim->mcol_list,
                          &sim->num_col_moving, &sim->mcol_ledger_size,
                          &sim->mcol_list_size, input, L_TYPE_COLLIDER);
      if (status) {
        *dest = NULL;
        return 0;
      }

      input.entity.ent = cur_ent;
      input.entity.data = NULL;
      status = ledger_add(&sim->ment_ledger, &sim->ment_list,
                          &sim->num_ent_moving, &sim->ment_ledger_size,
                          &sim->ment_list_size, input, L_TYPE_ENTITY);
      if (status) {
        *dest = NULL;
        return 0;
      }

      status = propagate_new_mcol(sim, cur_ent, collider_offset);
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
      propagate_rm_mcol(sim, cur_ent, collider_offset);
    }
  }
  // Clean up moving entity buffer
  for (size_t i = 0; i < sim->num_ent_moving; i++) {
    if (sim->ment_ledger[sim->ment_list[i]].ent.to_delete) {
      ledger_delete_direct(sim->ment_ledger, sim->ment_list,
                           &sim->num_ent_moving, i, L_TYPE_ENTITY);
      i--;
      propagate_rm_ment(sim, cur_ent);
    }
  }

  COLLISION *dest_buffer = malloc(sizeof(COLLISION) * buf_len);
  *dest = dest_buffer;

  // Preemtively add the second entity in each collision pair to the moving
  // ledger
  for (int i = 0; i < buf_len; i++) {
    dest_buffer[i] = collisions[i].col;

    input.collider.ent = collisions[i].col.b_ent;
    input.collider.col = collisions[i].col.b_offset;
    ledger_add(&sim->mcol_ledger, &sim->mcol_list, &sim->num_col_moving,
               &sim->mcol_ledger_size, &sim->mcol_list_size, input,
               L_TYPE_COLLIDER);
    input.entity.ent = collisions[i].col.b_ent;
    ledger_add(&sim->ment_ledger, &sim->ment_list, &sim->num_ent_moving,
               &sim->ment_ledger_size, &sim->ment_list_size, input,
               L_TYPE_ENTITY);

    propagate_new_mcol(sim, collisions[i].col.b_ent,
                       collisions[i].col.b_offset);
  }
  free(collisions);

  return buf_len;
}

size_t sim_get_nearby(SIMULATION *sim, COLLISION **dest, vec3 pos,
                      float range, int get_col_info) {
  COL_UPDATE *collisions = malloc(sizeof(COL_UPDATE) * BUFF_STARTING_LEN);
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

  BONE bone;
  memset(&bone, 0, sizeof(BONE));
  glm_mat3_identity(bone.coordinate_matrix);
  glm_vec3_copy(pos, bone.base);
  glm_vec3_add(pos, (vec3) {0.0, 1.0, 0.0}, bone.head);
  bone.parent = -1;
  bone.num_children = 0;

  int col_bone_link = 0;
  MODEL model;
  memset(&model, 0, sizeof(MODEL));
  model.colliders = &col;
  model.bones = &bone;
  model.num_colliders = 1;
  model.num_bones = 1;
  model.collider_bone_links = &col_bone_link;
  model.bone_collider_links = &col_bone_link;

  ENTITY ent;
  mat4 bone_mats[3] = { GLM_MAT4_IDENTITY_INIT, GLM_MAT4_IDENTITY_INIT,
                        GLM_MAT4_IDENTITY_INIT };
  mat4 final_b_mat = GLM_MAT4_IDENTITY_INIT;
  memset(&ent, 0, sizeof(ENTITY));
  ent.model = &model;
  ent.bone_mats = &bone_mats;
  ent.final_b_mats = &final_b_mat;

  // Perform collision check based on spoofed entity
  status = get_collider_collisions(sim, &ent, 0, &collisions, &buf_len,
                                   &buf_size, get_col_info, NULL);
  if (status) {
    *dest = NULL;
    return 0;
  }

  COLLISION *ret_cols = malloc(sizeof(COLLISION) * buf_len);
  *dest = ret_cols;

  for (size_t i = 0; i < buf_len; i++) {
    ret_cols[i] = collisions[i].col;
    ret_cols[i].a_ent = NULL;
    ret_cols[i].a_offset = INVALID_INDEX;
  }
  free(collisions);

  return buf_len;
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
    // TODO Num used is always 8
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
  COL_UPDATE **collisions = arg_data.collisions;
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
  int (*is_moving_cb)(ENTITY *, size_t) = NULL;

  LEDGER_INPUT input;
  size_t index = 0;

  for (size_t i = arg_data.start; i < arg_data.end; i++) {
    cur_ent = sim->mcol_ledger[sim->mcol_list[i]].col.entity;
    collider_offset = sim->mcol_ledger[sim->mcol_list[i]].col.collider_offset;
    is_moving_cb = cur_ent->is_moving_cb;

    // Only consider collider if it is within range
    global_collider(cur_ent, collider_offset, &cur_col);
    if ((range != SIM_RANGE_INF && cur_col.type == POLY &&
        glm_vec3_distance(origin, cur_col.data.center_of_mass) > range) ||
        (range != SIM_RANGE_INF && cur_col.type == SPHERE &&
        glm_vec3_distance(origin, cur_col.data.center) > range)) {
      continue;
    }

    if (is_moving_cb(cur_ent, collider_offset)) {
      // Check collisions
      status = get_collider_collisions(sim, cur_ent, collider_offset,
                                       collisions, buf_len, buf_size,
                                       get_col_info, arg_data.col_lock);
      if (status) {
        return (void *) -1;
      }

      // Collider is moving, therefore do not remove the collider's entity from
      // the simulations moving buffer
      input.entity.ent = cur_ent;
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
                            size_t collider_offset, COL_UPDATE **col,
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
        COLLISION *new_col = &((*col)[*col_buf_len].col);
        new_col->a_ent = subject;
        new_col->b_ent = candidate_ent;
        new_col->a_offset = collider_offset;
        new_col->b_offset = p_obj->collider_offset;
        glm_vec3_copy(collision_dir, new_col->col_dir);
        glm_vec3_copy(col_point, new_col->col_point);

        (*col_buf_len)++;
        if (*col_buf_len == *col_buf_size) {
          status = double_buffer((void **) col, col_buf_size,
                                 sizeof(COL_UPDATE));
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

// Default is_moving callback
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

int propagate_new_mcol(SIMULATION *sim, ENTITY *ent, size_t col) {
  SIMULATION *cur_sim = NULL;
  LEDGER_INPUT input;
  input.entity.ent = ent;

  int status = 0;
  size_t ent_idx = INVALID_INDEX;
  size_t collider_filter = 0;
  int col_cat = ent->model->colliders[col].category;
  for (int i = 0; i < sim->num_linked_sims; i++) {
    cur_sim = sim->linked_sims[i];
    ent_idx = ledger_search(cur_sim->ent_ledger, cur_sim->ent_ledger_size,
                            input, L_TYPE_ENTITY);
    if (ent_idx == INVALID_INDEX) {
      continue;
    }

    collider_filter = (size_t) cur_sim->ent_ledger[ent_idx].ent.data;
    if ((col_cat == DEFAULT && !(collider_filter & ALLOW_DEFAULT)) ||
        (col_cat == HIT_BOX && !(collider_filter & ALLOW_HIT_BOXES)) ||
        (col_cat == HURT_BOX && !(collider_filter & ALLOW_HURT_BOXES))) {
      continue;
    }

    input.collider.ent = ent;
    input.collider.col = col;
    input.collider.data = NULL;
    status = ledger_add(&cur_sim->mcol_ledger, &cur_sim->mcol_list,
                        &cur_sim->num_col_moving, &cur_sim->mcol_ledger_size,
                        &cur_sim->mcol_list_size, input, L_TYPE_COLLIDER);
    if (status) {
      fprintf(stderr,
              "Error: Unable to reallocate linked simulation ledger\n");
      return -1;
    }

    input.entity.ent = ent;
    input.entity.data = NULL;
    status = ledger_add(&cur_sim->ment_ledger, &cur_sim->ment_list,
                        &cur_sim->num_ent_moving, &cur_sim->ment_ledger_size,
                        &cur_sim->ment_list_size, input, L_TYPE_ENTITY);
    if (status) {
      fprintf(stderr,
              "Error: Unable to reallocate linked simulation ledger\n");
      return -1;
    }
  }
  return 0;
}

void propagate_rm_mcol(SIMULATION *sim, ENTITY *ent, size_t col) {
  LEDGER_INPUT input;
  input.collider.ent = ent;
  input.collider.col = col;
  SIMULATION *cur_sim = NULL;
  for (int i = 0; i < sim->num_linked_sims; i++) {
    cur_sim = sim->linked_sims[i];
    ledger_delete(cur_sim->mcol_ledger, cur_sim->mcol_list,
                  cur_sim->mcol_ledger_size, &cur_sim->num_col_moving, input,
                  L_TYPE_COLLIDER);
  }
}

void propagate_rm_ment(SIMULATION *sim, ENTITY *ent) {
  LEDGER_INPUT input;
  input.entity.ent = ent;
  SIMULATION *cur_sim = NULL;
  for (int i = 0; i < sim->num_linked_sims; i++) {
    cur_sim = sim->linked_sims[i];
    ledger_delete(cur_sim->ment_ledger, cur_sim->ment_list,
                  cur_sim->ment_ledger_size, &cur_sim->num_ent_moving, input,
                  L_TYPE_ENTITY);
  }
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
      key = (size_t) l_data.entity.ent;
    } else {
      key = ((size_t) l_data.collider.ent) + l_data.collider.col;
    }
    index = hash_item(key, i, *ledger_size);
    if (l_type == L_TYPE_ENTITY) {
      if ((*ledger)[index].ent.status != LEDGER_OCCUPIED) {
        (*ledger)[index].ent.entity = l_data.entity.ent;
        (*ledger)[index].ent.data = l_data.entity.data;
        (*ledger)[index].ent.index = *num_items;
        (*ledger)[index].ent.status = LEDGER_OCCUPIED;
        (*ledger)[index].ent.to_delete = 0;
        break;
      }
    } else {
      if ((*ledger)[index].col.status != LEDGER_OCCUPIED) {
        (*ledger)[index].col.entity = l_data.collider.ent;
        (*ledger)[index].col.data = l_data.collider.data;
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
    key = (size_t) l_data.entity.ent;
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
                 ledger[index].ent.entity == l_data.entity.ent) {
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
  void *data = NULL;
  int cur_del = 0;
  double key = 0.0;
  for (size_t i = 0; i < num_items; i++) {
    if (l_type == L_TYPE_ENTITY) {
      cur_ent = (*ledger)[l_list[i]].ent.entity;
      data = (*ledger)[l_list[i]].ent.data;
      cur_del = (*ledger)[l_list[i]].ent.to_delete;
      key = (size_t) cur_ent;
    } else {
      cur_ent = (*ledger)[l_list[i]].col.entity;
      data = (*ledger)[l_list[i]].col.data;
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
          new_ledger[index].ent.data = data;
          new_ledger[index].ent.index = i;
          new_ledger[index].ent.status = LEDGER_OCCUPIED;
          new_ledger[index].ent.to_delete = cur_del;
          break;
        }
      } else {
        if (new_ledger[index].col.status != LEDGER_OCCUPIED) {
          new_ledger[index].col.entity = cur_ent;
          new_ledger[index].col.data = data;
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

