#include <simulation.h>

extern vec3 col_point;

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
      if (status != 0) {
        return -1;
      }
    }
  }

  // Collision response for each moving (dynamic) entity
  for (size_t ent = 0; ent < dy_ent_buff_len; ent++) {
    entity = dynamic_ents[ent];
    if (entity->velocity[0] != 0.0 || entity->velocity[1] != 0.0 ||
        entity->velocity[2] != 0.0) {
      entity->velocity[1] -= (delta_time * GRAVITY);
      entity->translation[1] += entity->velocity[1];

      colliders = entity->model->colliders;
      for (size_t col = 0; col < entity->model->num_colliders; col++) {
        status = oct_tree_delete(physics_tree, entity->tree_offsets[col]);
        if (status != 0) {
          return -1;
        }
        status = oct_tree_insert(physics_tree, entity, col);
        if (status != 0) {
          return -1;
        }

        if (colliders[col].category == DEFAULT) {
          status = collision_test(entity, col);
          if (status != 0) {
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

int collision_test(ENTITY *target, size_t offset) {
  COLLIDER t_col;

  mat4 t_model = GLM_MAT4_IDENTITY_INIT;
  get_model_mat(target, t_model);

  global_collider(t_model, target->model->colliders + offset,
                  &t_col);

  COLLISION_RES col_res = oct_tree_search(physics_tree, &t_col);

  PHYS_OBJ *p_obj = NULL;
  COLLIDER collider;

  vec3 simplex[4];
  vec3 p_dir = GLM_VEC3_ZERO_INIT;
  float p_depth = 0.0;

  int collision = 0;
  int status = 0;
  mat4 p_model = GLM_MAT4_IDENTITY_INIT;
  for (size_t i = 0; i < col_res.list_len; i++) {
    p_obj = col_res.list[i];
    get_model_mat(p_obj->entity, p_model);
    global_collider(p_model,
                    p_obj->entity->model->colliders + p_obj->collider_offset,
                    &collider);

    if (p_obj->entity != target && collider.category == DEFAULT) {
      collision = collision_check(&t_col, &collider, simplex);
      if (collision) {
        status = epa_response(&t_col, &collider, simplex, p_dir, &p_depth);
        if (status != 0) {
          free(col_res.list);
          return -1;
        }
        glm_vec3_scale_as(p_dir, p_depth, p_dir);
        collision_point(&t_col, &collider, p_dir, col_point);

        glm_vec3_negate(p_dir);
        glm_vec3_add(p_dir, target->translation, target->translation);
        glm_vec3_add(p_dir, target->velocity, target->velocity);

        if ((target->velocity[0] > 0.0 && target->velocity[0] < 0.002) ||
            (target->velocity[0] < 0.0 && target->velocity[0] > -0.002)) {
          target->velocity[0] = 0.0;
        }
        if ((target->velocity[1] > 0.0 && target->velocity[1] < 0.002) ||
            (target->velocity[1] < 0.0 && target->velocity[1] > -0.002)) {
          target->velocity[1] = 0.0;
        }
        if ((target->velocity[2] > 0.0 && target->velocity[2] < 0.002) ||
            (target->velocity[2] < 0.0 && target->velocity[2] > -0.002)) {
          target->velocity[2] = 0.0;
        }
      }
    }
  }

  free(col_res.list);

  return 0;
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
    if (status != 0) {
      printf("Unable to reallocate dynamic entity buffer\n");
      end_simulation();
      return -1;
    }
  }

  if (entity->type & T_DRIVING) {
    entity->list_offsets[DRIVING] = dr_ent_buff_len;

    status = add_to_elist(driving_ents, &dr_ent_buff_len, &dr_ent_buff_size,
                          entity);
    if (status != 0) {
      printf("Unable to reallocate driving entity buffer\n");
      end_simulation();
      return -1;
    }
  }

  for (int i = 0; i < entity->model->num_colliders; i++) {
    status = oct_tree_insert(physics_tree, entity, i);
    if (status != 0) {
      printf("Unable to insert entity into physics oct-tree\n");
      end_simulation();
      return -1;
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
    oct_tree_delete(physics_tree, entity->tree_offsets[i]);
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
