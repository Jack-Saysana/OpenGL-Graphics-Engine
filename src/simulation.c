#include <simulation.h>

extern vec3 col_point;
extern int enable_gravity;



extern ENTITY *player;
extern ENTITY *ragdoll;
extern ENTITY **boxes;
extern ENTITY **rects;



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
    if (entity->velocity[0] != 0.0 || entity->velocity[1] != 0.0 ||
        entity->velocity[2] != 0.0 || entity->ang_velocity[0] != 0.0 ||
        entity->ang_velocity[1] != 0.0 || entity->ang_velocity[2] != 0.0) {

      colliders = entity->model->colliders;
      for (size_t col = 0; col < entity->model->num_colliders; col++) {
        if ((entity->type & T_DRIVING && colliders[col].category == DEFAULT) ||
            (!(entity->type & T_DRIVING) &&
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
    vec3 delta_d = GLM_VEC3_ZERO_INIT;
    versor delta_rot = GLM_QUAT_IDENTITY_INIT;
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
    /*
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
      vec3 from_center = GLM_VEC3_ZERO_INIT;
      vec3 to_center = GLM_VEC3_ZERO_INIT;
      if (s_col.type == SPHERE) {
        glm_vec3_negate_to(s_col.data.center, from_center);
        glm_vec3_copy(s_col.data.center, to_center);
      } else {
        glm_vec3_negate_to(s_col.data.center_of_mass, from_center);
        glm_vec3_copy(s_col.data.center_of_mass, to_center);
      }

      glm_mat4_identity(subject->final_b_mats[bone]);
      glm_translate(subject->final_b_mats[bone], from_center);
      glm_mat4_mul(subject->bone_mats[bone][SCALE],
                   subject->final_b_mats[bone], subject->final_b_mats[bone]);
      glm_mat4_mul(subject->bone_mats[bone][ROTATION],
                   subject->final_b_mats[bone], subject->final_b_mats[bone]);
      glm_translate(subject->final_b_mats[bone], to_center);
      glm_mat4_mul(subject->bone_mats[bone][LOCATION],
                   subject->final_b_mats[bone], subject->final_b_mats[bone]);
    }
    */
  }

  // Need to change to also work for ragdolls (Use bone matricies)
  mat4 s_model = GLM_MAT4_IDENTITY_INIT;
  get_model_mat(subject, s_model);
  COLLIDER s_col;
  global_collider(s_model, subject->model->colliders + offset,
                  &s_col);
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
                    p_ent->model->colliders + p_obj->collider_offset,
                    &collider);
    if (p_ent->model->colliders[p_obj->collider_offset].type == SPHERE) {
      collider.data.radius *= p_ent->scale[0];
    }

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
          height = subject->scale[1] *
                   (raw_verts[max_dot(raw_verts, num_raw, U_DIR)][1] -
                    raw_verts[max_dot(raw_verts, num_raw, D_DIR)][1]);
          width = subject->scale[0] *
                  (raw_verts[max_dot(raw_verts, num_raw, L_DIR)][0] -
                   raw_verts[max_dot(raw_verts, num_raw, R_DIR)][0]);
          depth = subject->scale[2] *
                  (raw_verts[max_dot(raw_verts, num_raw, F_DIR)][2] -
                   raw_verts[max_dot(raw_verts, num_raw, B_DIR)][2]);
          denominator = 12.0 * subject->inv_mass;
          subject->inv_inertia[0][0] = ((height * height) + (depth * depth)) /
                                        denominator;
          subject->inv_inertia[1][1] = ((width * width) + (depth * depth)) /
                                        denominator;
          subject->inv_inertia[2][2] = ((width * width) + (height * height)) /
                                        denominator;
          glm_mat4_inv(subject->inv_inertia, subject->inv_inertia);
        } else {
          i_val = (0.4 * s_col.data.radius * s_col.data.radius) /
                   subject->inv_mass;
          glm_mat4_scale(subject->inv_inertia, i_val);
          glm_mat4_inv(subject->inv_inertia, subject->inv_inertia);
        }
        subject->inv_inertia[3][3] = 1.0;

        glm_mat4_identity(p_ent->inv_inertia);
        if (collider.type == POLY) {
          raw_verts = p_ent->model->colliders[p_obj->collider_offset].data.
                             verts;
          num_raw = p_ent->model->colliders[p_obj->collider_offset].data.
                           num_used;
          height = p_ent->scale[1] *
                   (raw_verts[max_dot(raw_verts, num_raw, U_DIR)][1] -
                    raw_verts[max_dot(raw_verts, num_raw, D_DIR)][1]);
          width = p_ent->scale[0] *
                  (raw_verts[max_dot(raw_verts, num_raw, L_DIR)][0] -
                   raw_verts[max_dot(raw_verts, num_raw, R_DIR)][0]);
          depth = p_ent->scale[2] *
                  (raw_verts[max_dot(raw_verts, num_raw, F_DIR)][2] -
                   raw_verts[max_dot(raw_verts, num_raw, B_DIR)][2]);
          denominator = 12.0 * p_ent->inv_mass;
          p_ent->inv_inertia[0][0] = ((height * height) + (depth * depth)) /
                                      denominator;
          p_ent->inv_inertia[1][1] = ((width * width) + (depth * depth)) /
                                      denominator;
          p_ent->inv_inertia[2][2] = ((width * width) + (height * height)) /
                                      denominator;
          glm_mat4_inv(p_ent->inv_inertia, p_ent->inv_inertia);
        } else {
          i_val = (0.4 * collider.data.radius * collider.data.radius) /
                   p_ent->inv_mass;
          glm_mat4_scale(p_ent->inv_inertia, i_val);
          glm_mat4_inv(p_ent->inv_inertia, p_ent->inv_inertia);
        }
        p_ent->inv_inertia[3][3] = 1.0;

        COL_ARGS a;
        a.entity = subject;
        if (s_col.type == POLY) {
          glm_vec3_copy(s_col.data.center_of_mass, a.center_of_mass);
        } else {
          glm_vec3_copy(s_col.data.center, a.center_of_mass);
        }
        glm_mat4_copy(subject->inv_inertia, a.inv_inertia);
        a.type = subject->type;

        // Determine appropriate collision args based on if entity is a ragdoll
        /*
        int bone = subject->model->collider_bone_links[offset];
        if (s_col.category == DEFAULT || bone == -1) {
        */
          // Initial collision correction
          glm_vec3_sub(subject->translation, p_dir, subject->translation);

          a.velocity = &(subject->velocity);
          a.ang_velocity = &(subject->ang_velocity);
          glm_quat_copy(subject->rotation, a.rotation);
          a.inv_mass = subject->inv_mass;
        /*
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
        */

        COL_ARGS b;
        b.entity = p_ent;
        if (collider.type == POLY) {
          glm_vec3_copy(collider.data.center_of_mass, b.center_of_mass);
        } else {
          glm_vec3_copy(collider.data.center, b.center_of_mass);
        }
        glm_mat4_copy(p_ent->inv_inertia, b.inv_inertia);
        b.type = p_ent->type;
        // Determine appropriate collision args based on if entity is a ragdoll
        /*
        bone = p_ent->model->collider_bone_links[p_obj->collider_offset];
        if (collider.category == DEFAULT || bone == -1) {
        */
          b.velocity = &(p_ent->velocity);
          b.ang_velocity = &(p_ent->ang_velocity);
          glm_quat_copy(p_ent->rotation, b.rotation);
          b.inv_mass = p_ent->inv_mass;
        /*
        } else {
          b.velocity = &(p_ent->np_data[bone].velocity);
          b.ang_velocity = &(p_ent->np_data[bone].ang_velocity);
          glm_mat4_quat(p_ent->bone_mats[bone][ROTATION], b.rotation);
          b.inv_mass = p_ent->np_data[bone].inv_mass;
        }
        */

        //solve_collision(subject, &s_col, p_ent, &collider, p_dir, p_col);
        solve_collision(&a, &b, p_dir, p_col);
        //solve_collision(&a, &b, subject, &s_col, p_ent, &collider, p_dir,
        //                p_col);
      }
    }
  }

  free(col_res.list);

  return 0;
}

//void solve_collision(ENTITY *a, COLLIDER *a_col, ENTITY *b, COLLIDER *b_col,
//                     vec3 p_dir, vec3 p_loc) {
void solve_collision(COL_ARGS *a_args, COL_ARGS *b_args, vec3 p_dir,
                     vec3 p_loc) {
  vec3 a_vel = GLM_VEC3_ZERO_INIT;
  vec3 a_ang_vel = GLM_VEC3_ZERO_INIT;
  glm_vec3_copy(*(a_args->velocity), a_vel);
  glm_vec3_copy(*(a_args->ang_velocity), a_ang_vel);

  vec3 b_vel = GLM_VEC3_ZERO_INIT;
  vec3 b_ang_vel = GLM_VEC3_ZERO_INIT;
  glm_vec3_copy(*(b_args->velocity), b_vel);
  glm_vec3_copy(*(b_args->ang_velocity), b_ang_vel);

  // MOMENT OF INERTIA CONVERSION: I = R * I * R^T
  mat4 a_inv_inertia = GLM_MAT4_ZERO_INIT;
  if (a_args->inv_mass != 0.0) {
    mat4 rot_a = GLM_MAT4_IDENTITY_INIT;
    glm_quat_mat4(a_args->rotation, rot_a);
    mat4 rot_transpose_a = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_transpose_to(rot_a, rot_transpose_a);
    glm_mat4_mul(rot_a, a_args->inv_inertia, a_inv_inertia);
    glm_mat4_mul(a_inv_inertia, rot_transpose_a, a_inv_inertia);
  }

  mat4 b_inv_inertia = GLM_MAT4_ZERO_INIT;
  if (b_args->inv_mass != 0.0) {
    mat4 rot_b = GLM_MAT4_IDENTITY_INIT;
    glm_quat_mat4(b_args->rotation, rot_b);
    mat4 rot_transpose_b = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_transpose_to(rot_b, rot_transpose_b);
    glm_mat4_mul(rot_b, b_args->inv_inertia, b_inv_inertia);
    glm_mat4_mul(b_inv_inertia, rot_transpose_b, b_inv_inertia);
  }

  // RELATIVE POSITION: r_rel = collison_point - center_of_mass
  vec3 a_rel = GLM_VEC3_ZERO_INIT;
  glm_vec3_sub(p_loc, a_args->center_of_mass, a_rel);

  vec3 b_rel = GLM_VEC3_ZERO_INIT;
  glm_vec3_sub(p_loc, b_args->center_of_mass, b_rel);

  // TOTAL VELOCITY: v = v_linear + v_angular x r_rel
  vec3 a_velocity = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(a_ang_vel, a_rel, a_velocity);
  glm_vec3_add(a_vel, a_velocity, a_velocity);

  vec3 b_velocity = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(b_ang_vel, b_rel, b_velocity);
  glm_vec3_add(b_vel, b_velocity, b_velocity);

  vec3 rel_velocity = GLM_VEC3_ZERO_INIT;
  glm_vec3_sub(a_velocity, b_velocity, rel_velocity);

  vec3 col_normal = GLM_VEC3_ZERO_INIT;
  glm_vec3_normalize_to(p_dir, col_normal);

  vec3 a_cross_n = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(a_rel, col_normal, a_cross_n);
  vec3 b_cross_n = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(b_rel, col_normal, b_cross_n);

  // I^-1 * (r x n) x r
  vec3 a_ang_comp = GLM_VEC3_ZERO_INIT;
  if (a_args->inv_mass != 0.0) {
    glm_mat4_mulv3(a_inv_inertia, a_cross_n, 1.0, a_ang_comp);
    glm_vec3_cross(a_ang_comp, a_rel, a_ang_comp);
  }
  vec3 b_ang_comp = GLM_VEC3_ZERO_INIT;
  if (b_args->inv_mass != 0.0) {
    glm_mat4_mulv3(b_inv_inertia, b_cross_n, 1.0, b_ang_comp);
    glm_vec3_cross(b_ang_comp, b_rel, b_ang_comp);
  }

  vec3 angular_comp = GLM_VEC3_ZERO_INIT;
  glm_vec3_add(a_ang_comp, b_ang_comp, angular_comp);

  // -(1 + e)(v_rel * n)
  float impulse_nume = -1.0 * glm_vec3_dot(rel_velocity, col_normal);
  // (m1^-1 + m2^-1 + (I1^-1 * (r1 x n) x r1 + I2^-1 * (r2 x n) x r2) * n
  float impulse_den = a_args->inv_mass + b_args->inv_mass +
                      glm_vec3_dot(angular_comp, col_normal);
  // Total impulse
  float impulse = impulse_nume / impulse_den;

  // CHANGE IN VELOCITY: (impulse / m) * n
  vec3 delta_va = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(col_normal, impulse * a_args->inv_mass, delta_va);

  // CHANGE IN ANG VELOCITY: impulse * I^-1(r x n)
  vec3 delta_ang_va = GLM_VEC3_ZERO_INIT;
  // Check done because rotation only effects non-driving physics objects
  if ((a_args->type & T_DRIVING) == 0) {
    glm_mat4_mulv3(a_inv_inertia, a_cross_n, 1.0, delta_ang_va);
    glm_vec3_scale(delta_ang_va, impulse, delta_ang_va);
  }

  vec3 delta_vb = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(col_normal, impulse * b_args->inv_mass, delta_vb);

  vec3 delta_ang_vb = GLM_VEC3_ZERO_INIT;
  if ((b_args->type & T_DRIVING) == 0) {
    glm_mat4_mulv3(b_inv_inertia, b_cross_n, 1.0, delta_ang_vb);
    glm_vec3_scale(delta_ang_vb, impulse, delta_ang_vb);
  }

  // FRICTION: Very similar to impulse model
//#define FRICTION
#ifdef FRICTION
  // TANGENT VECTOR: t = norm(v_rel - (r_rel * n)n)
  float v_dot_n = glm_vec3_dot(rel_velocity, col_normal);
  vec3 col_tan = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(col_normal, v_dot_n, col_tan);
  glm_vec3_sub(rel_velocity, col_tan, col_tan);
  if (glm_vec3_norm(col_tan) != 0.0) {
    glm_vec3_normalize(col_tan);
  }

  // I^-1 * (r x t) x r
  vec3 a_cross_t = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(a_rel, col_tan, a_cross_t);
  vec3 b_cross_t = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(b_rel, col_tan, b_cross_t);
  if (a->inv_mass != 0.0) {
    glm_mat4_mulv3(a_inv_inertia, a_cross_t, 1.0, a_ang_comp);
    glm_vec3_cross(a_ang_comp, a_rel, a_ang_comp);
  }
  if (b->inv_mass != 0.0) {
    glm_mat4_mulv3(b_inv_inertia, b_cross_t, 1.0, b_ang_comp);
    glm_vec3_cross(b_ang_comp, b_rel, b_ang_comp);
  }
  glm_vec3_add(a_ang_comp, b_ang_comp, angular_comp);

  // -(v_rel * t)
  float f_nume = -1.0 * glm_vec3_dot(rel_velocity, col_tan);
  // m1^-1 + m2^-1 + (I1^-1 * (r1 x t) x r1 + I2^-1 * (r2 x t) x r2) * t
  float f_den = a_args->inv_mass + b_args->inv_mass +
                glm_vec3_dot(angular_comp, col_tan);
  float f = f_nume / f_den;

  // IMPULSE DUE TO FRICTION
  float static_f = 0.5;
  float dynamic_f = 0.2;
  float impulse_f = 0.0;
  if (abs(f) < static_f * impulse) {
    impulse_f = f;
  } else {
    impulse_f = -impulse * dynamic_f;
  }

  // CHANGE IN VELOCITY: (impulse / m)t
  vec3 delta_va_f = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(col_tan, impulse_f * a_args->inv_mass, delta_va_f);
  glm_vec3_sub(delta_va, delta_va_f, delta_va);
#endif

  // Dampen and update velocity
  glm_vec3_scale(a_vel, L_DAMP_FACTOR, a_vel);
  glm_vec3_add(a_vel, delta_va, a_vel);
  vec3_remove_noise(a_vel, 0.0001);
  glm_vec3_copy(a_vel, *(a_args->velocity));

#ifdef FRICTION
  // CHANGE IN ANGULAR VELOCITY: impulse * I^-1(r x t)
  if ((a_args->type & T_DRIVING) == 0) {
    vec3 delta_ang_va_f = GLM_VEC3_ZERO_INIT;
    glm_mat4_mulv3(a_inv_inertia, a_cross_t, 1.0, delta_ang_va_f);
    glm_vec3_scale(delta_ang_va_f, impulse_f, delta_ang_va_f);
    glm_vec3_sub(delta_ang_va, delta_ang_va_f, delta_ang_va);
  }
#endif
  // Dampen and update ang velocity
  glm_vec3_scale(a_ang_vel, A_DAMP_FACTOR, a_ang_vel);
  glm_vec3_add(a_ang_vel, delta_ang_va, a_ang_vel);
  vec3_remove_noise(a_ang_vel, 0.0001);
  glm_vec3_copy(a_ang_vel, *(a_args->ang_velocity));

#ifdef FRICTION
  vec3 delta_vb_f = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(col_tan, impulse_f * b_args->inv_mass, delta_vb_f);
  glm_vec3_sub(delta_vb, delta_vb_f, delta_vb);
#endif

  // Dampen and update velocity
  glm_vec3_scale(b_vel, L_DAMP_FACTOR, b_vel);
  glm_vec3_sub(b_vel, delta_vb, b_vel);
  vec3_remove_noise(b_vel, 0.0001);
  glm_vec3_copy(b_vel, *(b_args->velocity));

#ifdef FRICTION
  if ((b_args->type & T_DRIVING) == 0) {
    vec3 delta_ang_vb_f = GLM_VEC3_ZERO_INIT;
    glm_mat4_mulv3(b_inv_inertia, b_cross_t, 1.0, delta_ang_vb_f);
    glm_vec3_scale(delta_ang_vb_f, impulse_f, delta_ang_vb_f);
    glm_vec3_sub(delta_ang_vb, delta_ang_vb_f, delta_ang_vb);
  }
#endif
  // Dampen and update ang velocity
  glm_vec3_scale(b_ang_vel, A_DAMP_FACTOR, b_ang_vel);
  glm_vec3_sub(b_ang_vel, delta_ang_vb, b_ang_vel);
  vec3_remove_noise(b_ang_vel, 0.0001);
  glm_vec3_copy(b_ang_vel, *(b_args->ang_velocity));

  if ((b_vel[0] != 0.0 || b_vel[1] != 0.0 ||
       b_vel[2] != 0.0 || b_ang_vel[0] != 0.0 ||
       b_ang_vel[1] != 0.0 || b_ang_vel[2] != 0.0)
      && (b_args->type & T_DYNAMIC) == 0) {
    b_args->entity->type |= T_DYNAMIC;
    add_to_elist(&dynamic_ents, &dy_ent_buff_len, &dy_ent_buff_size,
                 b_args->entity);
  }
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

void vec3_remove_noise(vec3 vec, float threshold) {
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

void vec4_remove_noise(vec4 vec, float threshold) {
  if (vec[0] < threshold && vec[0] > -threshold) {
    vec[0] = 0.0;
  }
  if (vec[1] < threshold && vec[1] > -threshold) {
    vec[1] = 0.0;
  }
  if (vec[2] < threshold && vec[2] > -threshold) {
    vec[2] = 0.0;
  }
  if (vec[3] < threshold && vec[3] > -threshold) {
    vec[3] = 0.0;
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
