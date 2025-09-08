#include <render_primitives.h>

void init_quad() {
  float verts[] = {
    // VERTEX        TEX_COORD
    -1.0,  1.0, 0.0, 0.0, 1.0,
    -1.0, -1.0, 0.0, 0.0, 0.0,
     1.0, -1.0, 0.0, 1.0, 0.0,
     1.0,  1.0, 0.0, 1.0, 1.0
  };
  unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
  };

  glGenBuffers(1, &QUAD_EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, QUAD_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);


  glGenVertexArrays(1, &QUAD_VAO);
  glBindVertexArray(QUAD_VAO);
  glGenBuffers(1, &QUAD_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, QUAD_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5,
                        (void *) 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5,
                        (void *) (sizeof(GLfloat) * 3));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

void draw_quad() {
  if (QUAD_VAO == INVALID_INDEX) {
    init_quad();
  }

  glBindVertexArray(QUAD_VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, QUAD_EBO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

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

void draw_square(vec2 center, float half_len) {
  L_VBO points[8];
  memset(points, 0, sizeof(points));

  vec3 top_right = GLM_VEC3_ZERO_INIT;
  vec3 bottom_right = GLM_VEC3_ZERO_INIT;
  vec3 bottom_left = GLM_VEC3_ZERO_INIT;
  vec3 top_left = GLM_VEC3_ZERO_INIT;

  glm_vec3_copy((vec3) { center[X] + half_len, center[Y] + half_len, 0.0 },
                top_right);
  glm_vec3_copy((vec3) { center[X] + half_len, center[Y] - half_len, 0.0 },
                bottom_right);
  glm_vec3_copy((vec3) { center[X] - half_len, center[Y] - half_len, 0.0 },
                bottom_left);
  glm_vec3_copy((vec3) { center[X] - half_len, center[Y] + half_len, 0.0 },
                top_left);

  glm_vec3_copy(top_right, points[0].coords);
  glm_vec3_copy(bottom_right, points[1].coords);
  glm_vec3_copy(bottom_right, points[2].coords);
  glm_vec3_copy(bottom_left, points[3].coords);
  glm_vec3_copy(bottom_left, points[4].coords);
  glm_vec3_copy(top_left, points[5].coords);
  glm_vec3_copy(top_left, points[6].coords);
  glm_vec3_copy(top_right, points[7].coords);

  draw_lines(points, 8);
}

void draw_circle(vec2 center, float radius) {
  L_VBO points[24];
  memset(points, 0, sizeof(points));

  vec3 verts[12];
  for (int i = 0; i < 12; i++) {
    float angle = 2.0 * PI * ((1.0 / 12.0) * i);
    verts[i][X] = cos(angle);
    verts[i][Y] = sin(angle);
    verts[i][Z] = 0.0;
  }

  for (int i = 0; i < 12; i++) {
    glm_vec3_copy(verts[i], points[i*2].coords);
    glm_vec3_copy(verts[(i+1) % 12], points[(i*2)+1].coords);
  }

  draw_lines(points, 12);
}
