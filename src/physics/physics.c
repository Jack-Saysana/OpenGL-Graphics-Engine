#include <physics/physics.h>

// Perform actual physical calculations upon collision
void impulse_collision(COL_ARGS *a_args, COL_ARGS *b_args, vec3 p_dir,
                       vec3 p_loc, vec3 gravity) {
  if (a_args->inv_mass == 0.0 && b_args->inv_mass == 0.0) {
    // Halt all movement if both colliders are of infinite mass
    glm_vec3_zero(a_args->vel_dest);
    glm_vec3_zero(b_args->vel_dest);
    return;
  }

  vec3 a_vel = GLM_VEC3_ZERO_INIT;
  vec3 a_ang_vel = GLM_VEC3_ZERO_INIT;
  glm_vec3_copy(a_args->velocity, a_vel);
  glm_vec3_copy(a_args->ang_velocity, a_ang_vel);

  vec3 b_vel = GLM_VEC3_ZERO_INIT;
  vec3 b_ang_vel = GLM_VEC3_ZERO_INIT;
  glm_vec3_copy(b_args->velocity, b_vel);
  glm_vec3_copy(b_args->ang_velocity, b_ang_vel);

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
  glm_vec3_sub(p_loc, a_args->center_of_rotation, a_rel);

  vec3 b_rel = GLM_VEC3_ZERO_INIT;
  glm_vec3_sub(p_loc, b_args->center_of_rotation, b_rel);

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
#ifdef FRICTION
  // Maximum magnitude of friction force
  float max_friction = abs(impulse / DELTA_TIME);
  // Velocity component perpendicular to collision normal
  vec3 perp_a_vel = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(col_normal, glm_vec3_dot(col_normal, a_velocity), perp_a_vel);
  glm_vec3_sub(a_velocity, perp_a_vel, perp_a_vel);
  vec3 perp_b_vel = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(col_normal, glm_vec3_dot(col_normal, b_velocity), perp_b_vel);
  glm_vec3_sub(b_velocity, perp_b_vel, perp_b_vel);
  // Force of gravity perpendicular to collision normal
  vec3 perp_grav = GLM_VEC3_ZERO_INIT;
  glm_vec3_scale(col_normal, glm_vec3_dot(col_normal, gravity), perp_grav);
  glm_vec3_sub(gravity, perp_grav, perp_grav);
  // Calculate ideal force of friction to halt movement perpendicular to
  // collision normal
  vec3 a_fric = GLM_VEC3_ZERO_INIT;
  if (a_args->inv_mass == 0.0) {
    glm_vec3_scale(perp_a_vel, -1.0 / DELTA_TIME, a_fric);
  } else {
    glm_vec3_scale(perp_a_vel, -1.0 / (a_args->inv_mass * DELTA_TIME), a_fric);
  }
  glm_vec3_sub(a_fric, perp_grav, a_fric);
  vec3 b_fric = GLM_VEC3_ZERO_INIT;
  if (b_args->inv_mass == 0.0) {
    glm_vec3_scale(perp_b_vel, -1.0 / DELTA_TIME, b_fric);
  } else {
    glm_vec3_scale(perp_b_vel, -1.0 / (b_args->inv_mass * DELTA_TIME), b_fric);
  }
  glm_vec3_sub(b_fric, perp_grav, b_fric);
  // Cap force of friction based on its maximum magnitude
  if (glm_vec3_norm(a_fric) > max_friction) {
    glm_vec3_scale_as(a_fric, max_friction, a_fric);
  }
  if (glm_vec3_norm(b_fric) > max_friction) {
    glm_vec3_scale_as(b_fric, max_friction, b_fric);
  }

  //vec3 delta_va_f = GLM_VEC3_ZERO_INIT;
  //glm_vec3_scale(a_fric, a_args->inv_mass * DELTA_TIME, delta_va_f);
  //glm_vec3_add(delta_va, delta_va_f, delta_va);
#endif

  // Dampen and update velocity
  glm_vec3_scale(a_vel, LINEAR_DAMP_FACTOR, a_vel);
  glm_vec3_add(a_vel, delta_va, a_vel);
  vec3_remove_noise(a_vel, 0.0001);

#ifdef FRICTION
  if ((a_args->type & T_DRIVING) == 0) {
    vec3 delta_ang_va_f = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(a_rel, a_fric, delta_ang_va_f);
    glm_mat4_mulv3(a_inv_inertia, delta_ang_va_f, 1.0, delta_ang_va_f);
    glm_vec3_scale(delta_ang_va_f, DELTA_TIME, delta_ang_va_f);
    glm_vec3_add(delta_ang_va, delta_ang_va_f, delta_ang_va);
  }
#endif

  // Dampen and update ang velocity
  glm_vec3_scale(a_ang_vel, ANGULAR_DAMP_FACTOR, a_ang_vel);
  glm_vec3_add(a_ang_vel, delta_ang_va, a_ang_vel);
  vec3_remove_noise(a_ang_vel, 0.0001);

  //glm_vec3_cross(a_ang_vel, a_rel, a_velocity);
  //glm_vec3_add(a_vel, a_velocity, a_velocity);
  //glm_vec3_copy(a_velocity, a_args->vel_dest);
  glm_vec3_copy(a_vel, a_args->vel_dest);
  glm_vec3_copy(a_ang_vel, a_args->ang_vel_dest);

#ifdef FRICTION
  //vec3 delta_vb_f = GLM_VEC3_ZERO_INIT;
  //glm_vec3_scale(b_fric, b_args->inv_mass * DELTA_TIME, delta_vb_f);
  //glm_vec3_add(delta_vb, delta_vb_f, delta_vb);
#endif

  // Dampen and update velocity
  glm_vec3_scale(b_vel, LINEAR_DAMP_FACTOR, b_vel);
  glm_vec3_sub(b_vel, delta_vb, b_vel);
  vec3_remove_noise(b_vel, 0.0001);

#ifdef FRICTION
  if ((b_args->type & T_DRIVING) == 0) {
    vec3 delta_ang_vb_f = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(b_rel, b_fric, delta_ang_vb_f);
    glm_mat4_mulv3(b_inv_inertia, delta_ang_vb_f, 1.0, delta_ang_vb_f);
    glm_vec3_scale(delta_ang_vb_f, DELTA_TIME, delta_ang_vb_f);
    glm_vec3_add(delta_ang_vb, delta_ang_vb_f, delta_ang_vb);
  }
#endif

  // Dampen and update ang velocity
  glm_vec3_scale(b_ang_vel, ANGULAR_DAMP_FACTOR, b_ang_vel);
  glm_vec3_sub(b_ang_vel, delta_ang_vb, b_ang_vel);
  vec3_remove_noise(b_ang_vel, 0.0001);

  //glm_vec3_cross(b_ang_vel, b_rel, b_velocity);
  //glm_vec3_add(b_vel, b_velocity, b_velocity);
  //glm_vec3_copy(b_velocity, b_args->vel_dest);
  glm_vec3_copy(b_vel, b_args->vel_dest);
  glm_vec3_copy(b_ang_vel, b_args->ang_vel_dest);
}

// The inertia tensor of a sphere and rectangular prism is orthogonal AND
// diagonal, so the inertia tensor and its inverse are the same.
//   **NOTE**: This is not the case for a rotated inertia tensor
void calc_inv_inertia(ENTITY *ent, size_t col, mat4 dest) {
  mat4 scale = GLM_MAT4_IDENTITY_INIT;
  int bone = ent->model->collider_bone_links[col];
  glm_mat4_copy(ent->bone_mats[bone][SCALE], scale);

  COLLIDER *raw_col = ent->model->colliders + col;
  glm_mat4_identity(dest);
  float inv_mass = ent->np_data[col].inv_mass;
  if (inv_mass == 0.0) {
    glm_mat4_zero(dest);
  } else if (raw_col->type == POLY) {
    vec3 *verts = raw_col->data.verts;
    // TODO Num used is always 8
    unsigned int num_raw = raw_col->data.num_used;

    float height = ent->bone_mats[bone][SCALE][1][1] *
                   (verts[max_dot(verts, num_raw, U_DIR)][1] -
                    verts[max_dot(verts, num_raw, D_DIR)][1]);
    float width = ent->bone_mats[bone][SCALE][0][0] *
                  (verts[max_dot(verts, num_raw, L_DIR)][0] -
                   verts[max_dot(verts, num_raw, R_DIR)][0]);
    float depth = ent->bone_mats[bone][SCALE][2][2] *
                  (verts[max_dot(verts, num_raw, F_DIR)][2] -
                   verts[max_dot(verts, num_raw, B_DIR)][2]);
    float denominator = 12.0 * inv_mass;
    dest[0][0] = ((height * height) + (depth * depth)) / denominator;
    dest[1][1] = ((width * width) + (depth * depth)) / denominator;
    dest[2][2] = ((width * width) + (height * height)) / denominator;
  } else {
    float rad = raw_col->data.radius;
    float i_val = (0.4 * rad * rad) / inv_mass;
    glm_mat4_scale(dest, i_val);
  }
  dest[3][3] = 1.0;
}

void rotate_inv_inertia(ENTITY *ent, size_t col, mat4 dest) {
  int bone = ent->model->collider_bone_links[col];
  mat3 bone_to_world = GLM_MAT3_IDENTITY_INIT;
  mat3 inv_inertia = GLM_MAT3_IDENTITY_INIT;
  glm_mat4_pick3(ent->np_data[col].inv_inertia, inv_inertia);
  glm_mat4_pick3(ent->final_b_mats[bone], bone_to_world);
  glm_mat3_mul(bone_to_world, ent->model->bones[bone].coordinate_matrix,
               bone_to_world);
  glm_mat3_mul(bone_to_world, inv_inertia, inv_inertia);
  glm_mat3_transpose(bone_to_world);
  glm_mat3_mul(inv_inertia, bone_to_world, inv_inertia);

  glm_mat4_identity(dest);
  glm_mat4_ins3(inv_inertia, dest);
}

// Default integrate_ent callback
void integrate_ent(ENTITY *ent, vec3 forces) {
  if (ent->num_cons) {
    apply_constraints(ent, ent->p_cons, ent->num_cons, forces);
  }
  featherstone_abm(ent, forces);

  float delta = 0.0;
  for (int cur_bone = 0; cur_bone < ent->model->num_bones; cur_bone++) {
    int cur_col = ent->model->bone_collider_links[cur_bone];
    if (cur_col == -1) {
      continue;
    }

    int collider_root_bone = ent->model->collider_bone_links[cur_col];
    if (collider_root_bone == cur_bone) {
      // Integrate acceleration of zero joints
      for (size_t i = 0; i < ent->np_data[cur_col].num_z_joints; i++) {
        size_t cur_zj = ent->np_data[cur_col].zero_joint_offset + i;
        delta = ent->zj_data[cur_zj].accel_angle * DELTA_TIME;
        ent->zj_data[cur_zj].vel_angle *= 0.999;
        ent->zj_data[cur_zj].vel_angle += delta;
        remove_noise(ent->zj_data[cur_zj].vel_angle, ZERO_THRESHOLD);
      }
      // Integrate acceleration of main joint
      delta = ent->np_data[cur_col].accel_angle * DELTA_TIME;
      ent->np_data[cur_col].vel_angle *= 0.999;
      ent->np_data[cur_col].vel_angle += delta;
      remove_noise(ent->np_data[cur_col].vel_angle, ZERO_THRESHOLD);

      ///*
      //fprintf(stderr, "a: %f, %f, %f\n", ent->np_data[cur_col].a[0],
      //        ent->np_data[cur_col].a[1], ent->np_data[cur_col].a[2]);
      vec3 delta_vec3 = GLM_VEC3_ZERO_INIT;
      glm_vec3_scale(ent->np_data[cur_col].a, DELTA_TIME, delta_vec3);
      //fprintf(stderr, "d_v: %f, %f, %f\n", delta_vec3[0], delta_vec3[1],
      //        delta_vec3[2]);
      //fprintf(stderr, "v before: %f, %f, %f\n", ent->np_data[cur_col].v[0],
      //        ent->np_data[cur_col].v[1], ent->np_data[cur_col].v[2]);
      glm_vec3_scale(ent->np_data[cur_col].v, 0.999,
                     ent->np_data[cur_col].v);
      glm_vec3_add(ent->np_data[cur_col].v, delta_vec3,
                   ent->np_data[cur_col].v);
      vec3_remove_noise(ent->np_data[cur_col].v, ZERO_THRESHOLD);
      //fprintf(stderr, "v after: %f, %f, %f\n", ent->np_data[cur_col].v[0],
      //        ent->np_data[cur_col].v[1], ent->np_data[cur_col].v[2]);

      //fprintf(stderr, "ang_a: %f, %f, %f\n", ent->np_data[cur_col].ang_a[0],
      //        ent->np_data[cur_col].ang_a[1], ent->np_data[cur_col].ang_a[2]);
      glm_vec3_scale(ent->np_data[cur_col].ang_a, DELTA_TIME, delta_vec3);
      //fprintf(stderr, "d_v: %f, %f, %f\n", delta_vec3[0], delta_vec3[1],
      //        delta_vec3[2]);
      //fprintf(stderr, "ang_v before: %f, %f, %f\n",
      //        ent->np_data[cur_col].ang_v[0], ent->np_data[cur_col].ang_v[1],
      //        ent->np_data[cur_col].ang_v[2]);
      glm_vec3_scale(ent->np_data[cur_col].ang_v, 0.999,
                     ent->np_data[cur_col].ang_v);
      glm_vec3_add(ent->np_data[cur_col].ang_v, delta_vec3,
                   ent->np_data[cur_col].ang_v);
      vec3_remove_noise(ent->np_data[cur_col].ang_v, ZERO_THRESHOLD);
      //fprintf(stderr, "ang_v after: %f, %f, %f\n\n",
      //        ent->np_data[cur_col].ang_v[0], ent->np_data[cur_col].ang_v[1],
      //        ent->np_data[cur_col].ang_v[2]);
      //*/

      // Integrate velocity of zero joints
      for (size_t i = 0; i < ent->np_data[cur_col].num_z_joints; i++) {
        size_t cur_zj = ent->np_data[cur_col].zero_joint_offset + i;
        delta = ent->zj_data[cur_zj].vel_angle * DELTA_TIME;
        ent->zj_data[cur_zj].joint_angle += delta;
      }

      // Integrate velocity of main joint
      delta = ent->np_data[cur_col].vel_angle * DELTA_TIME;
      ent->np_data[cur_col].joint_angle += delta;
      glm_vec3_scale(ent->np_data[cur_col].v, DELTA_TIME, delta_vec3);
      glm_translate(ent->bone_mats[cur_bone][LOCATION], delta_vec3);

      vec3 delta_rot = GLM_VEC3_ZERO_INIT;
      glm_vec3_scale(ent->np_data[cur_col].ang_v, DELTA_TIME, delta_rot);
      versor rot_quat = GLM_QUAT_IDENTITY_INIT;
      glm_quatv(rot_quat, glm_vec3_norm(delta_rot), delta_rot);
      glm_quat_normalize(rot_quat);
      versor temp_quat = GLM_QUAT_IDENTITY_INIT;
      glm_mat4_quat(ent->bone_mats[cur_bone][ROTATION], temp_quat);
      glm_quat_normalize(temp_quat);
      glm_quat_mul(rot_quat, temp_quat, temp_quat);
      glm_quat_normalize(temp_quat);
      glm_quat_mat4(temp_quat, ent->bone_mats[cur_bone][ROTATION]);

      // Combine rotation, location and scale into final bone matrix
      vec3 temp = GLM_VEC3_ZERO_INIT;
      mat4 from_center = GLM_MAT4_IDENTITY_INIT;
      mat4 to_center = GLM_MAT4_IDENTITY_INIT;
      if (ent->model->colliders[cur_col].type == SPHERE) {
        glm_vec3_copy(ent->model->colliders[cur_col].data.center, temp);
      } else {
        glm_vec3_copy(ent->model->colliders[cur_col].data.center_of_mass,
                      temp);
      }
      glm_translate(to_center, temp);
      glm_vec3_negate(temp);
      glm_translate(from_center, temp);

      glm_mat4_identity(ent->final_b_mats[cur_bone]);
      glm_mat4_mul(from_center, ent->final_b_mats[cur_bone],
                   ent->final_b_mats[cur_bone]);
      glm_mat4_mul(ent->bone_mats[cur_bone][SCALE],
                   ent->final_b_mats[cur_bone],
                   ent->final_b_mats[cur_bone]);
      glm_mat4_mul(ent->bone_mats[cur_bone][ROTATION],
                   ent->final_b_mats[cur_bone],
                   ent->final_b_mats[cur_bone]);
      glm_mat4_mul(to_center, ent->final_b_mats[cur_bone],
                   ent->final_b_mats[cur_bone]);
      glm_mat4_mul(ent->bone_mats[cur_bone][LOCATION],
                   ent->final_b_mats[cur_bone],
                   ent->final_b_mats[cur_bone]);

      // "Anchor" revolute joints so they are not affected by translational
      // drift
      int parent_bone = ent->model->bones[cur_bone].parent;
      vec3 anchor = GLM_VEC3_ZERO_INIT;
      if (parent_bone != -1 &&
          ent->np_data[cur_col].joint_type != JOINT_PRISMATIC) {
        vec3 base_loc = GLM_VEC3_ZERO_INIT;
        glm_mat4_mulv3(ent->final_b_mats[cur_bone],
                       ent->model->bones[cur_bone].base, 1.0, base_loc);
        vec3 p_head_loc = GLM_VEC3_ZERO_INIT;
        glm_mat4_mulv3(ent->final_b_mats[parent_bone],
                       ent->model->bones[parent_bone].head, 1.0,
                       p_head_loc);
        glm_vec3_sub(p_head_loc, base_loc, anchor);
      }
      mat4 anchor_mat = GLM_MAT4_IDENTITY_INIT;
      glm_translate(anchor_mat, anchor);
      glm_mat4_mul(anchor_mat, ent->final_b_mats[cur_bone],
                   ent->final_b_mats[cur_bone]);
      glm_mat4_mul(anchor_mat, ent->bone_mats[cur_bone][LOCATION],
                   ent->bone_mats[cur_bone][LOCATION]);
    } else {
      glm_mat4_copy(ent->final_b_mats[collider_root_bone],
                    ent->final_b_mats[cur_bone]);
    }
  }

  for (size_t j = 0; j < ent->model->num_colliders; j++) {
    glm_vec3_zero(ent->np_data[j].e_force);
  }
}

// Get the max width of a collider in a direction
float get_width(COLLIDER *col, vec3 dir) {
  if (col->type == POLY) {
    vec3 neg_dir = GLM_VEC3_ZERO_INIT;
    glm_vec3_negate_to(dir, neg_dir);

    vec3 *verts = col->data.verts;
    // TODO Num used is always 8
    vec3 *a = verts + max_dot(verts, col->data.num_used, dir);
    vec3 *b = verts + max_dot(verts, col->data.num_used, neg_dir);
    return glm_vec3_distance(*a, *b);
  } else {
    return col->data.radius * 2.0;
  }
}
