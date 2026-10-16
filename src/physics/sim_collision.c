#include <physics/sim_collision.h>

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
  for (size_t i = 0; i < sim->dcol_ledger.num_items; i++) {
    SIM_ITEM *dcol_map = sim->dcol_ledger.map;
    size_t *dcol_list = sim->dcol_ledger.list;

    cur_ent = dcol_map[dcol_list[i]].col.entity;
    collider_offset = dcol_map[dcol_list[i]].col.collider_offset;
    is_moving_cb = cur_ent->is_moving_cb;

    if (is_moving_cb(cur_ent, collider_offset)) {
      input.collider.ent = cur_ent;
      input.collider.col = collider_offset;
      input.collider.data = NULL;
      status = ledger_add(&sim->mcol_ledger, input, L_TYPE_COLLIDER);
      if (status) {
        *dest = NULL;
        return 0;
      }

      input.entity.ent = cur_ent;
      input.entity.data = NULL;
      status = ledger_add(&sim->ment_ledger, input, L_TYPE_ENTITY);
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

  SIM_ITEM *ment_map = sim->ment_ledger.map;
  size_t *ment_list = sim->ment_ledger.list;
  // Flag all moving entities for deletion. Flags will be flipped back if any
  // collider of an entity is moving
  for (size_t i = 0; i < sim->ment_ledger.num_items; i++) {
    ment_map[ment_list[i]].ent.to_delete = 1;
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
  C_ARGS args;
  args.col_lock = &col_lock;
  args.sim = sim;
  args.start = 0;
  args.end = sim->mcol_ledger.num_items;
  args.collisions = &collisions;
  args.buf_len = &buf_len;
  args.buf_size = &buf_size;
  glm_vec3_copy(origin, args.origin);
  args.range = range;
  args.get_col_info = get_col_info;
  check_moving_buffer(&args);

  SIM_ITEM *mcol_map = sim->mcol_ledger.map;
  size_t *mcol_list = sim->mcol_ledger.list;

  // Clean up moving collider buffer
  for (size_t i = 0; i < sim->mcol_ledger.num_items; i++) {
    if (mcol_map[mcol_list[i]].col.to_delete) {
      ledger_delete_direct(&sim->mcol_ledger, i, L_TYPE_COLLIDER);
      i--;
      propagate_rm_mcol(sim, cur_ent, collider_offset);
    }
  }
  // Clean up moving entity buffer
  for (size_t i = 0; i < sim->ment_ledger.num_items; i++) {
    if (ment_map[ment_list[i]].ent.to_delete) {
      ledger_delete_direct(&sim->ment_ledger, i, L_TYPE_ENTITY);
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
    ledger_add(&sim->mcol_ledger, input, L_TYPE_COLLIDER);
    input.entity.ent = collisions[i].col.b_ent;
    ledger_add(&sim->ment_ledger, input, L_TYPE_ENTITY);

    propagate_new_mcol(sim, collisions[i].col.b_ent,
                       collisions[i].col.b_offset);
  }
  free(collisions);

  return buf_len;
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

  SIM_ITEM *ment_map = sim->ment_ledger.map;
  SIM_ITEM *mcol_map = sim->mcol_ledger.map;
  size_t *mcol_list = sim->mcol_ledger.list;

  LEDGER_INPUT input;
  size_t index = 0;

  for (size_t i = arg_data.start; i < arg_data.end; i++) {
    cur_ent = mcol_map[mcol_list[i]].col.entity;
    collider_offset = mcol_map[mcol_list[i]].col.collider_offset;
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
      index = ledger_search(&sim->ment_ledger, input, L_TYPE_ENTITY);
      if (index != INVALID_INDEX) {
        ment_map[index].ent.to_delete = 0;
      } else {
        fprintf(stderr, "Error: Simulation collider/entity pairity broken\n");
      }
    } else {
      mcol_map[mcol_list[i]].col.to_delete = 1;
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

