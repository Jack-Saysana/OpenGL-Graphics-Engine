#include <2d/physics/simulation_2d.h>

int sim_add_entity_2d(SIMULATION *sim, ENTITY_2D *entity,
                      size_t collider_filter) {
  if (entity == NULL || sim == NULL || sim->type != SIM_2D) {
    return -1;
  }

  int status = 0;
  LEDGER_INPUT input;

  input.entity_2d.ent = entity;
  input.entity_2d.data = (void *) collider_filter;
  status = ledger_add(&sim->ent_ledger, input, L_TYPE_ENTITY_2D);
  if (status) {
    fprintf(stderr, "Error: Unable to reallocate simulation ledger\n");
    return -1;
  }

  // Add desired colliders to the simulation
  COLLIDER_2D *cur_col = NULL;
  for (size_t i = 0; i < entity->num_cols; i++) {
    input.collider_2d.ent = entity;
    input.collider_2d.col = i;
    input.collider_2d.data = NULL;

    cur_col = entity->cols + i;
    if (((collider_filter & ALLOW_DEFAULT) &&
         cur_col->category == DEFAULT) ||
        ((collider_filter & ALLOW_HURT_BOXES) &&
         cur_col->category == HURT_BOX) ||
        ((collider_filter & ALLOW_HIT_BOXES) &&
         cur_col->category == HIT_BOX)) {
      // Check if collider is driving
      if (entity->type & T_DRIVING) {
        status = ledger_add(&sim->dcol_ledger, input, L_TYPE_COLLIDER_2D);
        if (status) {
          fprintf(stderr, "Error: Unable to reallocate simulation ledger\n");
          return -1;
        }
      }

      // Check if collider is currently moving
      if (entity->is_moving_cb(entity, i)) {
        status = ledger_add(&sim->mcol_ledger, input, L_TYPE_COLLIDER_2D);
        if (status) {
          fprintf(stderr, "Error: Unable to reallocate simulation ledger\n");
          return -1;
        }

        input.entity_2d.ent = entity;
        input.entity_2d.data = NULL;
        status = ledger_add(&sim->ment_ledger, input, L_TYPE_ENTITY_2D);
        if (status) {
          fprintf(stderr, "Error: Unable to reallocate simulation ledger\n");
          return -1;
        }

        status = propagate_new_mcol_2d(sim, entity, i);
        if (status) {
          return -1;
        }
      }

      // Insert collider into oct-tree
      status = quad_tree_insert(sim->quad_tree, entity, i);
      if (status) {
        fprintf(stderr,
                "Error: Unable to insert entity into simulation quad-tree\n");
        return -1;
      }
    }
  }

  return 0;
}

int sim_remove_entity_2d(SIMULATION *sim, ENTITY_2D *entity) {
  if (sim == NULL || entity == NULL || sim->type != SIM_2D) {
    return -1;
  }

  LEDGER_INPUT input;
  input.entity_2d.ent = entity;
  ledger_delete(&sim->ment_ledger, input, L_TYPE_ENTITY_2D);
  ledger_delete(&sim->ent_ledger, input, L_TYPE_ENTITY_2D);

  // TODO perhaps filter colliders better
  input.collider_2d.ent = entity;
  for (size_t i = 0; i < entity->num_cols; i++) {
    input.collider_2d.col = i;
    ledger_delete(&sim->mcol_ledger, input, L_TYPE_COLLIDER_2D);
    ledger_delete(&sim->dcol_ledger, input, L_TYPE_COLLIDER_2D);
    quad_tree_delete(sim->quad_tree, entity, i);
  }

  return 0;
}

void sim_add_force_2d(SIMULATION *sim, vec2 force) {
  glm_vec2_add(sim->forces, force, sim->forces);
}

void prep_sim_movement_2d(SIMULATION *sim) {
  SIM_ITEM *mcol_map = sim->mcol_ledger.map;
  size_t *mcol_list = sim->mcol_ledger.list;
  for (size_t i = 0; i < sim->mcol_ledger.num_items; i++) {
    quad_tree_delete(sim->quad_tree,
                     mcol_map[mcol_list[i]].col_2d.entity,
                     mcol_map[mcol_list[i]].col_2d.collider_offset);
  }
}

void update_sim_movement_2d(SIMULATION *sim) {
  SIM_ITEM *item = NULL;
  SIM_ITEM *mcol_map = sim->mcol_ledger.map;
  size_t *mcol_list = sim->mcol_ledger.list;
  for (size_t i = 0; i < sim->mcol_ledger.num_items; i++) {
    item = mcol_map + mcol_list[i];
    quad_tree_insert(sim->quad_tree, item->col_2d.entity,
                     item->col_2d.collider_offset);
  }
}

void integrate_sim_2d(SIMULATION *sim, vec2 origin, float range) {
  if (sim->type != SIM_2D) {
    return;
  }

  ENTITY_2D *cur_ent = NULL;
  void (*move_cb)(ENTITY_2D *, vec3) = NULL;
  SIM_ITEM *ment_map = sim->ment_ledger.map;
  size_t *ment_list = sim->ment_ledger.list;
  for (size_t i = 0; i < sim->ment_ledger.num_items; i++) {
    cur_ent = ment_map[ment_list[i]].ent_2d.entity;
    move_cb = cur_ent->move_cb;
    if (range == SIM_RANGE_INF ||
        entity_in_range_2d(sim, cur_ent, origin, range)) {
      move_cb(cur_ent, sim->forces);
    }
  }
}

size_t get_sim_collisions_2d(SIMULATION *sim, COLLISION_2D **dest, vec2 origin,
                          float range, int get_col_info) {
  pthread_mutex_t col_lock;
  pthread_mutex_init(&col_lock, NULL);
  COL_UPDATE_2D *collisions = malloc(sizeof(COL_UPDATE_2D) *
                                     BUFF_STARTING_LEN);
  size_t buf_len = 0;
  size_t buf_size = BUFF_STARTING_LEN;

  int status = 0;
  ENTITY_2D *cur_ent = NULL;
  size_t collider_offset = 0;
  int (*is_moving_cb)(ENTITY_2D *, size_t) = NULL;

  // Update placement of neccesarry driving entities
  LEDGER_INPUT input;
  for (size_t i = 0; i < sim->dcol_ledger.num_items; i++) {
    SIM_ITEM *dcol_map = sim->dcol_ledger.map;
    size_t *dcol_list = sim->dcol_ledger.list;

    cur_ent = dcol_map[dcol_list[i]].col_2d.entity;
    collider_offset = dcol_map[dcol_list[i]].col_2d.collider_offset;
    is_moving_cb = cur_ent->is_moving_cb;

    if (is_moving_cb(cur_ent, collider_offset)) {
      input.collider_2d.ent = cur_ent;
      input.collider_2d.col = collider_offset;
      input.collider_2d.data = NULL;
      status = ledger_add(&sim->mcol_ledger, input, L_TYPE_COLLIDER_2D);
      if (status) {
        *dest = NULL;
        return 0;
      }

      input.entity_2d.ent = cur_ent;
      input.entity_2d.data = NULL;
      status = ledger_add(&sim->ment_ledger, input, L_TYPE_ENTITY_2D);
      if (status) {
        *dest = NULL;
        return 0;
      }

      status = propagate_new_mcol_2d(sim, cur_ent, collider_offset);
      if (status) {
        *dest = NULL;
        return 0;
      }
    }
  }

  SIM_ITEM *ment_map = sim->ment_ledger.map;
  size_t *ment_list = sim->ment_ledger.list;
  // Flag all moving entities for deletion. Flags will be flipped back if any
  // collider of an entity is moving
  for (size_t i = 0; i < sim->ment_ledger.num_items; i++) {
    ment_map[ment_list[i]].ent_2d.to_delete = 1;
  }

  /*
  // Detect collisions for all moving entities
  pthread_t t1;
  C_ARGS t1_args;
  t1_args.col_lock = &col_lock;
  t1_args.sim = sim;
  t1_args.start = 0;
  //t1_args.end = sim->mcol_ledger.num_items / 3;
  t1_args.end = sim->mcol_ledger.num_items / 2;
  //t1_args.end = sim->mcol_ledger.num_items;
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
  //t2_args.end = t2_args.start + (sim->mcol_ledger.num_items / 3);
  t2_args.end = sim->mcol_ledger.num_items;
  */

  /*
  pthread_t t3;
  C_ARGS t3_args;
  memcpy(&t3_args, &t1_args, sizeof(C_ARGS));
  t3_args.start = t2_args.end;
  t3_args.end = sim->mcol_ledger.num_items;
  */

  /*
  pthread_create(&t1, NULL, check_moving_buffer, &t1_args);
  pthread_create(&t2, NULL, check_moving_buffer, &t2_args);
  //pthread_create(&t3, NULL, check_moving_buffer, &t3_args);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  //pthread_join(t3, NULL);
  */
  C_ARGS_2D args;
  args.col_lock = &col_lock;
  args.sim = sim;
  args.start = 0;
  args.end = sim->mcol_ledger.num_items;
  args.collisions = &collisions;
  args.buf_len = &buf_len;
  args.buf_size = &buf_size;
  glm_vec2_copy(origin, args.origin);
  args.range = range;
  check_moving_buffer_2d(&args);

  SIM_ITEM *mcol_map = sim->mcol_ledger.map;
  size_t *mcol_list = sim->mcol_ledger.list;

  // Clean up moving collider buffer
  for (size_t i = 0; i < sim->mcol_ledger.num_items; i++) {
    if (mcol_map[mcol_list[i]].col_2d.to_delete) {
      ledger_delete_direct(&sim->mcol_ledger, i, L_TYPE_COLLIDER_2D);
      i--;
      propagate_rm_mcol_2d(sim, cur_ent, collider_offset);
    }
  }
  // Clean up moving entity buffer
  for (size_t i = 0; i < sim->ment_ledger.num_items; i++) {
    if (ment_map[ment_list[i]].ent_2d.to_delete) {
      ledger_delete_direct(&sim->ment_ledger, i, L_TYPE_ENTITY_2D);
      i--;
      propagate_rm_ment_2d(sim, cur_ent);
    }
  }

  COLLISION_2D *dest_buffer = malloc(sizeof(COLLISION_2D) * buf_len);
  *dest = dest_buffer;

  // Preemtively add the second entity in each collision pair to the moving
  // ledger
  for (int i = 0; i < buf_len; i++) {
    dest_buffer[i] = collisions[i].col;

    input.collider_2d.ent = collisions[i].col.b_ent;
    input.collider_2d.col = collisions[i].col.b_offset;
    ledger_add(&sim->mcol_ledger, input, L_TYPE_COLLIDER_2D);
    input.entity_2d.ent = collisions[i].col.b_ent;
    ledger_add(&sim->ment_ledger, input, L_TYPE_ENTITY_2D);

    propagate_new_mcol_2d(sim, collisions[i].col.b_ent,
                          collisions[i].col.b_offset);
  }
  free(collisions);

  return buf_len;
}

size_t sim_get_nearby_2d(SIMULATION *sim, COLLISION_2D **dest, vec2 pos,
                         float range, int get_col_info) {
  COL_UPDATE_2D *collisions = malloc(sizeof(COL_UPDATE_2D) *
                                     BUFF_STARTING_LEN);
  size_t buf_len = 0;
  size_t buf_size = BUFF_STARTING_LEN;

  int status = 0;

  // Spoof an entity which will act as our search sphere
  COLLIDER_2D col;
  memset(&col, 0, sizeof(COLLIDER_2D));
  glm_vec2_copy(pos, col.origin);
  col.data.radius = range;
  col.type = CT_CIRCLE;
  col.category = DEFAULT;

  ENTITY_2D ent;
  memset(&ent, 0, sizeof(ENTITY_2D));
  ent.cols = &col;
  ent.num_cols = 1;

  // Perform collision check based on spoofed entity
  status = get_collider_collisions_2d(sim, &ent, 0, &collisions, &buf_len,
                                      &buf_size, NULL);
  if (status) {
    *dest = NULL;
    return 0;
  }

  COLLISION_2D *ret_cols = malloc(sizeof(COLLISION_2D) * buf_len);
  *dest = ret_cols;

  for (size_t i = 0; i < buf_len; i++) {
    ret_cols[i] = collisions[i].col;
    ret_cols[i].a_ent = NULL;
    ret_cols[i].a_offset = INVALID_INDEX;
  }
  free(collisions);

  return buf_len;
}

void *check_moving_buffer_2d(void *args) {
  C_ARGS_2D arg_data = *((C_ARGS_2D *) args);
  SIMULATION *sim = arg_data.sim;
  COL_UPDATE_2D **collisions = arg_data.collisions;
  size_t *buf_len = arg_data.buf_len;
  size_t *buf_size = arg_data.buf_size;
  vec2 origin = GLM_VEC2_ZERO_INIT;
  glm_vec2_copy(arg_data.origin, origin);
  float range = arg_data.range;

  int status = 0;
  COLLIDER_2D cur_col;
  memset(&cur_col, 0, sizeof(COLLIDER_2D));

  ENTITY_2D *cur_ent = NULL;
  size_t collider_offset = 0;
  int (*is_moving_cb)(ENTITY_2D *, size_t) = NULL;

  SIM_ITEM *ment_map = sim->ment_ledger.map;
  SIM_ITEM *mcol_map = sim->mcol_ledger.map;
  size_t *mcol_list = sim->mcol_ledger.list;

  LEDGER_INPUT input;
  size_t index = 0;

  for (size_t i = arg_data.start; i < arg_data.end; i++) {
    cur_ent = mcol_map[mcol_list[i]].col_2d.entity;
    collider_offset = mcol_map[mcol_list[i]].col_2d.collider_offset;
    is_moving_cb = cur_ent->is_moving_cb;

    vec2 col_pos = { 0.0, 0.0 };
    glm_vec2_add(cur_ent->pos, cur_ent->cols[collider_offset].origin, col_pos);

    // Only consider collider if it is within range
    if (range != SIM_RANGE_INF &&
        glm_vec2_distance(origin, cur_col.origin) > range) {
      continue;
    }

    if (is_moving_cb(cur_ent, collider_offset)) {
      // Check collisions
      status = get_collider_collisions_2d(sim, cur_ent, collider_offset,
                                          collisions, buf_len, buf_size,
                                          arg_data.col_lock);
      if (status) {
        return (void *) -1;
      }

      // Collider is moving, therefore do not remove the collider's entity from
      // the simulations moving buffer
      input.entity_2d.ent = cur_ent;
      index = ledger_search(&sim->ment_ledger, input, L_TYPE_ENTITY_2D);
      if (index != INVALID_INDEX) {
        ment_map[index].ent_2d.to_delete = 0;
      } else {
        fprintf(stderr, "Error: Simulation collider/entity pairity broken\n");
      }
    } else {
      mcol_map[mcol_list[i]].col_2d.to_delete = 1;
    }
  }

  return 0;
}

int get_collider_collisions_2d(SIMULATION *sim, ENTITY_2D *subject,
                               size_t collider_offset, COL_UPDATE_2D **col,
                               size_t *col_buf_len, size_t *col_buf_size,
                               pthread_mutex_t *col_lock) {
  // Calculate world space collider of subject
  COLLIDER_2D s_world_col;
  s_world_col = subject->cols[collider_offset];
  glm_vec2_add(subject->cols[collider_offset].origin, s_world_col.origin,
               s_world_col.origin);

  COLLISION_RES_2D col_res = quad_tree_search(sim->quad_tree, &s_world_col);

  PHYS_OBJ_2D *p_obj = NULL;
  ENTITY_2D *candidate_ent = NULL;
  size_t candidate_col = 0;
  COLLIDER_2D c_world_col;
  memset(&c_world_col, 0, sizeof(COLLIDER_2D));

  vec2 collision_dir = { 0.0, 0.0 };

  int collision = 0;
  int status = 0;

  size_t collisions = 0;
  for (size_t i = 0; i < col_res.list_len; i++) {
    p_obj = col_res.list[i];
    candidate_ent = p_obj->ent;
    candidate_col = p_obj->collider_offset;

    // Calculate world space collider of candidate
    c_world_col = candidate_ent->cols[candidate_col];
    glm_vec2_add(candidate_ent->cols[candidate_col].origin, c_world_col.origin,
                 c_world_col.origin);

    if (candidate_ent != subject ||
        ((subject->type & T_DRIVING) == 0 &&
         p_obj->collider_offset != collider_offset)) {
      collision = collision_check_2d(&s_world_col, &c_world_col,
                                     collision_dir);
      if (collision) {
        collisions++;

        if (col_lock) {
          pthread_mutex_lock(col_lock);
        }
        COLLISION_2D *new_col = &((*col)[*col_buf_len].col);
        new_col->a_ent = subject;
        new_col->b_ent = candidate_ent;
        new_col->a_offset = collider_offset;
        new_col->b_offset = p_obj->collider_offset;
        glm_vec2_copy(collision_dir, new_col->col_dir);

        (*col_buf_len)++;
        if (*col_buf_len == *col_buf_size) {
          status = double_buffer((void **) col, col_buf_size,
                                 sizeof(COL_UPDATE_2D));
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

int entity_in_range_2d(SIMULATION *sim, ENTITY_2D *ent, vec2 origin,
                       float range) {
  // TODO Might be too out aggressive of a range enforcement
  for (size_t i = 0; i < ent->num_cols; i++) {
    vec2 col_pos = { 0.0, 0.0 };
    glm_vec2_add(ent->pos, ent->cols[i].origin, col_pos);

    // Only consider collider if it is within range
    if (!(range != SIM_RANGE_INF &&
          glm_vec2_distance(origin, col_pos) > range)) {
      return 0;
    }
  }

  return 1;
}

int propagate_new_mcol_2d(SIMULATION *sim, ENTITY_2D *ent, size_t col) {
  SIMULATION *cur_sim = NULL;
  SIM_ITEM *cur_ent_map = NULL;
  LEDGER_INPUT input;
  input.entity_2d.ent = ent;

  int status = 0;
  size_t ent_idx = INVALID_INDEX;
  size_t collider_filter = 0;
  int col_cat = ent->cols[col].category;
  for (int i = 0; i < sim->num_linked_sims; i++) {
    cur_sim = sim->linked_sims[i];
    cur_ent_map = cur_sim->ent_ledger.map;
    ent_idx = ledger_search(&cur_sim->ent_ledger, input, L_TYPE_ENTITY_2D);
    if (ent_idx == INVALID_INDEX) {
      continue;
    }

    collider_filter = (size_t) cur_ent_map[ent_idx].ent_2d.data;
    if ((col_cat == DEFAULT && !(collider_filter & ALLOW_DEFAULT)) ||
        (col_cat == HIT_BOX && !(collider_filter & ALLOW_HIT_BOXES)) ||
        (col_cat == HURT_BOX && !(collider_filter & ALLOW_HURT_BOXES))) {
      continue;
    }

    input.collider_2d.ent = ent;
    input.collider_2d.col = col;
    input.collider_2d.data = NULL;
    status = ledger_add(&cur_sim->mcol_ledger, input, L_TYPE_COLLIDER_2D);
    if (status) {
      fprintf(stderr,
              "Error: Unable to reallocate linked simulation ledger\n");
      return -1;
    }

    input.entity_2d.ent = ent;
    input.entity_2d.data = NULL;
    status = ledger_add(&cur_sim->ment_ledger, input, L_TYPE_ENTITY_2D);
    if (status) {
      fprintf(stderr,
              "Error: Unable to reallocate linked simulation ledger\n");
      return -1;
    }
  }
  return 0;
}

void propagate_rm_mcol_2d(SIMULATION *sim, ENTITY_2D *ent, size_t col) {
  LEDGER_INPUT input;
  input.collider_2d.ent = ent;
  input.collider_2d.col = col;
  SIMULATION *cur_sim = NULL;
  for (int i = 0; i < sim->num_linked_sims; i++) {
    cur_sim = sim->linked_sims[i];
    ledger_delete(&cur_sim->mcol_ledger, input, L_TYPE_COLLIDER_2D);
  }
}

void propagate_rm_ment_2d(SIMULATION *sim, ENTITY_2D *ent) {
  LEDGER_INPUT input;
  input.entity_2d.ent = ent;
  SIMULATION *cur_sim = NULL;
  for (int i = 0; i < sim->num_linked_sims; i++) {
    cur_sim = sim->linked_sims[i];
    ledger_delete(&cur_sim->ment_ledger, input, L_TYPE_ENTITY_2D);
  }
}

// Default is_moving callback
int is_moving_2d(ENTITY_2D *ent, size_t col) {
  return 1;
}

// Default move callback
int integrate_ent_2d(ENTITY_2D *ent, vec2 forces) {
  return 1;
}
