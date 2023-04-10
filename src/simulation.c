#include <simulation.h>

extern vec3 col_point;
extern int enable_gravity;

int init_simulation() {
  if (dynamic_ents != NULL || driving_ents != NULL) {
    printf("Simulation already initialized."\
           "Simulation initialization failed\n");
    return -1;
  }

  dynamic_ents = malloc(sizeof(ENTITY *) * BUFF_STARTING_LEN);
  if (dynamic_ents == NULL) {
    printf("Failed to allocate dynamic entities buffer. "\
           "Simulation initialization failed\n");
    return -1;
  }

  driving_ents = malloc(sizeof(ENTITY *) * BUFF_STARTING_LEN);
  if (driving_ents == NULL) {
    free(dynamic_ents);
    printf("Failed to allocate static entities buffer. "\
           "Simulation initialization failed\n");
    return -1;
  }

  physics_tree = init_tree();
  if (physics_tree == NULL) {
    free(dynamic_ents);
    free(driving_ents);
    printf("Failed to initialized physics oct-tree."\
           "Simulation initialization failed\n");
    return -1;
  }

  dy_ent_buff_len = 0;
  dy_ent_buff_size = BUFF_STARTING_LEN;
  dr_ent_buff_len = 0;
  dr_ent_buff_size = BUFF_STARTING_LEN;

  return 0;
}

int simulate_frame() {
  int status = 0;

  float current_time = glfwGetTime();
  delta_time = current_time - last_frame;
  last_frame = current_time;

  ENTITY *entity = NULL;
  COLLIDER *colliders = NULL;

  // Update placement of neccesarry driving entities
  for (size_t ent = 0; ent < dr_ent_buff_len; ent++) {
    entity = driving_ents[ent];
    if ((entity->type & T_DYNAMIC) == 0 && (entity->velocity[0] != 0.0 ||
        entity->velocity[1] != 0.0 || entity->velocity[2] != 0.0)) {
      entity->type |= T_DYNAMIC;
      status = add_to_elist(dynamic_ents, &dy_ent_buff_len, &dy_ent_buff_size,
                            entity);
      if (status) {
        return -1;
      }
    }
  }

  // Collision response for each moving (dynamic) entity
  for (size_t ent = 0; ent < dy_ent_buff_len; ent++) {
    entity = dynamic_ents[ent];
    if (entity->velocity[0] != 0.0 || entity->velocity[1] != 0.0 ||
        entity->velocity[2] != 0.0 || entity->ang_velocity[0] != 0.0 ||
        entity->ang_velocity[1] != 0.0 || entity->ang_velocity[2] != 0.0) {

      colliders = entity->model->colliders;
      for (size_t col = 0; col < entity->model->num_colliders; col++) {
        if ((entity->type & T_DRIVING && colliders[col].category == DEFAULT) ||
            (!(entity->type & T_DRIVING) &&
            colliders[col].category == HURT_BOX)) {
          status = oct_tree_delete(physics_tree, entity->tree_offsets[col]);
          if (status) {
            return -1;
          }
          status = oct_tree_insert(physics_tree, entity, col);
          if (status) {
            return -1;
          }

          status = collision_test(entity, col);
          if (status) {
            return -1;
          }
        }
      }
    } else {
      // Entity is actually not moving, so remove it from dynamic list
      entity->type &= ~T_DYNAMIC;
      remove_from_elist(dynamic_ents, DYNAMIC, ent, &dy_ent_buff_len);
      ent--;
    }
  }

  return 0;
}

int collision_test(ENTITY *subject, size_t offset) {
  if (enable_gravity) {
    subject->velocity[1] -= (delta_time * GRAVITY);
    glm_vec3_add(subject->velocity, subject->translation,
                 subject->translation);

    versor delta_rot = GLM_QUAT_IDENTITY_INIT;
    vec3 ang_vel_axis = GLM_VEC3_ZERO_INIT;
    glm_vec3_normalize_to(subject->ang_velocity, ang_vel_axis);
    glm_quatv(delta_rot, 360 * glm_vec3_norm(subject->ang_velocity),
              ang_vel_axis);
    glm_quat_mul(subject->rotation, delta_rot, subject->rotation);

  }

  mat4 s_model = GLM_MAT4_IDENTITY_INIT;
  COLLIDER s_col;
  get_model_mat(subject, s_model);
  global_collider(s_model, subject->model->colliders + offset,
                  &s_col);

  COLLISION_RES col_res = oct_tree_search(physics_tree, &s_col);

  PHYS_OBJ *p_obj = NULL;
  ENTITY *p_ent = NULL;
  COLLIDER collider;

  vec3 simplex[4];
  vec3 p_dir = GLM_VEC3_ZERO_INIT;
  float p_depth = 0.0;
  vec3 p_col = GLM_VEC3_ZERO_INIT;

  int collision = 0;
  int status = 0;
  mat4 p_model = GLM_MAT4_IDENTITY_INIT;

  vec3 *raw_verts = NULL;
  unsigned int num_raw = 0;
  float height = 0.0;
  float width = 0.0;
  float depth = 0.0;
  float denominator = 0.0;
  float i_val = 0.0;

  for (size_t i = 0; i < col_res.list_len; i++) {
    p_obj = col_res.list[i];
    p_ent = p_obj->entity;
    get_model_mat(p_ent, p_model);
    global_collider(p_model,
                    p_obj->entity->model->colliders + p_obj->collider_offset,
                    &collider);

    if (p_ent != subject) {
      collision = collision_check(&s_col, &collider, simplex);
      if (collision) {
        status = epa_response(&s_col, &collider, simplex, p_dir, &p_depth);
        if (status) {
          free(col_res.list);
          return -1;
        }
        glm_vec3_scale_as(p_dir, p_depth, p_dir);

        collision_point(&s_col, &collider, p_dir, p_col);
        glm_vec3_copy(p_col, col_point);

        // MOMENT OF INERTIA CALCULATION (SHOULD BE MOVED TO ACTUAL COLLIDER
        // CREATION IN OBJ_PREPROCESSOR)
        glm_mat4_identity(subject->inv_inertia);
        if (s_col.type == POLY) {
          raw_verts = subject->model->colliders[offset].data.verts;
          num_raw = subject->model->colliders[offset].data.num_used;
          height = raw_verts[max_dot(raw_verts, num_raw, U_DIR)][1] -
                   raw_verts[max_dot(raw_verts, num_raw, D_DIR)][1];
          width = raw_verts[max_dot(raw_verts, num_raw, L_DIR)][0] -
                  raw_verts[max_dot(raw_verts, num_raw, R_DIR)][0];
          depth = raw_verts[max_dot(raw_verts, num_raw, F_DIR)][2] -
                  raw_verts[max_dot(raw_verts, num_raw, B_DIR)][2];
          denominator = 12.0 * subject->inv_mass;
          subject->inv_inertia[0][0] = ((height * height) + (depth * depth)) /
                                        denominator;
          subject->inv_inertia[1][1] = ((width * width) + (depth * depth)) /
                                        denominator;
          subject->inv_inertia[2][2] = ((width * width) + (height * height)) /
                                        denominator;
        } else {
          i_val = (0.4 * s_col.data.radius * s_col.data.radius) /
                   subject->inv_mass;
          glm_mat4_scale(subject->inv_inertia, i_val);
        }

        glm_mat4_identity(p_ent->inv_inertia);
        if (collider.type == POLY) {
          raw_verts = p_ent->model->colliders[p_obj->collider_offset].data.
                             verts;
          num_raw = p_ent->model->colliders[p_obj->collider_offset].data.
                           num_used;
          height = raw_verts[max_dot(raw_verts, num_raw, U_DIR)][1] -
                   raw_verts[max_dot(raw_verts, num_raw, D_DIR)][1];
          width = raw_verts[max_dot(raw_verts, num_raw, L_DIR)][0] -
                  raw_verts[max_dot(raw_verts, num_raw, R_DIR)][0];
          depth = raw_verts[max_dot(raw_verts, num_raw, F_DIR)][2] -
                  raw_verts[max_dot(raw_verts, num_raw, B_DIR)][2];
          denominator = 12.0 * p_ent->inv_mass;
          p_ent->inv_inertia[0][0] = ((height * height) + (depth * depth)) /
                                      denominator;
          p_ent->inv_inertia[1][1] = ((width * width) + (depth * depth)) /
                                      denominator;
          p_ent->inv_inertia[2][2] = ((width * width) + (height * height)) /
                                      denominator;
        } else {
          i_val = (0.4 * collider.data.radius * collider.data.radius) /
                   p_ent->inv_mass;
          glm_mat4_scale(p_ent->inv_inertia, i_val);
        }

        solve_collision(subject, &s_col, p_ent, &collider, p_dir, p_col);
      }
    }
  }

  free(col_res.list);

  return 0;
}

void solve_collision(ENTITY *a, COLLIDER *a_col, ENTITY *b, COLLIDER *b_col,
                     vec3 p_dir, vec3 p_loc) {
  glm_vec3_sub(a->translation, p_dir, a->translation);




  // TEMPORARY
  /*if (b->inv_mass == 0.0) {//if (b->type & T_IMMUTABLE) {
    float diff_x = p_loc[0] - a_col->data.center_of_mass[0];
    float diff_z = p_loc[2] - a_col->data.center_of_mass[2];
    if (a->model->num_colliders == 2 &&
        (diff_x < -0.01 || diff_x > 0.01 ||
         diff_z < -0.01 || diff_z > 0.01)) {
      printf("Col: %f %f %f Com: %f %f %f Rot: %f %f %f %f\n",
             p_loc[0], p_loc[1], p_loc[2],
             a_col->data.center_of_mass[0],
             a_col->data.center_of_mass[1],
             a_col->data.center_of_mass[2],
             a->rotation[0], a->rotation[1], a->rotation[2], a->rotation[3]);
      fflush(stdout);
    }
    glm_vec3_sub(a->velocity, p_dir, a->velocity);
    return;
  }*/




  // MOMENT OF INERTIA CONVERSION AND INVERSION
  if (a->inv_mass != 0.0) {
    mat4 rot_a = GLM_MAT4_IDENTITY_INIT;
    glm_quat_mat4(a->rotation, rot_a);
    mat4 rot_transpose_a = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_transpose_to(rot_a, rot_transpose_a);
    glm_mat4_mul(rot_a, a->inv_inertia, a->inv_inertia);
    glm_mat4_mul(a->inv_inertia, rot_transpose_a, a->inv_inertia);
    glm_mat4_inv(a->inv_inertia, a->inv_inertia);
  }

  if (b->inv_mass != 0.0) {
    mat4 rot_b = GLM_MAT4_IDENTITY_INIT;
    glm_quat_mat4(b->rotation, rot_b);
    mat4 rot_transpose_b = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_transpose_to(rot_b, rot_transpose_b);
    glm_mat4_mul(rot_b, b->inv_inertia, b->inv_inertia);
    glm_mat4_mul(b->inv_inertia, rot_transpose_b, b->inv_inertia);
    glm_mat4_inv(b->inv_inertia, b->inv_inertia);
  }

  vec3 a_rel = GLM_VEC3_ZERO_INIT;
  if (a_col->type == POLY) {
    glm_vec3_sub(p_loc, a_col->data.center_of_mass, a_rel);
  } else {
    glm_vec3_sub(p_loc, a_col->data.center, a_rel);
  }

  vec3 b_rel = GLM_VEC3_ZERO_INIT;
  if (b_col->type == POLY) {
    glm_vec3_sub(p_loc, b_col->data.center_of_mass, b_rel);
  } else {
    glm_vec3_sub(p_loc, b_col->data.center, b_rel);
  }

  vec3 a_velocity = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(a->ang_velocity, a_rel, a_velocity);
  glm_vec3_add(a->velocity, a_velocity, a_velocity);
  vec3 b_velocity = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(b->ang_velocity, b_rel, b_velocity);
  glm_vec3_add(b->velocity, b_velocity, b_velocity);

  vec3 rel_velocity = GLM_VEC3_ZERO_INIT;
  glm_vec3_sub(a_velocity, b_velocity, rel_velocity);

  vec3 col_normal = GLM_VEC3_ZERO_INIT;
  glm_vec3_normalize_to(p_dir, col_normal);

  vec3 a_cross_n = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(a_rel, col_normal, a_cross_n);
  vec3 b_cross_n = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(b_rel, col_normal, b_cross_n);

  vec3 a_ang_comp = GLM_VEC3_ZERO_INIT;
  if (a->inv_mass != 0.0) {
    glm_vec3_cross(a_cross_n, a_rel, a_ang_comp);
    glm_mat4_mulv3(a->inv_inertia, a_ang_comp, 1.0, a_ang_comp);
  }
  vec3 b_ang_comp = GLM_VEC3_ZERO_INIT;
  if (b->inv_mass != 0.0) {
    glm_vec3_cross(b_cross_n, b_rel, b_ang_comp);
    glm_mat4_mulv3(b->inv_inertia, b_ang_comp, 1.0, b_ang_comp);
  }

  vec3 angular_comp = GLM_VEC3_ZERO_INIT;
  glm_vec3_add(a_ang_comp, b_ang_comp, angular_comp);

  float impulse_nume = -1.0 * glm_vec3_dot(rel_velocity, col_normal);
  float impulse_den = a->inv_mass + b->inv_mass +
                      glm_vec3_dot(angular_comp, col_normal);
  float impulse = impulse_nume / impulse_den;

  vec3 delta_va = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(col_normal, impulse * a->inv_mass, delta_va);
  glm_vec3_add(a->velocity, delta_va, a->velocity);
  remove_noise(a->velocity, 0.001);
  glm_vec3_add(a->velocity, a->translation, a->translation);

  if ((a->type & T_DRIVING) == 0) {
    vec3 delta_ang_va = GLM_VEC3_ZERO_INIT;
    glm_mat4_scale(a->inv_inertia, impulse);
    glm_mat4_mulv3(a->inv_inertia, a_cross_n, 1.0, delta_ang_va);
    glm_vec3_add(a->ang_velocity, delta_ang_va, a->ang_velocity);
    remove_noise(a->ang_velocity, 0.001);
    if (glm_vec3_norm(a->ang_velocity) > 0.5){
      glm_vec3_scale_as(a->ang_velocity, 0.5, a->ang_velocity);
    }
    versor delta_rota = GLM_QUAT_IDENTITY_INIT;
    vec3 a_ang_vel_axis = GLM_VEC3_ZERO_INIT;
    glm_vec3_normalize_to(a->ang_velocity, a_ang_vel_axis);
    glm_quatv(delta_rota, 360 * glm_vec3_norm(a->ang_velocity),
              a_ang_vel_axis);
    glm_quat_mul(a->rotation, delta_rota, a->rotation);
  }

  vec3 delta_vb = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(col_normal, impulse * b->inv_mass, delta_vb);
  glm_vec3_sub(b->velocity, delta_vb, b->velocity);
  remove_noise(b->velocity, 0.001);
  glm_vec3_add(b->velocity, b->translation, b->translation);

  if ((b->type & T_DRIVING) == 0) {
    vec3 delta_ang_vb = GLM_VEC3_ZERO_INIT;
    glm_mat4_scale(b->inv_inertia, impulse);
    glm_mat4_mulv3(b->inv_inertia, b_cross_n, 1.0, delta_ang_vb);
    glm_vec3_sub(b->ang_velocity, delta_ang_vb, b->ang_velocity);
    remove_noise(b->ang_velocity, 0.001);
    if (glm_vec3_norm(b->ang_velocity) > 0.5) {
      glm_vec3_scale_as(b->ang_velocity, 0.5, b->ang_velocity);
    }
    versor delta_rotb = GLM_QUAT_IDENTITY_INIT;
    vec3 b_ang_vel_axis = GLM_VEC3_ZERO_INIT;
    glm_vec3_normalize_to(b->ang_velocity, b_ang_vel_axis);
    glm_quatv(delta_rotb, 360 * glm_vec3_norm(b->ang_velocity),
              b_ang_vel_axis);
    glm_quat_mul(b->rotation, delta_rotb, b->rotation);
  }

  if ((b->velocity[0] != 0.0 || b->velocity[1] != 0.0 ||
       b->velocity[2] != 0.0 || b->ang_velocity[0] != 0.0 ||
       b->ang_velocity[1] != 0.0 || b->ang_velocity[2] != 0.0)
      && (b->type & T_DYNAMIC) == 0) {
    b->type |= T_DYNAMIC;
    add_to_elist(dynamic_ents, &dy_ent_buff_len, &dy_ent_buff_size, b);
  }
}

int add_to_elist(ENTITY **list, size_t *len, size_t *buff_size,
                 ENTITY *entity) {
  list[*len] = entity;
  (*len)++;
  if (*len == *buff_size) {
    return double_buffer((void **) &list, buff_size, sizeof(ENTITY *));
  }

  return 0;
}

void remove_from_elist(ENTITY **list, int type, size_t index, size_t *len) {
  (*len)--;
  list[index] = list[*len];
  list[*len]->list_offsets[type] = index;
}

int insert_entity(ENTITY *entity) {
  if (entity == NULL || dynamic_ents == NULL || driving_ents == NULL) {
    return -1;
  }

  int status = 0;
  if (entity->velocity[0] != 0.0 || entity->velocity[1] != 0.0 ||
      entity->velocity[2] != 0.0) {
    entity->type |= T_DYNAMIC;
    entity->list_offsets[DYNAMIC] = dy_ent_buff_len;

    status = add_to_elist(dynamic_ents, &dy_ent_buff_len, &dy_ent_buff_size,
                          entity);
    if (status) {
      printf("Unable to reallocate dynamic entity buffer\n");
      end_simulation();
      return -1;
    }
  }

  if (entity->type & T_DRIVING) {
    entity->list_offsets[DRIVING] = dr_ent_buff_len;

    status = add_to_elist(driving_ents, &dr_ent_buff_len, &dr_ent_buff_size,
                          entity);
    if (status) {
      printf("Unable to reallocate driving entity buffer\n");
      end_simulation();
      return -1;
    }
  }

  for (int i = 0; i < entity->model->num_colliders; i++) {
    if ((entity->type & T_DRIVING &&
        entity->model->colliders[i].category == DEFAULT) ||
        (!(entity->type & T_DRIVING) &&
        entity->model->colliders[i].category == HURT_BOX)) {
      status = oct_tree_insert(physics_tree, entity, i);
      if (status) {
        printf("Unable to insert entity into physics oct-tree\n");
        end_simulation();
        return -1;
      }
    }
  }

  return 0;
}

int remove_entity(ENTITY *entity) {
  if (entity == NULL) {
    return -1;
  }

  size_t offset = 0;
  if (entity->type & T_DYNAMIC) {
    offset = entity->list_offsets[DYNAMIC];
    if (offset >= dy_ent_buff_len) {
      return -1;
    }

    remove_from_elist(dynamic_ents, DYNAMIC, offset, &dy_ent_buff_len);
  }

  if (entity->type & T_DRIVING) {
    offset = entity->list_offsets[DRIVING];
    if (offset >= dr_ent_buff_len) {
      return -1;
    }

    remove_from_elist(driving_ents, DRIVING, offset, &dr_ent_buff_len);
  }

  entity->list_offsets[DYNAMIC] = 0xBAADF00D;
  entity->list_offsets[DRIVING] = 0xBAADF00D;

  for (int i = 0; i < entity->model->num_colliders; i++) {
    if ((entity->type & T_DRIVING &&
        entity->model->colliders[i].category == DEFAULT) ||
        (!(entity->type & T_DRIVING) &&
        entity->model->colliders[i].category == HURT_BOX)) {
      oct_tree_delete(physics_tree, entity->tree_offsets[i]);
    }
  }

  return 0;
}

void global_collider(mat4 model_mat, COLLIDER *source, COLLIDER *dest) {
  dest->type = source->type;
  dest->category = source->category;
  if (dest->type == POLY) {
    dest->data.num_used = source->data.num_used;
    for (int i = 0; i < source->data.num_used; i++) {
      glm_mat4_mulv3(model_mat, source->data.verts[i], 1.0,
                     dest->data.verts[i]);
    }
    glm_mat4_mulv3(model_mat, source->data.center_of_mass, 1.0,
                   dest->data.center_of_mass);
  } else if (dest->type == SPHERE) {
    dest->data.radius = source->data.radius;
    glm_mat4_mulv3(model_mat, source->data.center, 1.0, dest->data.center);
  }
}

void remove_noise(vec3 vec, float threshold) {
  if (vec[0] < threshold && vec[0] > -threshold) {
    vec[0] = 0.0;
  }
  if (vec[1] < threshold && vec[1] > -threshold) {
    vec[1] = 0.0;
  }
  if (vec[2] < threshold && vec[2] > -threshold) {
    vec[2] = 0.0;
  }
}

void end_simulation() {
  free(dynamic_ents);
  free(driving_ents);
  free_oct_tree(physics_tree);

  dynamic_ents = NULL;
  driving_ents = NULL;
  physics_tree = NULL;
}
