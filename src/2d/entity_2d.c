#include <2d/entity_2d.h>

ENTITY_2D *init_entity_2d(int type, float height, float width) {
  ENTITY_2D *ent = malloc(sizeof(ENTITY_2D));
  if (!ent) {
    goto ENT_ERR;
  }
  memset(ent, 0, sizeof(ENTITY_2D));
  ent->type = type;
  ent->height = height;
  ent->width = width;
  ent->move_cb = integrate_ent_2d;
  ent->is_moving_cb = is_moving_2d;

  return ent;

ENT_ERR:
  return NULL;
}

void draw_entity_2d(unsigned int shader, ENTITY_2D *ent) {
  if (!ent) {
    return;
  }

  glUseProgram(shader);

  mat4 model = GLM_MAT4_IDENTITY_INIT;
  glm_translate(model, ent->pos);
  // Make sure colliders appear on top of actual quad
  glm_translate(model, (vec3) { 0.0, 0.0, -0.1 });
  glm_scale(model, (vec3) { ent->width / 2.0, ent->height / 2.0, 1.0 });
  set_mat4("model", model, shader);
  draw_quad();
}

void draw_2d_colliders(unsigned int shader, ENTITY_2D *ent) {
  if (!ent) {
    return;
  }

  glUseProgram(shader);
  for (size_t i = 0; i < ent->num_cols; i++) {
    draw_2d_collider(shader, ent->cols + i, ent->pos);
  }
}

void draw_2d_collider(unsigned int shader, COLLIDER_2D *col, vec3 ent_pos) {
  mat4 col_to_world = GLM_MAT4_IDENTITY_INIT;
  glm_translate(col_to_world, ent_pos);

  set_mat4("model", col_to_world, shader);

  if (col->type == SQUARE) {
    draw_square(col->center, col->data.width / 2.0, col->data.height / 2.0);
  } else {
    draw_circle(col->center, col->data.radius);
  }
}

void free_entity_2d(ENTITY_2D *ent) {
  free(ent->cols);
  free(ent);
}
