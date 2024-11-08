#include <render_primitives.h>

void draw_lines(L_VBO *lines, size_t num_lines) {
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(L_VBO) * num_lines * 2, lines,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(L_VBO), (void *) 0);
  glVertexAttribIPointer(1, 1, GL_INT, sizeof(L_VBO),
                         (void *) (3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glDrawArrays(GL_LINES, 0, 2 * num_lines);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}

void draw_poly(vec3 *verts) {
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

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 8, verts, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3,
                        (void *) 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}

