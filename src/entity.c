#include <entity.h>

ENTITY *init_entity(MODEL *model) {
  if (model == NULL) {
    return NULL;
  }

  ENTITY *ent = malloc(sizeof(ENTITY));
  if (ent == NULL) {
    return NULL;
  }

  if (model->num_colliders > 0) {
    ent->tree_offsets = malloc(sizeof(size_t) * NUM_OCT_TREES *
                               model->num_colliders);
    if (ent->tree_offsets == NULL) {
      free(ent);
      return NULL;
    }
    for (size_t i = 0; i < model->num_colliders; i++) {
      ent->tree_offsets[i][PHYS_TREE] = 0xBAADF00D;
      ent->tree_offsets[i][HIT_TREE] = 0xBAADF00D;
      ent->tree_offsets[i][EVENT_TREE] = 0xBAADF00D;
    }
  } else {
    ent->tree_offsets = NULL;
  }

  if (model->num_bones > 0) {
    ent->bone_mats = malloc(sizeof(mat4) * 3 * model->num_bones);
    if (ent->bone_mats == NULL) {
      free(ent->tree_offsets);
      free(ent);
      return NULL;
    }

    ent->final_b_mats = malloc(sizeof(mat4) * model->num_bones);
    if (ent->final_b_mats == NULL) {
      free(ent->tree_offsets);
      free(ent->bone_mats);
      free(ent);
      return NULL;
    }

    ent->np_data = calloc(model->num_bones, sizeof(PHYS_DATA));
    if (ent->np_data == NULL) {
      free(ent->final_b_mats);
      free(ent->tree_offsets);
      free(ent->bone_mats);
      free(ent);
      return NULL;
    }
  } else {
    ent->bone_mats = NULL;
    ent->final_b_mats = NULL;
    ent->np_data = NULL;
  }

  ent->model = model;

  glm_quat_identity(ent->rotation);
  glm_vec3_one(ent->scale);
  glm_vec3_zero(ent->translation);

  ent->list_offsets[0] = 0xBAADF00D;
  ent->list_offsets[1] = 0xBAADF00D;
  glm_vec3_zero(ent->velocity);
  glm_vec3_zero(ent->ang_velocity);
  ent->inv_mass = 0.0;
  //ent->mass = 1.0;
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

  mat4 model = GLM_MAT4_IDENTITY_INIT;
  get_model_mat(entity, model);
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1,
                     GL_FALSE, (float *) model);

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

  mat4 model = GLM_MAT4_IDENTITY_INIT;
  get_model_mat(entity, model);
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1,
                     GL_FALSE, (float *) model);

  draw_bones(entity->model);
}

void draw_colliders(unsigned int shader, ENTITY *entity, MODEL *sphere) {
  if (entity == NULL) {
    return;
  }

  glUseProgram(shader);
  mat4 used = GLM_MAT4_IDENTITY_INIT;
  int bone = 0;
  COL_TYPE type = POLY;
  unsigned int *VAO = malloc(sizeof(unsigned int) *
                             entity->model->num_colliders);
  unsigned int *VBO = malloc(sizeof(unsigned int) *
                             entity->model->num_colliders);

  mat4 model = GLM_MAT4_IDENTITY_INIT;
  get_model_mat(entity, model);
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1,
                     GL_FALSE, (float *) model);
  for (int i = 0; i < entity->model->num_colliders; i++) {
    bone = entity->model->collider_bone_links[i];
    if (bone >= 0 && bone < entity->model->num_colliders) {
      glm_mat4_mul(model, entity->final_b_mats[bone], used);
    } else {
      glm_mat4_copy(model, used);
    }
    type = entity->model->colliders[i].type;

    if (type == POLY) {
      glGenVertexArrays(1, VAO + i);
      glBindVertexArray(VAO[i]);
      glGenBuffers(1, VBO + i);
      glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 *
                   entity->model->colliders[i].data.num_used,
                   entity->model->colliders[i].data.verts, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3,
                            (void *) 0);
      glEnableVertexAttribArray(0);

      glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                       (float *) used);

      glDrawArrays(GL_POINTS, 0, entity->model->colliders[i].data.num_used);
      glBindVertexArray(0);
      glDeleteVertexArrays(1, VAO + i);
      glDeleteBuffers(1, VBO + i);
    } else if (type == SPHERE) {
      glm_translate(used, entity->model->colliders[i].data.center);
      glm_scale_uni(used, entity->model->colliders[i].data.radius);
      glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                       (float *) used);
      draw_model(shader, sphere);
    }
  }
  free(VAO);
  free(VBO);
}

void free_entity(ENTITY *entity) {
  if (entity->tree_offsets) {
    free(entity->tree_offsets);
  }
  if (entity->bone_mats) {
    free(entity->bone_mats);
    free(entity->final_b_mats);
    free(entity->np_data);
  }

  free(entity);
}

void get_model_mat(ENTITY *entity, mat4 model) {
  glm_mat4_identity(model);
  glm_translate(model, entity->translation);
  glm_quat_rotate(model, entity->rotation, model);
  glm_scale(model, entity->scale);
}
