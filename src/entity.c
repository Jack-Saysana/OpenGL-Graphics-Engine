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
    ent->tree_offsets = malloc(sizeof(size_t) * model->num_colliders);
    if (ent->tree_offsets == NULL) {
      free(ent);
      return NULL;
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

    model->ref_count++;
  } else {
    ent->bone_mats = NULL;
  }

  ent->model = model;

  glm_mat4_identity(ent->model_mat);

  return ent;
}

void draw_entity(unsigned int shader, ENTITY *entity) {
  if (entity == NULL) {
    return;
  }

  glUseProgram(shader);
  for (int i = 0; i < entity->model->num_bones; i++) {
    char var_name[50];
    sprintf(var_name, "bones[%d].coords", i);
    glUniform3f(glGetUniformLocation(shader, var_name),
                entity->model->bones[i].coords[0],
                entity->model->bones[i].coords[1],
                entity->model->bones[i].coords[2]);
    sprintf(var_name, "bones[%d].parent", i);
    glUniform1i(glGetUniformLocation(shader, var_name),
                 entity->model->bones[i].parent);

    sprintf(var_name, "bone_mats[%d]", i);
    glUniformMatrix4fv(glGetUniformLocation(shader, var_name),
                       3, GL_FALSE,
                       (float *) entity->bone_mats[i]);
  }

  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1,
                     GL_FALSE, (float *) entity->model_mat);

  draw_model(shader, entity->model);
}

void draw_skeleton(unsigned int shader, ENTITY *entity) {
  if (entity == NULL || entity->bone_mats == NULL) {
    return;
  }

  glUseProgram(shader);
  for (int i = 0; i < entity->model->num_bones; i++) {
    char var_name[50];
    sprintf(var_name, "bones[%d].coords", i);
    glUniform3f(glGetUniformLocation(shader, var_name),
                entity->model->bones[i].coords[0],
                entity->model->bones[i].coords[1],
                entity->model->bones[i].coords[2]);
    sprintf(var_name, "bones[%d].parent", i);
    glUniform1i(glGetUniformLocation(shader, var_name),
                 entity->model->bones[i].parent);

    sprintf(var_name, "bone_mats[%d]", i);
    glUniformMatrix4fv(glGetUniformLocation(shader, var_name),
                       3, GL_FALSE,
                       (float *) entity->bone_mats[i]);
  }
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1,
                     GL_FALSE, (float *) entity->model_mat);

  draw_bones(entity->model);
}

void free_entity(ENTITY *entity) {
  if (entity->model->ref_count > 0) {
    entity->model->ref_count--;
  }
  if (entity->tree_offsets) {
    free(entity->tree_offsets);
  }
  if (entity->bone_mats) {
    free(entity->bone_mats);
  }

  free(entity);
}
