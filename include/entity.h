#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cglm/quat.h>
#include <cglm/affine.h>
#include <structs/models/entity_str.h>
#include <structs/models/render_primitives_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

void draw_collider(unsigned int shader, ENTITY *entity, size_t col,
                   MODEL *sphere);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

void draw_model(unsigned int shader, MODEL *model);
void draw_bones(MODEL *model);
void draw_axes(unsigned int shader, MODEL *model);
void set_vec3(char *loc, vec3 vec, unsigned int shader);
void draw_poly(vec3 *verts);
void draw_lines(L_VBO *lines, size_t num_lines);
