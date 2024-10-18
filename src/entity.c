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
    ent->np_data = malloc(sizeof(P_DATA) * model->num_colliders);
    if (ent->np_data == NULL) {
      free(ent->p_cons);
      free(ent);
      return NULL;
    }
    ent->zj_data = NULL;

    for (size_t i = 0; i < model->num_colliders; i++) {
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
      vec6_zero(ent->np_data[i].e_force);
      glm_vec3_zero(ent->np_data[i].a);
      glm_vec3_zero(ent->np_data[i].ang_a);
      glm_vec3_zero(ent->np_data[i].v);
      glm_vec3_zero(ent->np_data[i].ang_v);
      glm_vec3_zero(ent->np_data[i].dof);
      glm_vec3_zero(ent->np_data[i].from_parent_lin);
      glm_vec3_zero(ent->np_data[i].joint_to_com);
      glm_vec3_zero(ent->np_data[i].e_force);
      ent->np_data[i].zero_joint_offset = INVALID_INDEX;
      ent->np_data[i].num_z_joints = 0;
      ent->np_data[i].joint_type = JOINT_REVOLUTE;
      ent->np_data[i].inv_mass = 0.0;
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
      free(ent->p_cons);
      free(ent);
      return NULL;
    }

    ent->final_b_mats = malloc(sizeof(mat4) * model->num_bones);
    if (ent->final_b_mats == NULL) {
      free(ent->bone_mats);
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

void draw_colliders(unsigned int shader, ENTITY *entity, MODEL *sphere) {
  if (entity == NULL) {
    return;
  }

  glUseProgram(shader);
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

  for (int i = 0; i < entity->model->num_colliders; i++) {
    bone = entity->model->collider_bone_links[i];
    type = entity->model->colliders[i].type;

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
                         (float *) bone_to_world);

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
      glm_translate(bone_to_world, entity->model->colliders[i].data.center);
      glm_scale_uni(bone_to_world, entity->model->colliders[i].data.radius);
      glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                       (float *) bone_to_world);
      draw_model(shader, sphere);
    }
  }
  glDeleteBuffers(1, &EBO);
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

