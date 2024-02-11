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

  sim->moving_colliders = malloc(sizeof(SIM_COLLIDER) * BUFF_STARTING_LEN);
  if (sim->moving_colliders == NULL) {
    free_oct_tree(sim->oct_tree);
    free(sim);
    fprintf(stderr, "Error: Unable to allocate simulation buffers\n");
    return NULL;
  }
  sim->moving_buf_len = 0;
  sim->moving_buf_size = BUFF_STARTING_LEN;

  sim->driving_colliders = malloc(sizeof(SIM_COLLIDER) * BUFF_STARTING_LEN);
  if (sim->driving_colliders == NULL) {
    free(sim->moving_colliders);
    free_oct_tree(sim->oct_tree);
    free(sim);
    fprintf(stderr, "Error: Unable to allocate simulation buffers\n");
    return NULL;
  }
  sim->driving_buf_len = 0;
  sim->driving_buf_size = BUFF_STARTING_LEN;

  glm_vec3_zero(sim->forces);

  return sim;
}

void free_sim(SIMULATION *sim) {
  if (sim == NULL) {
    return;
  }

  free_oct_tree(sim->oct_tree);
  free(sim->moving_colliders);
  free(sim->driving_colliders);
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
        status = elist_add(&sim->driving_colliders, &sim->driving_buf_len,
                           &sim->driving_buf_size, entity, i);
        if (status) {
          fprintf(stderr, "Error: Unable to reallocate simulation buffer\n");
          return -1;
        }
      }

      // Check if collider is currently moving
      vec3 vel = GLM_VEC3_ZERO_INIT;
      vec3 ang_vel = GLM_VEC3_ZERO_INIT;
      get_collider_velocity(entity, i, vel, ang_vel);
      if (is_moving(vel, ang_vel)) {
        entity->type |= T_DYNAMIC;
        status = elist_add(&sim->moving_colliders, &sim->moving_buf_len,
                           &sim->moving_buf_size, entity, i);
        if (status) {
          fprintf(stderr, "Error: Unable to reallocate simulation buffer\n");
          return -1;
        }
      }

      // Insert collider into oct-tree
      status = oct_tree_insert(sim->oct_tree, entity, i);
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

  if (entity->type & T_DYNAMIC) {
    // TODO perhaps come up with a way to make this O(1)
    for (size_t i = 0; i < sim->moving_buf_len; i++) {
      if (sim->moving_colliders[i].entity == entity) {
        elist_delete(sim->moving_colliders, i, &sim->moving_buf_len);
      }
    }
  }

  if (entity->type & T_DRIVING) {
    // TODO perhaps come up with a way to make this O(1)
    for (size_t i = 0; i < sim->driving_buf_len; i++) {
      if (sim->driving_colliders[i].entity == entity) {
        elist_delete(sim->driving_colliders, i, &sim->driving_buf_len);
      }
    }
  }

  // TODO perhaps filter colliders better
  for (size_t i = 0; i < entity->model->num_colliders; i++) {
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
  for (size_t i = 0; i < sim->moving_buf_len; i++) {
    oct_tree_delete(sim->oct_tree, sim->moving_colliders[i].entity,
                    sim->moving_colliders[i].collider_offset);
  }
}

void update_sim_movement(SIMULATION *sim) {
  for (size_t i = 0; i < sim->moving_buf_len; i++) {
    oct_tree_insert(sim->oct_tree, sim->moving_colliders[i].entity,
                    sim->moving_colliders[i].collider_offset);
  }
}

void integrate_sim(SIMULATION *sim) {
  for (size_t i = 0; i < sim->moving_buf_len; i++) {
    integrate_collider(sim->moving_colliders[i].entity,
                       sim->moving_colliders[i].collider_offset,
                       sim->forces);
  }
}

size_t get_sim_collisions(SIMULATION *sim, COLLISION **dest) {
  COLLISION *collisions = malloc(sizeof(COLLISION) * BUFF_STARTING_LEN);
  size_t buf_len = 0;
  size_t buf_size = BUFF_STARTING_LEN;

  int status = 0;
  ENTITY *cur_ent = NULL;
  size_t collider_offset = 0;
  vec3 vel = GLM_VEC3_ZERO_INIT;
  vec3 ang_vel = GLM_VEC3_ZERO_INIT;

  // Update placement of neccesarry driving entities
  for (size_t i = 0; i < sim->driving_buf_len; i++) {
    cur_ent = sim->driving_colliders[i].entity;
    collider_offset = sim->driving_colliders[i].collider_offset;

    get_collider_velocity(cur_ent, collider_offset, vel, ang_vel);
    if (!(cur_ent->type & T_DYNAMIC) && is_moving(vel, ang_vel)) {
      cur_ent->type |= T_DYNAMIC;
      status = elist_add(&sim->moving_colliders, &sim->moving_buf_len,
                         &sim->moving_buf_size, cur_ent, collider_offset);
      if (status) {
        *dest = NULL;
        return 0;
      }
    }
  }

  // Detect collisions for all moving entities
  for (size_t i = 0; i < sim->moving_buf_len; i++) {
    cur_ent = sim->moving_colliders[i].entity;
    collider_offset = sim->moving_colliders[i].collider_offset;

    get_collider_velocity(cur_ent, collider_offset, vel, ang_vel);
    if (is_moving(vel, ang_vel)) {
      // Check collisions
      status = get_collider_collisions(sim, cur_ent, collider_offset,
                                       &collisions, &buf_len, &buf_size);
      if (status) {
        *dest = NULL;
        return 0;
      }
    } else {
      cur_ent->type &= ~T_DYNAMIC;
      elist_delete(sim->moving_colliders, i, &sim->moving_buf_len);
      i--;
    }
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
    calc_inertia_tensor(col.a_ent, col.a_offset, &col.a_world_col,
                        a_args.inv_mass, a_args.inv_inertia);
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
    calc_inertia_tensor(col.b_ent, col.b_offset, &col.b_world_col,
                        b_args.inv_mass, b_args.inv_inertia);
    glm_mat4_inv(b_args.inv_inertia, b_args.inv_inertia);
  } else {
    glm_mat4_zero(b_args.inv_inertia);
  }

  solve_collision(&a_args, &b_args, col.col_dir, col.col_point);

  // Update placement of recieving object if it becomes dynamic
  /*
  vec3 *b_vel = b_args.velocity;
  vec3 *b_ang_vel = b_args.ang_velocity;
  if (!(col.b_ent->type & T_DYNAMIC) && is_moving(*b_vel, *b_ang_vel)) {
    col.b_ent->type |= T_DYNAMIC;
    elist_add(&sim->moving_colliders, &sim->moving_buf_len,
              &sim->moving_buf_size, col.b_ent, col.b_offset);
  }
  */
}

// Manually refresh the status of the collider in the simulation
void refresh_collider(SIMULATION *sim, ENTITY *ent, size_t offset) {
  vec3 vel = GLM_VEC3_ZERO_INIT;
  vec3 ang_vel = GLM_VEC3_ZERO_INIT;

  size_t moving_buff_index = INVALID_INDEX;
  for (size_t i = 0; i < sim->moving_buf_len; i++) {
    if (sim->moving_colliders[i].entity == ent &&
        sim->moving_colliders[i].collider_offset == offset) {
      moving_buff_index = i;
    }
  }

  get_collider_velocity(ent, offset, vel, ang_vel);
  if (is_moving(vel, ang_vel)) {
    ent->type |= T_DYNAMIC;
    if (moving_buff_index == INVALID_INDEX) {
      elist_add(&sim->moving_colliders, &sim->moving_buf_len,
                &sim->moving_buf_size, ent, offset);
    }
  } else {
    ent->type &= ~T_DYNAMIC;
    if (moving_buff_index != INVALID_INDEX) {
      elist_delete(sim->moving_colliders, moving_buff_index,
                   &sim->moving_buf_len);
    }
  }
}

// ============================== MISC HELPERS ===============================

void global_collider(mat4 bone_to_entity, mat4 entity_to_world,
                     COLLIDER *source, COLLIDER *dest) {
  dest->type = source->type;
  dest->category = source->category;
  dest->children_offset = 0;
  dest->num_children = 0;
  if (dest->type == POLY) {
    dest->data.num_used = source->data.num_used;
    for (int i = 0; i < source->data.num_used; i++) {
      glm_mat4_mulv3(bone_to_entity, source->data.verts[i], 1.0,
                     dest->data.verts[i]);
      glm_mat4_mulv3(entity_to_world, dest->data.verts[i], 1.0,
                     dest->data.verts[i]);
    }
    glm_mat4_mulv3(entity_to_world, source->data.center_of_mass, 1.0,
                   dest->data.center_of_mass);
  } else if (dest->type == SPHERE) {
    dest->data.radius = source->data.radius;
    glm_mat4_mulv3(entity_to_world, source->data.center, 1.0,
                   dest->data.center);
  }
}

void elist_delete(SIM_COLLIDER *list, size_t index, size_t *len) {
  (*len)--;
  list[index] = list[*len];
}

int elist_add(SIM_COLLIDER **list, size_t *len, size_t *buff_size,
              ENTITY *entity, size_t collider_offset) {
  SIM_COLLIDER *new_col = (*list) + *len;
  new_col->entity = entity;
  new_col->collider_offset = collider_offset;
  (*len)++;
  if (*len == *buff_size) {
    return double_buffer((void **) list, buff_size, sizeof(SIM_COLLIDER));
  }

  return 0;
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
    glm_vec3_add(entity->velocity, force_vec, entity->velocity);
    glm_vec3_scale(entity->velocity, DELTA_TIME, delta_d);
    glm_vec3_add(delta_d, entity->translation, entity->translation);

    // Update angular velocity and rotation of object
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
    glm_vec3_add(entity->np_data[bone].velocity, force_vec,
                 entity->np_data[bone].velocity);
    glm_vec3_scale(entity->np_data[offset].velocity, DELTA_TIME, delta_d);
    glm_translate(entity->bone_mats[bone][LOCATION], delta_d);

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

int get_collider_collisions(SIMULATION *sim, ENTITY *subject,
                            size_t collider_offset, COLLISION **col,
                            size_t *col_buf_len, size_t *col_buf_size) {
  // Calculate world space collider of subject
  mat4 s_bone_to_entity = GLM_MAT4_IDENTITY_INIT;
  mat4 s_entity_to_world = GLM_MAT4_IDENTITY_INIT;
  get_model_mat(subject, s_entity_to_world);
  int bone = subject->model->collider_bone_links[collider_offset];
  if (bone != -1) {
    COLLIDER *raw_col = subject->model->colliders + collider_offset;
    glm_mat4_ins3(subject->model->bones[bone].coordinate_matrix,
                  s_bone_to_entity);
    if (raw_col->type == POLY) {
      glm_vec4(raw_col->data.center_of_mass, 1.0, s_bone_to_entity[3]);
    } else {
      glm_vec4(raw_col->data.center, 1.0, s_bone_to_entity[3]);
    }
    glm_mat4_mul(s_entity_to_world, subject->final_b_mats[bone],
                 s_entity_to_world);
  }

  COLLIDER s_world_col;
  memset(&s_world_col, 0, sizeof(COLLIDER));
  global_collider(s_bone_to_entity, s_entity_to_world,
                  subject->model->colliders + collider_offset, &s_world_col);
  if (subject->model->colliders[collider_offset].type == SPHERE) {
    s_world_col.data.radius *= subject->scale[0];
  }

  COLLISION_RES col_res = oct_tree_search(sim->oct_tree, &s_world_col);

  PHYS_OBJ *p_obj = NULL;
  ENTITY *candidate_ent = NULL;
  COLLIDER c_world_col;
  memset(&c_world_col, 0, sizeof(COLLIDER));

  vec3 simplex[4] = { GLM_VEC3_ZERO_INIT, GLM_VEC3_ZERO_INIT,
                      GLM_VEC3_ZERO_INIT, GLM_VEC3_ZERO_INIT };
  vec3 collision_dir = GLM_VEC3_ZERO_INIT;
  float collision_depth = 0.0;

  int collision = 0;
  int status = 0;
  mat4 c_bone_to_entity = GLM_MAT4_IDENTITY_INIT;
  mat4 c_entity_to_world = GLM_MAT4_IDENTITY_INIT;

  for (size_t i = 0; i < col_res.list_len; i++) {
    p_obj = col_res.list[i];
    candidate_ent = p_obj->entity;

    // Calculate world space collider of candidate
    glm_mat4_identity(c_bone_to_entity);
    get_model_mat(candidate_ent, c_entity_to_world);
    bone = candidate_ent->model->collider_bone_links[p_obj->collider_offset];
    if (bone != -1) {
      COLLIDER *raw_col = candidate_ent->model->colliders +
                          p_obj->collider_offset;
      glm_mat4_ins3(candidate_ent->model->bones[bone].coordinate_matrix,
                    c_bone_to_entity);
      if (raw_col->type == POLY) {
        glm_vec4(raw_col->data.center_of_mass, 1.0, c_bone_to_entity[3]);
      } else {
        glm_vec4(raw_col->data.center, 1.0, c_bone_to_entity[3]);
      }
      glm_mat4_mul(c_entity_to_world, candidate_ent->final_b_mats[bone],
                   c_entity_to_world);
    }
    global_collider(c_bone_to_entity, c_entity_to_world,
                    candidate_ent->model->colliders + p_obj->collider_offset,
                    &c_world_col);
    if (candidate_ent->model->colliders[p_obj->collider_offset].type ==
        SPHERE) {
      c_world_col.data.radius *= candidate_ent->scale[0];
    }

    if (candidate_ent != subject ||
        ((subject->type & T_DRIVING) == 0 &&
         p_obj->collider_offset != collider_offset)) {
      collision = collision_check(&s_world_col, &c_world_col, simplex);
      if (collision) {
        status = epa_response(&s_world_col, &c_world_col, simplex,
                              collision_dir, &collision_depth);
        if (status) {
          free(col_res.list);
          return -1;
        }
        vec3_remove_noise(collision_dir, 0.000001);
        glm_vec3_scale_as(collision_dir, collision_depth, collision_dir);

        COLLISION *new_col = (*col) + (*col_buf_len);
        new_col->a_ent = subject;
        new_col->b_ent = candidate_ent;
        new_col->a_offset = collider_offset;
        new_col->b_offset = p_obj->collider_offset;
        new_col->a_world_col = s_world_col;
        new_col->b_world_col = c_world_col;
        glm_vec3_copy(collision_dir, new_col->col_dir);
        collision_point(&s_world_col, &c_world_col, collision_dir,
                        new_col->col_point);

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
      }
    }
  }

  free(col_res.list);

  return 0;
}

void get_collider_velocity(ENTITY *entity, size_t collider_offset,
                           vec3 vel, vec3 ang_vel) {
  COLLIDER *collider = entity->model->colliders + collider_offset;
  int bone = entity->model->collider_bone_links[collider_offset];
  P_DATA *phys_data = entity->np_data;
  if (collider->category == DEFAULT || bone == -1) {
    glm_vec3_copy(entity->velocity, vel);
    glm_vec3_copy(entity->ang_velocity, ang_vel);
  } else {
    glm_vec3_copy(phys_data[collider_offset].velocity, vel);
    glm_vec3_copy(phys_data[collider_offset].ang_velocity, ang_vel);
  }
}

int is_moving(vec3 vel, vec3 ang_vel) {
  return vel[X] || vel[Y] || vel[Z] || ang_vel[X] || ang_vel[Y] || ang_vel[Z];
}

