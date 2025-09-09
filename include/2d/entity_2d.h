#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include <const.h>
#include <structs/2d/models/entity_2d_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

void draw_2d_collider(unsigned int shader, COLLIDER_2D *col, vec3 ent_pos);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

void draw_square(vec2 center, float half_w, float half_h);
void draw_circle(vec2 center, float radius);
void set_mat4(char *name, mat4 mat, unsigned int shader);
void draw_quad();
