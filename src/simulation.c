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
        entity->velocity[2] != 0.0) {
      if (enable_gravity) {
        entity->velocity[1] -= (delta_time * GRAVITY);
        //entity->translation[1] += entity->velocity[1];
        glm_vec3_add(entity->velocity, entity->translation,
                     entity->translation);
      }

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

  // Update all simulated objects
  /*size_t cur_col = 0;
  size_t bone_index = 0;
  vec3 *velocity = NULL;
  for (size_t i = 0; i < physics_tree->data_buff_len; i++) {
    entity = physics_tree->data_buffer[i].entity;
    cur_col = physics_tree->data_buffer[i].collider_offset;
    int obj_driving = (entity->type & T_DRIVING) ||
                      (entity->model->num_bones == 0);
    if (obj_driving) {
      velocity = &(entity->velocity);
      //glm_vec3_add(entity->delta_v, *velocity, *velocity);
      glm_vec3_add(*velocity, entity->translation, entity->translation);
      //glm_vec3_zero(entity->delta_v);
    } else {
      bone_index = entity->model->collider_bone_links[cur_col];
      //velocity = &(entity->np_data[bone_index].velocity);
      //glm_vec3_add(entity->np_data[bone_index].delta_v, *velocity, *velocity);
      glm_translate(entity->bone_mats[bone_index][LOCATION], *velocity);
      //glm_vec3_zero(entity->np_data[bone_index].delta_v);
    }

    if (*velocity[0] < 0.002 && *velocity[0] > -0.002) {
      *velocity[0] = 0.0;
    }
    if (*velocity[1] < 0.002 && *velocity[1] > -0.002) {
      *velocity[1] = 0.0;
    }
    if (*velocity[2] < 0.002 && *velocity[2] > -0.002) {
      *velocity[2] = 0.0;
    }

    if ((*velocity[0] != 0.0 || *velocity[1] != 0.0 || *velocity[2] != 0.0) &&
        (entity->type & T_DYNAMIC) == 0) {
      entity->type |= T_DYNAMIC;
      status = add_to_elist(dynamic_ents, &dy_ent_buff_len, &dy_ent_buff_size,
                            entity);
      if (status) {
        return -1;
      }
    }
  }*/

  return 0;
}

int collision_test(ENTITY *subject, size_t offset) {
  COLLIDER s_col;

  mat4 s_model = GLM_MAT4_IDENTITY_INIT;
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
        glm_vec3_negate(p_dir);

        solve_collision(subject, offset, p_ent, p_obj->collider_offset,
                        p_dir, p_col);
        /*glm_vec3_add(p_dir, subject->translation, subject->translation);
        glm_vec3_add(p_dir, subject->velocity, subject->velocity);

        if ((subject->velocity[0] > 0.0 && subject->velocity[0] < 0.002) ||
            (subject->velocity[0] < 0.0 && subject->velocity[0] > -0.002)) {
          subject->velocity[0] = 0.0;
        }
        if ((subject->velocity[1] > 0.0 && subject->velocity[1] < 0.002) ||
            (subject->velocity[1] < 0.0 && subject->velocity[1] > -0.002)) {
          subject->velocity[1] = 0.0;
        }
        if ((subject->velocity[2] > 0.0 && subject->velocity[2] < 0.002) ||
            (subject->velocity[2] < 0.0 && subject->velocity[2] > -0.002)) {
          subject->velocity[2] = 0.0;
        }*/
      }
    }
  }

  free(col_res.list);

  return 0;
}

void solve_collision(ENTITY *a, size_t a_col, ENTITY *b, size_t b_col,
                     vec3 p_dir, vec3 p_loc) {
  float mass_a = a->mass;
  vec3 va_naut = GLM_VEC3_ZERO_INIT;
  glm_vec3_copy(a->velocity, va_naut);
  glm_vec3_add(p_dir, a->translation, a->translation);

  if (b->type & T_IMMUTABLE) {
    glm_vec3_add(p_dir, va_naut, a->velocity);
    return;
  }

  float mass_b = b->mass;
  vec3 vb_naut = GLM_VEC3_ZERO_INIT;
  glm_vec3_copy(b->velocity, vb_naut);

  // A collision response
  vec3 momentum_a = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(va_naut, mass_a, momentum_a);

  vec3 momentum_b = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(vb_naut, mass_b, momentum_b);

  vec3 v_final = GLM_VEC3_ZERO_INIT;
  glm_vec3_add(momentum_a, momentum_b, v_final);
  glm_vec3_divs(v_final, mass_a + mass_b, a->velocity);
  if (a->velocity[0] < 0.002 && a->velocity[0] > -0.002) {
    a->velocity[0] = 0.0;
  }
  if (a->velocity[1] < 0.002 && a->velocity[1] > -0.002) {
    a->velocity[1] = 0.0;
  }
  if (a->velocity[2] < 0.002 && a->velocity[2] > -0.002) {
    a->velocity[2] = 0.0;
  }
  glm_vec3_add(a->velocity, a->translation, a->translation);

  // B collision response
  vec3 cm_vminv = GLM_VEC3_ZERO_INIT;
  glm_vec3_sub(va_naut, vb_naut, cm_vminv);
  glm_vec3_scale(cm_vminv, 0.5 * mass_a, cm_vminv);

  glm_vec3_add(momentum_a, momentum_b, v_final);
  glm_vec3_add(cm_vminv, v_final, v_final);
  glm_vec3_divs(v_final, mass_a + mass_b, b->velocity);
  if (b->velocity[0] < 0.002 && b->velocity[0] > -0.002) {
    b->velocity[0] = 0.0;
  }
  if (b->velocity[1] < 0.002 && b->velocity[1] > -0.002) {
    b->velocity[1] = 0.0;
  }
  if (b->velocity[2] < 0.002 && b->velocity[2] > -0.002) {
    b->velocity[2] = 0.0;
  }
  glm_vec3_add(b->velocity, b->translation, b->translation);

  if ((b->velocity[0] != 0.0 || b->velocity[1] != 0.0 || b->velocity[2] != 0.0)
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
  } else if (dest->type == SPHERE) {
    dest->data.radius = source->data.radius;
    glm_mat4_mulv3(model_mat, source->data.center, 1.0, dest->data.center);
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
