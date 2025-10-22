#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include <const.h>
#include <structs/2d/models/entity_2d_str.h>
#include <structs/models/render_primitives_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

void draw_2d_collider(unsigned int shader, COLLIDER_2D *col, vec3 ent_pos);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

void draw_square(vec2 center, float half_w, float half_h);
void draw_circle(vec2 center, float radius);
void draw_lines(L_VBO *lines, size_t num_lines);
void set_mat4(char *name, mat4 mat, unsigned int shader);
void draw_quad();
int get_texture_id(char *tex_path, unsigned int *dest);
int is_moving_2d(ENTITY_2D *ent, size_t col);
void integrate_ent_2d(ENTITY_2D *ent, vec2 forces);
