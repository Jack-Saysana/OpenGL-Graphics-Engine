#include <simulation.h>

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

  int status = ledger_init(&sim->moving_ledger, &sim->m_list, &sim->num_moving,
                           &sim->m_ledger_size, &sim->m_list_size);
  if (status) {
    free_oct_tree(sim->oct_tree);
    free(sim);
    fprintf(stderr, "Error: Unable to allocate simulation ledgers\n");
    return NULL;
  }

  status = ledger_init(&sim->driving_ledger, &sim->d_list, &sim->num_driving,
                       &sim->d_ledger_size, &sim->d_list_size);
  if (status) {
    free(sim->moving_ledger);
    free(sim->m_list);
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
  free(sim->moving_ledger);
  free(sim->m_list);
  free(sim->driving_ledger);
  free(sim->d_list);
  free(sim);
}

// ============================ SIMULATION HELPERS ===========================

int sim_add_entity(SIMULATION *sim, ENTITY *entity, int collider_filter) {
  if (entity == NULL || sim == NULL) {
    return -1;
  }

  int status = 0;

  // Add desired colliders to the simulation
  COLLIDER *cur_col = NULL;
  for (size_t i = 0; i < entity->model->num_colliders; i++) {
    cur_col = entity->model->colliders + i;
    if (((collider_filter & ALLOW_DEFAULT) &&
         cur_col->category == DEFAULT) ||
        ((collider_filter & ALLOW_HURT_BOXES) &&
         cur_col->category == HURT_BOX) ||
        ((collider_filter & ALLOW_HIT_BOXES) &&
         cur_col->category == HIT_BOX)) {
      // Check if collider is driving
      if (entity->type & T_DRIVING) {
        status = ledger_add(&sim->driving_ledger, &sim->d_list,
                            &sim->num_driving, &sim->d_ledger_size,
                            &sim->d_list_size, entity, i);
        if (status) {
          fprintf(stderr, "Error: Unable to reallocate simulation ledger\n");
          return -1;
        }
      }

      // Check if collider is currently moving
      vec3 vel = GLM_VEC3_ZERO_INIT;
      vec3 ang_vel = GLM_VEC3_ZERO_INIT;
      get_collider_velocity(entity, i, vel, ang_vel);
      if (is_moving(vel, ang_vel)) {
        status = ledger_add(&sim->moving_ledger, &sim->m_list,
                            &sim->num_moving, &sim->m_ledger_size,
                            &sim->m_list_size, entity, i);
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

  // TODO perhaps filter colliders better
  for (size_t i = 0; i < entity->model->num_colliders; i++) {
    ledger_delete(sim->moving_ledger, sim->m_list, sim->m_ledger_size,
                  &sim->num_moving, entity, i);
    ledger_delete(sim->driving_ledger, sim->d_list, sim->d_ledger_size,
                  &sim->num_driving, entity, i);
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
  for (size_t i = 0; i < sim->num_moving; i++) {
#ifdef DEBUG_OCT_TREE
    fprintf(stderr, "Prep before:\n");
    for (size_t j = 0; j < sim->oct_tree->data_buff_len; j++) {
      if (sim->oct_tree->data_buffer[j].entity == sim->moving_ledger[sim->m_list[i]].entity &&
          sim->oct_tree->data_buffer[j].collider_offset == sim->moving_ledger[sim->m_list[i]].collider_offset) {
        fprintf(stderr, "  %p, %ld, %ld, %d, %ld\n", sim->oct_tree->data_buffer[j].entity,
                sim->oct_tree->data_buffer[j].collider_offset,
                sim->oct_tree->data_buffer[j].node_offset,
                sim->oct_tree->data_buffer[j].birthmark,
                j);
      }
    }
#endif
    oct_tree_delete(sim->oct_tree, sim->moving_ledger[sim->m_list[i]].entity,
                    sim->moving_ledger[sim->m_list[i]].collider_offset);
#ifdef DEBUG_OCT_TREE
    int bad = 0;
    fprintf(stderr, "Prep after:\n");
    for (size_t j = 0; j < sim->oct_tree->data_buff_len; j++) {
      if (sim->oct_tree->data_buffer[j].entity == sim->moving_ledger[sim->m_list[i]].entity &&
          sim->oct_tree->data_buffer[j].collider_offset == sim->moving_ledger[sim->m_list[i]].collider_offset) {
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
      global_collider(sim->moving_ledger[sim->m_list[i]].entity,
                      sim->moving_ledger[sim->m_list[i]].collider_offset, &obj);
      fprintf(stderr, "BAD\n");
    }
#endif
  }
}

#ifdef DEBUG_OCT_TREE
void update_sim_movement(SIMULATION *sim, int birthmark) {
  for (size_t i = 0; i < sim->num_moving; i++) {
    oct_tree_insert(sim->oct_tree, sim->moving_ledger[sim->m_list[i]].entity,
                    sim->moving_ledger[sim->m_list[i]].collider_offset,
                    birthmark);
  }
}
#else
void update_sim_movement(SIMULATION *sim) {
  for (size_t i = 0; i < sim->num_moving; i++) {
    oct_tree_insert(sim->oct_tree, sim->moving_ledger[sim->m_list[i]].entity,
                    sim->moving_ledger[sim->m_list[i]].collider_offset);
  }
}
#endif

void integrate_sim(SIMULATION *sim, vec3 origin, float range) {
  ENTITY *cur_ent = NULL;
  size_t collider_offset = 0;

  COLLIDER cur_col;
  memset(&cur_col, 0, sizeof(COLLIDER));

  for (size_t i = 0; i < sim->num_moving; i++) {
    cur_ent = sim->moving_ledger[sim->m_list[i]].entity;
    collider_offset = sim->moving_ledger[sim->m_list[i]].collider_offset;

    // Only consider collider if it is within range
    global_collider(cur_ent, collider_offset, &cur_col);
    if ((range != SIM_RANGE_INF && cur_col.type == POLY &&
        glm_vec3_distance(origin, cur_col.data.center_of_mass) > range) ||
        (range != SIM_RANGE_INF && cur_col.type == SPHERE &&
        glm_vec3_distance(origin, cur_col.data.center) > range)) {
      continue;
    }

    integrate_collider(cur_ent, collider_offset, sim->forces);
  }
}

void integrate_sim_collider(SIMULATION *sim, ENTITY *ent,
                            size_t collider_offset) {
  if (!(ent->type & T_IMMUTABLE)) {
    integrate_collider(ent, collider_offset, sim->forces);
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

  vec3 vel = GLM_VEC3_ZERO_INIT;
  vec3 ang_vel = GLM_VEC3_ZERO_INIT;

  // Update placement of neccesarry driving entities
  for (size_t i = 0; i < sim->num_driving; i++) {
    cur_ent = sim->driving_ledger[sim->d_list[i]].entity;
    collider_offset = sim->driving_ledger[sim->d_list[i]].collider_offset;

    get_collider_velocity(cur_ent, collider_offset, vel, ang_vel);
    if (is_moving(vel, ang_vel)) {
      status = ledger_add(&sim->moving_ledger, &sim->m_list,
                          &sim->num_moving, &sim->m_ledger_size,
                          &sim->m_list_size, cur_ent, collider_offset);
      if (status) {
        *dest = NULL;
        return 0;
      }
    }
  }

  // Detect collisions for all moving entities
  pthread_t t1;
  C_ARGS t1_args;
  t1_args.col_lock = &col_lock;
  t1_args.sim = sim;
  t1_args.start = 0;
  //t1_args.end = sim->num_moving / 3;
  t1_args.end = sim->num_moving / 2;
  //t1_args.end = sim->num_moving;
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
  //t2_args.end = t2_args.start + (sim->num_moving / 3);
  t2_args.end = sim->num_moving;

  /*
  pthread_t t3;
  C_ARGS t3_args;
  memcpy(&t3_args, &t1_args, sizeof(C_ARGS));
  t3_args.start = t2_args.end;
  t3_args.end = sim->num_moving;
  */

  pthread_create(&t1, NULL, check_moving_buffer, &t1_args);
  pthread_create(&t2, NULL, check_moving_buffer, &t2_args);
  //pthread_create(&t3, NULL, check_moving_buffer, &t3_args);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  //pthread_join(t3, NULL);

  for (size_t i = 0; i < sim->num_moving; i++) {
    if (sim->moving_ledger[sim->m_list[i]].to_delete) {
      ledger_delete_direct(sim->moving_ledger, sim->m_list, &sim->num_moving,
                           i);
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
  glm_quat_identity(ent.rotation);
  glm_vec3_copy(GLM_VEC3_ONE, ent.scale);
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

void impulse_resolution(SIMULATION *sim, COLLISION col) {
  COL_ARGS a_args;
  a_args.entity = col.a_ent;
  if (col.a_world_col.type == POLY) {
    glm_vec3_copy(col.a_world_col.data.center_of_mass, a_args.center_of_mass);
  } else {
    glm_vec3_copy(col.a_world_col.data.center, a_args.center_of_mass);
  }
  a_args.type = col.a_ent->type;

  // Determine appropriate collision args based on if entity is a ragdoll
  int bone = col.a_ent->model->collider_bone_links[col.a_offset];
  if (col.a_world_col.category == DEFAULT || bone == -1) {
    glm_vec3_sub(col.a_ent->translation, col.col_dir, col.a_ent->translation);

    a_args.velocity = &(col.a_ent->velocity);
    a_args.ang_velocity = &(col.a_ent->ang_velocity);
    glm_quat_copy(col.a_ent->rotation, a_args.rotation);
    a_args.inv_mass = col.a_ent->inv_mass;
  } else {
    vec3 correction = GLM_VEC3_ZERO_INIT;
    glm_vec3_negate_to(col.col_dir, correction);
    glm_translate(col.a_ent->bone_mats[bone][LOCATION], correction);

    a_args.velocity = &(col.a_ent->np_data[col.a_offset].velocity);
    a_args.ang_velocity = &(col.a_ent->np_data[col.a_offset].ang_velocity);
    glm_mat4_quat(col.a_ent->bone_mats[bone][ROTATION], a_args.rotation);
    a_args.inv_mass = col.a_ent->np_data[col.a_offset].inv_mass;
  }
  if (a_args.inv_mass) {
    //calc_inertia_tensor(col.a_ent, col.a_offset, &col.a_world_col,
    //                    a_args.inv_mass, a_args.inv_inertia);
    calc_inertia_tensor(col.a_ent, col.a_offset, a_args.inv_mass,
                        a_args.inv_inertia);
    glm_mat4_inv(a_args.inv_inertia, a_args.inv_inertia);
  } else {
    glm_mat4_zero(a_args.inv_inertia);
  }

  COL_ARGS b_args;
  b_args.entity = col.b_ent;
  if (col.b_world_col.type == POLY) {
    glm_vec3_copy(col.b_world_col.data.center_of_mass, b_args.center_of_mass);
  } else {
    glm_vec3_copy(col.b_world_col.data.center, b_args.center_of_mass);
  }
  b_args.type = col.b_ent->type;
  // Determine appropriate collision args based on if entity is a ragdoll
  bone = col.b_ent->model->collider_bone_links[col.b_offset];
  if (col.b_world_col.category == DEFAULT || bone == -1) {
    b_args.velocity = &(col.b_ent->velocity);
    b_args.ang_velocity = &(col.b_ent->ang_velocity);
    glm_quat_copy(col.b_ent->rotation, b_args.rotation);
    b_args.inv_mass = col.b_ent->inv_mass;
  } else {
    b_args.velocity = &(col.b_ent->np_data[col.b_offset].velocity);
    b_args.ang_velocity = &(col.b_ent->np_data[col.b_offset].ang_velocity);
    glm_mat4_quat(col.b_ent->bone_mats[bone][ROTATION], b_args.rotation);
    b_args.inv_mass = col.b_ent->np_data[col.b_offset].inv_mass;
  }
  if (b_args.inv_mass) {
    //calc_inertia_tensor(col.b_ent, col.b_offset, &col.b_world_col,
    //                    b_args.inv_mass, b_args.inv_inertia);
    calc_inertia_tensor(col.b_ent, col.b_offset, b_args.inv_mass,
                        b_args.inv_inertia);
    glm_mat4_inv(b_args.inv_inertia, b_args.inv_inertia);
  } else {
    glm_mat4_zero(b_args.inv_inertia);
  }

  solve_collision(&a_args, &b_args, col.col_dir, col.col_point, sim->forces);
}

// Manually refresh the status of the collider in the simulation
void refresh_collider(SIMULATION *sim, ENTITY *ent, size_t offset) {
  vec3 vel = GLM_VEC3_ZERO_INIT;
  vec3 ang_vel = GLM_VEC3_ZERO_INIT;

  get_collider_velocity(ent, offset, vel, ang_vel);
  if (is_moving(vel, ang_vel)) {
    ledger_add(&sim->moving_ledger, &sim->m_list, &sim->num_moving,
               &sim->m_ledger_size, &sim->m_list_size, ent, offset);
  } else {
    ledger_delete(sim->moving_ledger, sim->m_list, sim->m_ledger_size,
                   &sim->num_moving, ent, offset);
  }
}

// ============================== MISC HELPERS ===============================

void global_collider(ENTITY *ent, size_t collider_offset, COLLIDER *dest) {
  COLLIDER *raw_col = ent->model->colliders + collider_offset;
  mat4 bone_to_entity = GLM_MAT4_IDENTITY_INIT;
  mat4 entity_to_world = GLM_MAT4_IDENTITY_INIT;
  get_model_mat(ent, entity_to_world);
  int bone = ent->model->collider_bone_links[collider_offset];
  if (bone != -1) {
    glm_mat4_ins3(ent->model->bones[bone].coordinate_matrix, bone_to_entity);
    if (raw_col->type == POLY) {
      glm_vec4(raw_col->data.center_of_mass, 1.0, bone_to_entity[3]);
    } else {
      glm_vec4(raw_col->data.center, 1.0, bone_to_entity[3]);
    }
    glm_mat4_mul(entity_to_world, ent->final_b_mats[bone], entity_to_world);
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
    dest->data.radius *= ent->scale[0];
  }
}

void integrate_collider(ENTITY *entity, size_t offset, vec3 force) {
  int bone = entity->model->collider_bone_links[offset];
  vec3 delta_d = GLM_VEC3_ZERO_INIT;
  versor delta_rot = GLM_QUAT_IDENTITY_INIT;

  vec3 force_vec = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(force, DELTA_TIME, force_vec);

  // Update "Broad" physics data if entity is driving or rigid body
  // Update "Narrow" physics data if entity is softbody/ragdoll
  if (entity->model->colliders[offset].category == DEFAULT || bone == -1) {
    // Update linear velocity and position of object
    glm_vec3_scale(entity->velocity, LINEAR_DAMP_FACTOR, entity->velocity);
    glm_vec3_add(entity->velocity, force_vec, entity->velocity);
    glm_vec3_scale(entity->velocity, DELTA_TIME, delta_d);
    glm_vec3_add(delta_d, entity->translation, entity->translation);

    // Update angular velocity and rotation of object
    glm_vec3_scale(entity->ang_velocity, ANGULAR_DAMP_FACTOR,
                   entity->ang_velocity);
    versor ang_vel = { entity->ang_velocity[0],
                       entity->ang_velocity[1],
                       entity->ang_velocity[2], 0.0 };
    // Applying ang velocity to quaternion:
    // q_new = q_old + 0.5 * ang_vel * q_old
    glm_quat_mul(ang_vel, entity->rotation, delta_rot);
    glm_vec4_scale(delta_rot, DELTA_TIME * 0.5, delta_rot);
    glm_quat_add(delta_rot, entity->rotation, entity->rotation);
    glm_quat_normalize(entity->rotation);
  } else {
    glm_vec3_scale(entity->np_data[bone].velocity, LINEAR_DAMP_FACTOR,
                   entity->np_data[bone].velocity);
    glm_vec3_add(entity->np_data[bone].velocity, force_vec,
                 entity->np_data[bone].velocity);
    glm_vec3_scale(entity->np_data[offset].velocity, DELTA_TIME, delta_d);
    glm_translate(entity->bone_mats[bone][LOCATION], delta_d);

    glm_vec3_scale(entity->np_data[offset].ang_velocity, ANGULAR_DAMP_FACTOR,
                   entity->np_data[offset].ang_velocity);
    versor ang_vel = { entity->np_data[offset].ang_velocity[0],
                       entity->np_data[offset].ang_velocity[1],
                       entity->np_data[offset].ang_velocity[2], 0.0 };
    versor temp_quat = GLM_QUAT_IDENTITY_INIT;
    glm_mat4_quat(entity->bone_mats[bone][ROTATION], temp_quat);
    glm_quat_mul(ang_vel, temp_quat, delta_rot);
    glm_vec4_scale(delta_rot, DELTA_TIME * 0.5, delta_rot);
    glm_quat_add(delta_rot, temp_quat, temp_quat);
    glm_quat_normalize(temp_quat);
    glm_quat_mat4(temp_quat, entity->bone_mats[bone][ROTATION]);

    // Combine rotation, location and scale into final bone matrix
    mat4 model_mat = GLM_MAT4_IDENTITY_INIT;
    get_model_mat(entity, model_mat);

    vec3 temp = GLM_VEC3_ZERO_INIT;
    mat4 from_center = GLM_MAT4_IDENTITY_INIT;
    mat4 to_center = GLM_MAT4_IDENTITY_INIT;
    if (entity->model->colliders[offset].type == SPHERE) {
      glm_vec3_copy(entity->model->colliders[offset].data.center, temp);
    } else {
      glm_vec3_copy(entity->model->colliders[offset].data.center_of_mass,
                    temp);
    }
    glm_translate(to_center, temp);
    glm_vec3_negate(temp);
    glm_translate(from_center, temp);

    glm_mat4_identity(entity->final_b_mats[bone]);
    glm_mat4_mul(from_center, entity->final_b_mats[bone],
                 entity->final_b_mats[bone]);
    glm_mat4_mul(entity->bone_mats[bone][SCALE],
                 entity->final_b_mats[bone], entity->final_b_mats[bone]);
    glm_mat4_mul(entity->bone_mats[bone][ROTATION],
                 entity->final_b_mats[bone], entity->final_b_mats[bone]);
    glm_mat4_mul(to_center, entity->final_b_mats[bone],
                 entity->final_b_mats[bone]);
    glm_mat4_mul(entity->bone_mats[bone][LOCATION],
                 entity->final_b_mats[bone], entity->final_b_mats[bone]);
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
  vec3 vel = GLM_VEC3_ZERO_INIT;
  vec3 ang_vel = GLM_VEC3_ZERO_INIT;

  for (size_t i = arg_data.start; i < arg_data.end; i++) {
    cur_ent = sim->moving_ledger[sim->m_list[i]].entity;
    collider_offset = sim->moving_ledger[sim->m_list[i]].collider_offset;

    // Only consider collider if it is within range
    global_collider(cur_ent, collider_offset, &cur_col);
    if ((range != SIM_RANGE_INF && cur_col.type == POLY &&
        glm_vec3_distance(origin, cur_col.data.center_of_mass) > range) ||
        (range != SIM_RANGE_INF && cur_col.type == SPHERE &&
        glm_vec3_distance(origin, cur_col.data.center) > range)) {
      continue;
    }

    get_collider_velocity(cur_ent, collider_offset, vel, ang_vel);
    if (is_moving(vel, ang_vel)) {
      // Check collisions
      status = get_collider_collisions(sim, cur_ent, collider_offset,
                                       collisions, buf_len, buf_size,
                                       get_col_info, arg_data.col_lock);
      if (status) {
        return (void *) -1;
      }
    } else {
      sim->moving_ledger[sim->m_list[i]].to_delete = 1;
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
        new_col->a_world_col = s_world_col;
        new_col->b_world_col = c_world_col;
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

void get_collider_velocity(ENTITY *entity, size_t collider_offset,
                           vec3 vel, vec3 ang_vel) {
  glm_vec3_copy(entity->velocity, vel);
  glm_vec3_copy(entity->ang_velocity, ang_vel);

  COLLIDER *collider = entity->model->colliders + collider_offset;
  int bone = entity->model->collider_bone_links[collider_offset];
  P_DATA *phys_data = entity->np_data;
  if (collider->category != DEFAULT && bone != -1) {
    glm_vec3_add(phys_data[collider_offset].velocity, vel, vel);
    glm_vec3_add(phys_data[collider_offset].ang_velocity, ang_vel, ang_vel);
  }
}

int is_moving(vec3 vel, vec3 ang_vel) {
  return vel[X] || vel[Y] || vel[Z] || ang_vel[X] || ang_vel[Y] || ang_vel[Z];
}

// =========================== BOOK KEEPING HELPERS ==========================

int ledger_init(SIM_COLLIDER **ledger, size_t **l_list, size_t *num_cols,
                size_t *ledger_size, size_t *list_size) {
  *ledger = malloc(sizeof(SIM_COLLIDER) * HASH_MAP_STARTING_LEN);
  if ((*ledger) == NULL) {
    return -1;
  }
  memset(*ledger, 0, sizeof(SIM_COLLIDER) * HASH_MAP_STARTING_LEN);

  *l_list = malloc(sizeof(size_t) * BUFF_STARTING_LEN);
  if((*l_list) == NULL) {
    free(*ledger);
    return -1;
  }

  *num_cols = 0;
  *ledger_size = HASH_MAP_STARTING_LEN;
  *list_size = BUFF_STARTING_LEN;
  return 0;
}

size_t hash_col(ENTITY *ent, size_t col, size_t i, size_t size) {
  double key = ((size_t) ent) + col;
  size_t ret = size * ((key * HASH_MAGIC_NUM) - floor(key * HASH_MAGIC_NUM));
  return (ret + i) % size;
}

int ledger_add(SIM_COLLIDER **ledger, size_t **l_list,
               size_t *num_cols, size_t *ledger_size, size_t *list_size,
               ENTITY *ent, size_t col) {
  size_t index = ledger_search(*ledger, *ledger_size, ent, col);
  if (index != INVALID_INDEX) {
    return 0;
  }

  size_t i = 0;
  while (1) {
    index = hash_col(ent, col, i, *ledger_size);
    if ((*ledger)[index].status != LEDGER_OCCUPIED) {
      (*ledger)[index].entity = ent;
      (*ledger)[index].collider_offset = col;
      (*ledger)[index].index = *num_cols;
      (*ledger)[index].status = LEDGER_OCCUPIED;
      (*ledger)[index].to_delete = 0;
      break;
    }
    i++;
  }

  int status = 0;
  (*l_list)[*num_cols] = index;
  (*num_cols)++;
  if (*num_cols == *list_size) {
    status = double_buffer((void **) l_list, list_size, sizeof(size_t));
    if (status) {
      (*ledger)[index].status = LEDGER_FREE;
      (*num_cols)--;
      return -1;
    }
  }

  double load_factor = ((double) (*num_cols)) / ((double) (*ledger_size));
  if (load_factor > 0.5) {
    status = resize_ledger(ledger, *l_list, ledger_size, *num_cols);
    if (status) {
      (*ledger)[index].status = LEDGER_FREE;
      (*num_cols)--;
      return -1;
    }
  }

  return 0;
}

size_t ledger_search(SIM_COLLIDER *ledger, size_t ledger_size,
                     ENTITY *ent, size_t col) {
  size_t i = 0;
  size_t index = 0;
  while (1) {
    index = hash_col(ent, col, i, ledger_size);
    if (ledger[index].status == LEDGER_FREE) {
      break;
    } else if (ledger[index].status == LEDGER_OCCUPIED &&
               ledger[index].entity == ent &&
               ledger[index].collider_offset == col) {
      return index;
    }
    i++;
  }
  return INVALID_INDEX;
}

void ledger_delete(SIM_COLLIDER *ledger, size_t *l_list, size_t ledger_size,
                   size_t *num_cols, ENTITY *ent, size_t col) {
  size_t index = ledger_search(ledger, ledger_size, ent, col);
  if (index != INVALID_INDEX) {
    (*num_cols)--;
    ledger[index].status = LEDGER_DELETED;
    l_list[ledger[index].index] = l_list[*num_cols];
    ledger[l_list[*num_cols]].index = ledger[index].index;
  }
}

void ledger_delete_direct(SIM_COLLIDER *ledger, size_t *l_list,
                          size_t *num_cols, size_t index) {
  (*num_cols)--;
  ledger[l_list[index]].status = LEDGER_DELETED;
  l_list[index] = l_list[*num_cols];
  ledger[l_list[*num_cols]].index = index;
}

int resize_ledger(SIM_COLLIDER **ledger, size_t *l_list, size_t *ledger_size,
                  size_t num_cols) {
  SIM_COLLIDER *new_ledger = malloc(sizeof(SIM_COLLIDER) * 2 * *ledger_size);
  if (new_ledger == NULL) {
    return -1;
  }
  (*ledger_size) *= 2;
  memset(new_ledger, 0, sizeof(SIM_COLLIDER) * *ledger_size);

  size_t j = 0;
  size_t index = 0;
  ENTITY *cur_ent = NULL;
  size_t cur_col = 0;
  int cur_del = 0;
  for (size_t i = 0; i < num_cols; i++) {
    cur_ent = (*ledger)[l_list[i]].entity;
    cur_col = (*ledger)[l_list[i]].collider_offset;
    cur_del = (*ledger)[l_list[i]].to_delete;
    j = 0;
    while (1) {
      index = hash_col(cur_ent, cur_col, j, *ledger_size);
      if (new_ledger[index].status != LEDGER_OCCUPIED) {
        new_ledger[index].entity = cur_ent;
        new_ledger[index].collider_offset = cur_col;
        new_ledger[index].index = i;
        new_ledger[index].status = LEDGER_OCCUPIED;
        new_ledger[index].to_delete = cur_del;
        break;
      }
      j++;
    }
    l_list[i] = index;
  }

  free(*ledger);
  *ledger = new_ledger;
  return 0;
}

