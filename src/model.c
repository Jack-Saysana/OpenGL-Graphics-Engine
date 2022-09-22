#include <model.h>

void draw_model(MODEL *model) {
  glBindVertexArray(model->VAO);
  glDrawElements(GL_TRIANGLES, model->num_verts, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
