#include <model.h>

void draw_model(unsigned int shader, MODEL *model) {
  if (model == NULL) {
    return;
  }

  int uniform_loc = -1;
  char *uniform_names[5] = { "material.amb_map", "material.diff_map",
                             "material.spec_map", "material.spec_exponent",
                             "material.bump_map" };

  for (int i = 0; i < NUM_PROPS; i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, model->textures[i]);
    uniform_loc = glGetUniformLocation(shader, uniform_names[i]);
    if (uniform_loc != -1) {
      glUniform1i(uniform_loc, i);
    }
  }

  glBindVertexArray(model->VAO);
  glDrawElements(GL_TRIANGLES, model->num_indicies, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void draw_bones(MODEL *model) {
  if (model == NULL) {
    return;
  }

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  typedef struct b_vbo {
    float coords[3];
    int bone_id;
  } B_VBO;

  B_VBO *bone_data = malloc(sizeof(B_VBO) * model->num_bones);
  for (int i = 0; i < model->num_bones; i++) {
    bone_data[i].coords[0] = model->bones[i].coords[0];
    bone_data[i].coords[1] = model->bones[i].coords[1];
    bone_data[i].coords[2] = model->bones[i].coords[2];
    bone_data[i].bone_id = i;
  }

  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(B_VBO) * model->num_bones,
               bone_data, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(B_VBO),
                        (void *) 0);
  glVertexAttribIPointer(1, 1, GL_INT, sizeof(B_VBO),
                        (void *) (3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glDrawArrays(GL_POINTS, 0, model->num_bones);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  free(bone_data);
}

void free_model(MODEL *model) {
  glDeleteVertexArrays(1, &(model->VAO));
  glDeleteBuffers(1, &(model->VBO));
  glDeleteBuffers(1, &(model->EBO));
  free(model->k_chain_block);
  free(model->keyframe_block);
  free(model->sled_block);
  free(model->animations);
  free(model->bones);
  free(model->colliders);
  free(model->collider_bone_links);
  for (int i = 0; i < model->num_bones; i++) {
    free(model->bone_relations[i]);
  }
  free(model->bone_relations);
  free(model);
}
