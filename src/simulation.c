#include <simulation.h>

extern vec3 col_point;
extern int enable_gravity;

int init_simulation() {
  if (dynamic_ents != NULL || driving_ents != NULL) {
    fprintf(stderr, "Simulation already initialized."\
                    "Simulation initialization failed\n");
    return -1;
  }

  dynamic_ents = malloc(sizeof(ENTITY *) * BUFF_STARTING_LEN);
  if (dynamic_ents == NULL) {
    fprintf(stderr, "Failed to allocate dynamic entities buffer. "\
                    "Simulation initialization failed\n");
    return -1;
  }

  driving_ents = malloc(sizeof(ENTITY *) * BUFF_STARTING_LEN);
  if (driving_ents == NULL) {
    free(dynamic_ents);
    fprintf(stderr, "Failed to allocate static entities buffer. "\
                    "Simulation initialization failed\n");
    return -1;
  }

  physics_tree = init_tree();
  if (physics_tree == NULL) {
    free(dynamic_ents);
    free(driving_ents);
    fprintf(stderr, "Failed to initialize physics oct-tree."\
                    "Simulation initialization failed\n");
    return -1;
  }
  physics_tree->type = PHYS_TREE;

  combat_tree = init_tree();
  if (combat_tree == NULL) {
    free(dynamic_ents);
    free(driving_ents);
    free_oct_tree(physics_tree);
    fprintf(stderr, "Failed to initialize combat oct-tree."\
                    "Simulation initialization failed\n");
  }
  combat_tree->type = HIT_TREE;

  event_tree = init_tree();
  if (event_tree == NULL) {
    free(dynamic_ents);
    free(driving_ents);
    free_oct_tree(physics_tree);
    free_oct_tree(combat_tree);
    fprintf(stderr, "Failed to initialize event oct-tree."\
                    "Simulation initialization failed\n");
  }
  event_tree->type = EVENT_TREE;

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

  if (delta_time > 0.25) {
    delta_time = 0.01;
  }

  ENTITY *entity = NULL;
  COLLIDER *colliders = NULL;
  vec3 col_vel = GLM_VEC3_ZERO_INIT;
  vec3 col_ang_vel = GLM_VEC3_ZERO_INIT;
  int bone = 0;

  int moving = 0;

  // Update placement of neccesarry driving entities
  for (size_t ent = 0; ent < dr_ent_buff_len; ent++) {
    entity = driving_ents[ent];
    if ((entity->type & T_DYNAMIC) == 0 && (entity->velocity[0] != 0.0 ||
        entity->velocity[1] != 0.0 || entity->velocity[2] != 0.0)) {
      entity->type |= T_DYNAMIC;
      status = add_to_elist(&dynamic_ents, &dy_ent_buff_len, &dy_ent_buff_size,
                            entity);
      if (status) {
        return -1;
      }
    }
  }

  // Collision response for each moving (dynamic) entity
  for (size_t ent = 0; ent < dy_ent_buff_len; ent++) {
    entity = dynamic_ents[ent];
    colliders = entity->model->colliders;
    if (entity->type & T_DRIVING || entity->model->num_bones == 0) {
      moving = 0;
      glm_vec3_copy(entity->velocity, col_vel);
      glm_vec3_copy(entity->ang_velocity, col_ang_vel);
      if (col_vel[0] != 0.0 || col_vel[1] != 0.0 || col_vel[2] != 0.0 ||
          col_ang_vel[0] != 0.0 || col_ang_vel[1] != 0.0 ||
          col_ang_vel[2] != 0.0) {
        moving = 1;
        for (size_t col = 0; col < entity->model->num_colliders; col++) {
          if ((entity->type & T_DRIVING &&
               colliders[col].category == DEFAULT) ||
              ((entity->type & T_DRIVING) == 0 &&
               colliders[col].category == HURT_BOX)) {
            status = oct_tree_delete(physics_tree,
                                     entity->tree_offsets[col][PHYS_TREE]);
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
            break;
          }
        }
      }
    } else {
      moving = 0;
      for (size_t col = 0; col < entity->model->num_colliders; col++) {
        bone = entity->model->collider_bone_links[col];
        if (bone != -1) {
          glm_vec3_copy(entity->np_data[bone].velocity, col_vel);
          glm_vec3_copy(entity->np_data[bone].ang_velocity, col_ang_vel);
          if (colliders[col].category == HURT_BOX &&
              (col_vel[0] != 0.0 || col_vel[1] != 0.0 || col_vel[2] != 0.0 ||
               col_ang_vel[0] != 0.0 || col_ang_vel[1] != 0.0 ||
               col_ang_vel[2] != 0.0)) {
            moving = 1;
            status = oct_tree_delete(physics_tree,
                                     entity->tree_offsets[col][PHYS_TREE]);
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
      }
    }

    if (moving == 0) {
      entity->type &= ~T_DYNAMIC;
      remove_from_elist(dynamic_ents, DYNAMIC, ent, &dy_ent_buff_len);
      ent--;
    }
  }

  return 0;
}

int collision_test(ENTITY *subject, size_t offset) {
  if (enable_gravity) {
    int bone = subject->model->collider_bone_links[offset];
    vec3 delta_d = GLM_VEC3_ZERO_INIT;
    versor delta_rot = GLM_QUAT_IDENTITY_INIT;
    // Update "Broad" physics data if entity is driving or rigid body
    // Update "Narrow" physics data if entity is softbody/ragdoll
    if (subject->model->colliders[offset].category == DEFAULT || bone == -1) {
      // Update linear velocity and position of object
      subject->velocity[1] -= (delta_time * GRAVITY);
      glm_vec3_scale(subject->velocity, delta_time, delta_d);
      glm_vec3_add(delta_d, subject->translation, subject->translation);

      // Update angular velocity and rotation of object
      versor ang_vel = { subject->ang_velocity[0],
                         subject->ang_velocity[1],
                         subject->ang_velocity[2], 0.0 };
      // Applying ang velocity to quaternion:
      // q_new = q_old + 0.5 * ang_vel * q_old
      glm_quat_mul(ang_vel, subject->rotation, delta_rot);
      glm_vec4_scale(delta_rot, delta_time * 0.5, delta_rot);
      glm_quat_add(delta_rot, subject->rotation, subject->rotation);
      glm_quat_normalize(subject->rotation);
    } else {
      subject->np_data[bone].velocity[1] -= (delta_time * GRAVITY);
      glm_vec3_scale(subject->np_data[bone].velocity, delta_time, delta_d);
      glm_translate(subject->bone_mats[bone][LOCATION], delta_d);

      versor ang_vel = { subject->np_data[bone].ang_velocity[0],
                         subject->np_data[bone].ang_velocity[1],
                         subject->np_data[bone].ang_velocity[2], 0.0 };
      versor temp_quat = GLM_QUAT_IDENTITY_INIT;
      glm_mat4_quat(subject->bone_mats[bone][ROTATION], temp_quat);
      glm_quat_mul(ang_vel, temp_quat, delta_rot);
      glm_vec4_scale(delta_rot, delta_time * 0.5, delta_rot);
      glm_quat_add(delta_rot, temp_quat, temp_quat);
      glm_quat_normalize(temp_quat);
      glm_quat_mat4(temp_quat, subject->bone_mats[bone][ROTATION]);

      // Combine rotation, location and scale into final bone matrix
      mat4 model_mat = GLM_MAT4_IDENTITY_INIT;
      get_model_mat(subject, model_mat);

      vec3 temp = GLM_VEC3_ZERO_INIT;
      mat4 from_center = GLM_MAT4_IDENTITY_INIT;
      mat4 to_center = GLM_MAT4_IDENTITY_INIT;
      if (subject->model->colliders[offset].type == SPHERE) {
        glm_vec3_copy(subject->model->colliders[offset].data.center, temp);
      } else {
        glm_vec3_copy(subject->model->colliders[offset].data.center_of_mass,
                      temp);
      }
      glm_translate(to_center, temp);
      glm_vec3_negate(temp);
      glm_translate(from_center, temp);

      glm_mat4_identity(subject->final_b_mats[bone]);
      glm_mat4_mul(from_center, subject->final_b_mats[bone],
                   subject->final_b_mats[bone]);
      glm_mat4_mul(subject->bone_mats[bone][SCALE],
                   subject->final_b_mats[bone], subject->final_b_mats[bone]);
      glm_mat4_mul(subject->bone_mats[bone][ROTATION],
                   subject->final_b_mats[bone], subject->final_b_mats[bone]);
      glm_mat4_mul(to_center, subject->final_b_mats[bone],
                   subject->final_b_mats[bone]);
      glm_mat4_mul(subject->bone_mats[bone][LOCATION],
                   subject->final_b_mats[bone], subject->final_b_mats[bone]);
    }
  }

  mat4 s_bone_to_entity = GLM_MAT4_IDENTITY_INIT;
  mat4 s_entity_to_world = GLM_MAT4_IDENTITY_INIT;
  get_model_mat(subject, s_entity_to_world);
  int bone = subject->model->collider_bone_links[offset];
  if (bone != -1) {
    glm_mat4_ins3(subject->model->bones[bone].coordinate_matrix,
                  s_bone_to_entity);
    glm_mat4_mul(s_entity_to_world, subject->final_b_mats[bone],
                 s_entity_to_world);
  }

  COLLIDER s_col;
  global_collider(s_bone_to_entity, s_entity_to_world,
                  subject->model->colliders + offset, &s_col);
  if (subject->model->colliders[offset].type == SPHERE) {
    s_col.data.radius *= subject->scale[0];
  }

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
  mat4 p_bone_to_entity = GLM_MAT4_IDENTITY_INIT;
  mat4 p_entity_to_world = GLM_MAT4_IDENTITY_INIT;

  for (size_t i = 0; i < col_res.list_len; i++) {
    p_obj = col_res.list[i];
    p_ent = p_obj->entity;

    glm_mat4_identity(p_bone_to_entity);
    get_model_mat(p_ent, p_entity_to_world);
    bone = p_ent->model->collider_bone_links[p_obj->collider_offset];
    if (bone != -1) {
      glm_mat4_ins3(p_ent->model->bones[bone].coordinate_matrix,
                    p_bone_to_entity);
      glm_mat4_mul(p_entity_to_world, p_ent->final_b_mats[bone],
                   p_entity_to_world);
    }

    global_collider(p_bone_to_entity, p_entity_to_world,
                    p_ent->model->colliders + p_obj->collider_offset,
                    &collider);
    if (p_ent->model->colliders[p_obj->collider_offset].type == SPHERE) {
      collider.data.radius *= p_ent->scale[0];
    }

    if ((subject->type & T_DRIVING && p_ent != subject) ||
        ((subject->type & T_DRIVING) == 0 &&
         (p_ent != subject || p_obj->collider_offset != offset))) {
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

        COL_ARGS a;
        a.entity = subject;
        if (s_col.type == POLY) {
          glm_vec3_copy(s_col.data.center_of_mass, a.center_of_mass);
        } else {
          glm_vec3_copy(s_col.data.center, a.center_of_mass);
        }
        a.type = subject->type;
        // Determine appropriate collision args based on if entity is a ragdoll
        int bone = subject->model->collider_bone_links[offset];
        if (s_col.category == DEFAULT || bone == -1) {
          // Initial collision correction
          glm_vec3_sub(subject->translation, p_dir, subject->translation);

          a.velocity = &(subject->velocity);
          a.ang_velocity = &(subject->ang_velocity);
          glm_quat_copy(subject->rotation, a.rotation);
          a.inv_mass = subject->inv_mass;
        } else {
          // Initial collision correction
          vec3 correction = GLM_VEC3_ZERO_INIT;
          glm_vec3_negate_to(p_dir, correction);
          glm_translate(subject->bone_mats[bone][LOCATION], correction);

          a.velocity = &(subject->np_data[bone].velocity);
          a.ang_velocity = &(subject->np_data[bone].ang_velocity);
          glm_mat4_quat(subject->bone_mats[bone][ROTATION], a.rotation);
          a.inv_mass = subject->np_data[bone].inv_mass;
        }
        calc_inertia_tensor(subject, offset, &s_col, a.inv_mass,
                            a.inv_inertia);
        glm_mat4_inv(a.inv_inertia, a.inv_inertia);

        COL_ARGS b;
        b.entity = p_ent;
        if (collider.type == POLY) {
          glm_vec3_copy(collider.data.center_of_mass, b.center_of_mass);
        } else {
          glm_vec3_copy(collider.data.center, b.center_of_mass);
        }
        b.type = p_ent->type;
        // Determine appropriate collision args based on if entity is a ragdoll
        bone = p_ent->model->collider_bone_links[p_obj->collider_offset];
        if (collider.category == DEFAULT || bone == -1) {
          b.velocity = &(p_ent->velocity);
          b.ang_velocity = &(p_ent->ang_velocity);
          glm_quat_copy(p_ent->rotation, b.rotation);
          b.inv_mass = p_ent->inv_mass;
        } else {
          b.velocity = &(p_ent->np_data[bone].velocity);
          b.ang_velocity = &(p_ent->np_data[bone].ang_velocity);
          glm_mat4_quat(p_ent->bone_mats[bone][ROTATION], b.rotation);
          b.inv_mass = p_ent->np_data[bone].inv_mass;
        }
        calc_inertia_tensor(p_ent, p_obj->collider_offset, &collider,
                            b.inv_mass, b.inv_inertia);
        glm_mat4_inv(b.inv_inertia, b.inv_inertia);

        solve_collision(&a, &b, p_dir, p_col);

        // Update placement of recieving object if it becomes dynamic
        vec3 *b_vel = b.velocity;
        vec3 *b_ang_vel = b.ang_velocity;
        if (((*b_vel)[0] != 0.0 || (*b_vel)[1] != 0.0 ||
             (*b_vel)[2] != 0.0 || (*b_ang_vel)[0] != 0.0 ||
             (*b_ang_vel)[1] != 0.0 || (*b_ang_vel)[2] != 0.0)
            && (p_ent->type & T_DYNAMIC) == 0) {
          p_ent->type |= T_DYNAMIC;
          add_to_elist(&dynamic_ents, &dy_ent_buff_len, &dy_ent_buff_size,
                       p_ent);
        }
      }
    }
  }

  free(col_res.list);

  return 0;
}

int add_to_elist(ENTITY ***list, size_t *len, size_t *buff_size,
                 ENTITY *entity) {
  (*list)[*len] = entity;
  (*len)++;
  if (*len == *buff_size) {
    return double_buffer((void **) list, buff_size, sizeof(ENTITY *));
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
  // Add entity to dynamic list if moving
  if (entity->velocity[0] != 0.0 || entity->velocity[1] != 0.0 ||
      entity->velocity[2] != 0.0) {
    entity->type |= T_DYNAMIC;
    entity->list_offsets[DYNAMIC] = dy_ent_buff_len;

    status = add_to_elist(&dynamic_ents, &dy_ent_buff_len, &dy_ent_buff_size,
                          entity);
    if (status) {
      fprintf(stderr, "Unable to reallocate dynamic entity buffer\n");
      end_simulation();
      return -1;
    }
  }

  // Add entity to driving list if driving
  if (entity->type & T_DRIVING) {
    entity->list_offsets[DRIVING] = dr_ent_buff_len;

    status = add_to_elist(&driving_ents, &dr_ent_buff_len, &dr_ent_buff_size,
                          entity);
    if (status) {
      fprintf(stderr, "Unable to reallocate driving entity buffer\n");
      end_simulation();
      return -1;
    }
  }

  // Add colliders to the appropriate oct-trees
  int cur_type = 0;
  int cur_cat = 0;
  for (int i = 0; i < entity->model->num_colliders; i++) {
    cur_type = entity->type;
    cur_cat = entity->model->colliders[i].category;
    if (((cur_type & T_DRIVING) && cur_cat == DEFAULT) ||
       (!(cur_type & T_DRIVING) && cur_cat == HURT_BOX)) {
      status = oct_tree_insert(physics_tree, entity, i);
      if (status) {
        fprintf(stderr, "Unable to insert entity into physics oct-tree\n");
        end_simulation();
        return -1;
      }
    }

    if (cur_cat == HURT_BOX) {
      status = oct_tree_insert(combat_tree, entity, i);
      if (status) {
        fprintf(stderr, "Unable to insert entity into combat oct-tree\n");
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

  entity->list_offsets[DYNAMIC] = INVALID_VAL;
  entity->list_offsets[DRIVING] = INVALID_VAL;

  int cur_type = 0;
  int cur_cat = 0;
  for (int i = 0; i < entity->model->num_colliders; i++) {
    cur_type = entity->type;
    cur_cat = entity->model->colliders[i].category;
    if (((cur_type & T_DRIVING) && cur_cat == DEFAULT) ||
       (!(cur_type & T_DRIVING) && cur_cat == HURT_BOX)) {
      oct_tree_delete(physics_tree, entity->tree_offsets[i][PHYS_TREE]);
    }

    if ((cur_cat == HURT_BOX || cur_cat == HIT_BOX) &&
        entity->tree_offsets[i][HIT_TREE] != INVALID) {
      oct_tree_delete(combat_tree, entity->tree_offsets[i][HIT_TREE]);
    }
  }

  return 0;
}

int disable_h_box(ENTITY *entity, size_t index) {
  if (entity == NULL ||
     (entity->model->colliders[index].category != HURT_BOX &&
      entity->model->colliders[index].category != HIT_BOX)) {
    return -1;
  }

  if (entity->tree_offsets[index][HIT_TREE] != INVALID) {
    oct_tree_delete(combat_tree, entity->tree_offsets[index][HIT_TREE]);
  }

  return 0;
}

int enable_h_box(ENTITY *entity, size_t index) {
  if (entity == NULL ||
     (entity->model->colliders[index].category != HURT_BOX &&
      entity->model->colliders[index].category != HIT_BOX)) {
    return -1;
  }

  int status = 0;
  if (entity->tree_offsets[index][HIT_TREE] == INVALID) {
    status = oct_tree_insert(combat_tree, entity, index);
    if (status) {
      fprintf(stderr, "Unable to insert hurtbox into combat oct-tree\n");
      end_simulation();
      return -1;
    }
  }

  return 0;
}

int disable_hurtboxes(ENTITY *entity) {
  if (entity == NULL) {
    return -1;
  }

  int cur_cat = 0;
  for (int i = 0; i < entity->model->num_colliders; i++) {
    cur_cat = entity->model->colliders[i].category;
    if ((cur_cat == HURT_BOX) &&
        (entity->tree_offsets[i][HIT_TREE] != INVALID)) {
      oct_tree_delete(combat_tree, entity->tree_offsets[i][HIT_TREE]);
    }
  }

  return 0;
}

int enable_hurtboxes(ENTITY *entity) {
  if (entity == NULL) {
    return -1;
  }

  int status = 0;
  int cur_cat = 0;
  for (int i = 0; i < entity->model->num_colliders; i++) {
    cur_cat = entity->model->colliders[i].category;
    if ((cur_cat == HURT_BOX) &&
        (entity->tree_offsets[i][HIT_TREE] == INVALID)) {
      status = oct_tree_insert(combat_tree, entity, i);
      if (status) {
        fprintf(stderr, "Unable to insert hurtbox into combat oct-tree\n");
        end_simulation();
        return -1;
      }
    }
  }

  return 0;
}

void global_collider(mat4 bone_to_entity, mat4 entity_to_world,
                     COLLIDER *source, COLLIDER *dest) {
  dest->type = source->type;
  dest->category = source->category;
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

void end_simulation() {
  free(dynamic_ents);
  free(driving_ents);
  free_oct_tree(physics_tree);

  dynamic_ents = NULL;
  driving_ents = NULL;
  physics_tree = NULL;
}
