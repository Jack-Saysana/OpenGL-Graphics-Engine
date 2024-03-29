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
    ent->np_data = malloc(sizeof(P_DATA) * model->num_colliders);
    if (ent->np_data == NULL) {
      free(ent);
      return NULL;
    }

    for (size_t i = 0; i < model->num_colliders; i++) {
      // TEMP
      glm_vec3_zero(ent->np_data[i].velocity);
      glm_vec3_zero(ent->np_data[i].ang_velocity);
      // END TEMP

      mat6_zero(ent->np_data[i].I_hat);
      mat6_zero(ent->np_data[i].I_hat_A);
      mat6_zero(ent->np_data[i].ST_to_parent);
      mat6_zero(ent->np_data[i].ST_from_parent);
      glm_mat4_identity(ent->np_data[i].inv_inertia);
      vec6_zero(ent->np_data[i].s_hat);
      vec6_zero(ent->np_data[i].Z_hat);
      vec6_zero(ent->np_data[i].Z_hat_A);
      vec6_zero(ent->np_data[i].coriolis_vector);
      vec6_zero(ent->np_data[i].a_hat);
      vec6_zero(ent->np_data[i].v_hat);
      vec6_zero(ent->np_data[i].s_inner_I);
      glm_vec3_zero(ent->np_data[i].dof);
      glm_vec3_zero(ent->np_data[i].from_parent_lin);
      glm_vec3_zero(ent->np_data[i].joint_to_com);
      ent->np_data[i].inv_mass = 0.0;
      ent->np_data[i].Q = 0.0;
      ent->np_data[i].s_inner_I_dot_s = 0.0;
      ent->np_data[i].SZI = 0.0;
      ent->np_data[i].accel_angle = 0.0;
      ent->np_data[i].vel_angle = 0.0;
      ent->np_data[i].joint_angle = 0.0;
    }
  } else {
    ent->np_data = NULL;
  }

  if (model->num_bones > 0) {
    ent->bone_mats = malloc(sizeof(mat4) * 3 * model->num_bones);
    if (ent->bone_mats == NULL) {
      free(ent->np_data);
      free(ent);
      return NULL;
    }

    ent->final_b_mats = malloc(sizeof(mat4) * model->num_bones);
    if (ent->final_b_mats == NULL) {
      free(ent->bone_mats);
      free(ent->np_data);
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

  glm_quat_identity(ent->rotation);
  glm_vec3_one(ent->scale);
  glm_vec3_zero(ent->translation);

  glm_vec3_zero(ent->velocity);
  glm_vec3_zero(ent->ang_velocity);
  ent->inv_mass = 0.0;
  ent->type = 0;
  ent->data = NULL;

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

  unsigned int VAO;
  unsigned int VBO;
  unsigned int indicies[] = {
    //TOP
    0, 1, 2,
    2, 3, 0,
    //BOTTOM
    4, 5, 6,
    6, 7, 4,
    //LEFT
    1, 6, 5,
    5, 2, 1,
    //RIGHT
    0, 3, 4,
    4, 7, 0,
    //FORWARD
    0, 7, 6,
    6, 1, 0,
    //BACK
    2, 5, 4,
    4, 3, 2
  };
  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies,
               GL_STATIC_DRAW);

  mat4 model = GLM_MAT4_IDENTITY_INIT;
  get_model_mat(entity, model);
  for (int i = 0; i < entity->model->num_colliders; i++) {
    bone = entity->model->collider_bone_links[i];
    type = entity->model->colliders[i].type;

    if (bone != -1) {
      mat4 bone_to_entity = GLM_MAT4_IDENTITY_INIT;
      glm_mat4_ins3(entity->model->bones[bone].coordinate_matrix,
                    bone_to_entity);
      if (type == POLY) {
        glm_vec3_copy(entity->model->colliders[i].data.center_of_mass,
                      bone_to_entity[3]);
      } else {
        glm_vec3_copy(entity->model->colliders[i].data.center,
                      bone_to_entity[3]);
      }

      mat4 bone_to_world = GLM_MAT4_IDENTITY_INIT;
      glm_mat4_mul(entity->final_b_mats[bone], bone_to_entity, bone_to_world);
      glm_mat4_mul(model, bone_to_world, used);
    } else {
      glm_mat4_copy(model, used);
    }

    if (type == POLY) {
      glGenVertexArrays(1, &VAO);
      glBindVertexArray(VAO);
      glGenBuffers(1, &VBO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 *
                   entity->model->colliders[i].data.num_used,
                   entity->model->colliders[i].data.verts, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3,
                            (void *) 0);
      glEnableVertexAttribArray(0);

      glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                       (float *) used);

      if (entity->model->colliders[i].data.num_used == 8) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
      } else {
        glDrawArrays(GL_POINTS, 0, entity->model->colliders[i].data.num_used);
      }
      if (entity->model->colliders[i].data.num_used == 8) {
      }
      glBindVertexArray(0);
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &VBO);
    } else if (type == SPHERE) {
      glm_translate(used, entity->model->colliders[i].data.center);
      glm_scale_uni(used, entity->model->colliders[i].data.radius);
      glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                       (float *) used);
      draw_model(shader, sphere);
    }
  }
  glDeleteBuffers(1, &EBO);
}

void free_entity(ENTITY *entity) {
  if (entity == NULL) {
    return;
  }

  if (entity->bone_mats) {
    free(entity->bone_mats);
    free(entity->final_b_mats);
  }
  if (entity->np_data) {
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
