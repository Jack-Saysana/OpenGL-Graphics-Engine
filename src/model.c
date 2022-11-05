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
