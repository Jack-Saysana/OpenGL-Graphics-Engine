#include <entity.h>

ENTITY *init_entity(MODEL *model) {
  if (model == NULL) {
    return NULL;
  }

  ENTITY *ent = malloc(sizeof(ENTITY));
  if (ent == NULL) {
    return NULL;
  }

  ent->p_cons = malloc(sizeof(J_CONS) * BUFF_STARTING_LEN);
  if (ent->p_cons == NULL) {
    free(ent);
    return NULL;
  }
  ent->num_cons = 0;
  ent->cons_size = BUFF_STARTING_LEN;

  if (model->num_colliders > 0) {
    size_t num_zj = 0;
    for (size_t i = 0; i < model->num_colliders; i++) {
      num_zj += model->colliders[i].num_dofs - 1;
    }
    ent->np_data = malloc(sizeof(P_DATA) * model->num_colliders);
    if (ent->np_data == NULL) {
      free(ent->p_cons);
      free(ent);
      return NULL;
    }
    if (num_zj) {
      ent->zj_data = malloc(sizeof(ZERO_JOINT) * num_zj);
      if (ent->zj_data == NULL) {
        free(ent->np_data);
        free(ent->p_cons);
        free(ent);
        return NULL;
      }
    } else {
      ent->zj_data = NULL;
    }

    size_t cur_zj = 0;
    memset(ent->np_data, 0, sizeof(P_DATA) * model->num_colliders);
    memset(ent->zj_data, 0, sizeof(ZERO_JOINT) * num_zj);
    for (size_t i = 0; i < model->num_colliders; i++) {
      ent->np_data[i].inv_inertia[3][3] = 1.0;
      ent->np_data[i].zero_joint_offset = cur_zj;
      ent->np_data[i].num_z_joints = model->colliders[i].num_dofs - 1;

      int joint_dof = model->colliders[i].num_dofs - 1;
      glm_vec3_copy(model->colliders[i].dofs[joint_dof], ent->np_data[i].dof);
      ent->np_data[i].joint_type = model->colliders[i].dofs[joint_dof][W];

      for (int j = 0; j < joint_dof; j++) {
        glm_vec3_copy(model->colliders[i].dofs[j], ent->zj_data[cur_zj].dof);
        ent->zj_data[cur_zj].joint_type = model->colliders[i].dofs[j][W];
        cur_zj++;
      }
    }
  } else {
    ent->np_data = NULL;
    ent->zj_data = NULL;
  }

  if (model->num_bones > 0) {
    ent->bone_mats = malloc(sizeof(mat4) * 3 * model->num_bones);
    if (ent->bone_mats == NULL) {
      free(ent->zj_data);
      free(ent->np_data);
      free(ent->p_cons);
      free(ent);
      return NULL;
    }

    ent->final_b_mats = malloc(sizeof(mat4) * model->num_bones);
    if (ent->final_b_mats == NULL) {
      free(ent->bone_mats);
      free(ent->zj_data);
      free(ent->np_data);
      free(ent->p_cons);
      free(ent);
      return NULL;
    }

    for (size_t i = 0; i < model->num_bones; i++) {
      glm_mat4_identity(ent->bone_mats[i][0]);
      glm_mat4_identity(ent->bone_mats[i][1]);
      glm_mat4_identity(ent->bone_mats[i][2]);

      glm_mat4_identity(ent->final_b_mats[i]);
    }
  } else {
    ent->bone_mats = NULL;
    ent->final_b_mats = NULL;
  }

  ent->model = model;
  ent->data = NULL;
  ent->type = 0;

  return ent;
}

void draw_entity(unsigned int shader, ENTITY *entity) {
  if (entity == NULL) {
    return;
  }

  glUseProgram(shader);
  char var_name[50];
  for (int i = 0; i < entity->model->num_bones; i++) {
    sprintf(var_name, "bone_mats[%d]", i);
    glUniformMatrix4fv(glGetUniformLocation(shader, var_name), 1, GL_FALSE,
                       (float *) entity->final_b_mats[i]);
  }

  draw_model(shader, entity->model);
}

void draw_skeleton(unsigned int shader, ENTITY *entity) {
  if (entity == NULL || entity->bone_mats == NULL) {
    return;
  }

  glUseProgram(shader);
  char var_name[50];
  for (int i = 0; i < entity->model->num_bones; i++) {
    sprintf(var_name, "bone_mats[%d]", i);
    glUniformMatrix4fv(glGetUniformLocation(shader, var_name), 1, GL_FALSE,
                       (float *) entity->final_b_mats[i]);
  }

  set_vec3("col", (vec3) {1.0, 1.0, 0.0}, shader);
  draw_bones(entity->model);
  draw_axes(shader, entity->model);
}

void draw_bone(unsigned int shader, ENTITY *entity, size_t bone) {
  if (entity == NULL || entity->bone_mats == NULL) {
    return;
  }

  glUseProgram(shader);
  char var_name[50];
#ifdef __linux__
  sprintf(var_name, "bone_mats[%ld]", bone);
#else
  sprintf(var_name, "bone_mats[%lld]", bone);
#endif
  glUniformMatrix4fv(glGetUniformLocation(shader, var_name), 1, GL_FALSE,
                     (float *) entity->final_b_mats[bone]);
  set_vec3("col", (vec3) {1.0, 1.0, 0.0}, shader);
  L_VBO bone_data[2];
  glm_vec3_copy(entity->model->bones[bone].base, bone_data[0].coords);
  glm_vec3_copy(entity->model->bones[bone].head, bone_data[1].coords);
  bone_data[0].id = bone;
  bone_data[1].id = bone;

  draw_lines(bone_data, 1);
}

void draw_bone_axis(unsigned int shader, ENTITY *entity, size_t bone) {
  if (entity == NULL || entity->bone_mats == NULL) {
    return;
  }

  glUseProgram(shader);
  char var_name[50];
#ifdef __linux__
  sprintf(var_name, "bone_mats[%ld]", bone);
#else
  sprintf(var_name, "bone_mats[%lld]", bone);
#endif
  glUniformMatrix4fv(glGetUniformLocation(shader, var_name), 1, GL_FALSE,
                     (float *) entity->final_b_mats[bone]);

  L_VBO axis_data[2];
  for (int i = 0; i < 3; i++) {
    glm_vec3_copy(entity->model->bones[bone].base, axis_data[0].coords);
    axis_data[0].id = i;

    vec3 axis_head = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(entity->model->bones[bone].coordinate_matrix[i], axis_head);
    glm_vec3_scale(axis_head, 0.1, axis_head);
    glm_vec3_add(axis_data[0].coords, axis_head, axis_data[1].coords);
    axis_data[1].id = i;

    vec3 col = GLM_VEC3_ZERO_INIT;
    col[i] = 1.0;
    set_vec3("col", col, shader);

    draw_lines(axis_data, 1);
  }
}

void draw_colliders(unsigned int shader, ENTITY *entity, MODEL *sphere) {
  if (entity == NULL) {
    return;
  }

  for (int i = 0; i < entity->model->num_colliders; i++) {
    draw_collider(shader, entity, i, sphere);
  }
}

void draw_collider(unsigned int shader, ENTITY *entity, size_t col,
                   MODEL *sphere) {

  int bone = entity->model->collider_bone_links[col];
  COL_TYPE type = entity->model->colliders[col].type;

  mat4 bone_to_entity = GLM_MAT4_IDENTITY_INIT;
  glm_mat4_ins3(entity->model->bones[bone].coordinate_matrix,
                bone_to_entity);
  if (type == POLY) {
    glm_vec3_copy(entity->model->colliders[col].data.center_of_mass,
                  bone_to_entity[3]);
  } else {
    glm_vec3_copy(entity->model->colliders[col].data.center,
                  bone_to_entity[3]);
  }

  mat4 bone_to_world = GLM_MAT4_IDENTITY_INIT;
  if (bone != 0) {
    // The collider is paired with a non-default bone. Ensure that it acts as
    // a child of the default bone
    glm_mat4_mul(entity->final_b_mats[0], bone_to_entity, bone_to_world);
  } else {
    // Since the collider is paired to the default bone, do not redundantly
    // apply the default bone's matrix twice
    glm_mat4_copy(bone_to_entity, bone_to_world);
  }
  glm_mat4_mul(entity->final_b_mats[bone], bone_to_world, bone_to_world);

  if (type == POLY) {
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                       (float *) bone_to_world);
    draw_poly(entity->model->colliders[col].data.verts);
  } else if (type == SPHERE) {
    glm_translate(bone_to_world, entity->model->colliders[col].data.center);
    glm_scale_uni(bone_to_world, entity->model->colliders[col].data.radius);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                     (float *) bone_to_world);
    // TODO Kill need to have sphere model: Render sphere and poly primitives
    draw_model(shader, sphere);
  }
}

void set_inv_mass(ENTITY *entity, size_t col, float inv_mass) {
  entity->np_data[col].inv_mass = inv_mass;
  calc_inv_inertia(entity, col, entity->np_data[col].inv_inertia);
}

void free_entity(ENTITY *entity) {
  if (entity == NULL) {
    return;
  }

  free(entity->p_cons);

  if (entity->bone_mats) {
    free(entity->bone_mats);
    free(entity->final_b_mats);
  }
  if (entity->np_data) {
    free(entity->np_data);
  }

  free(entity);
}

