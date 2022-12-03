#include <model.h>

void draw_model(unsigned int shader, MODEL *model) {
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
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(BONE) * model->num_bones, model->bones,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BONE),
                        (void *) 0);
  glEnableVertexAttribArray(0);

  glDrawArrays(GL_POINTS, 0, model->num_bones * 3);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}

void free_model(MODEL *model) {
  free(model->bones);
  free(model);
}
