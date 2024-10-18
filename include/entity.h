#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>
#include <cglm/quat.h>
#include <cglm/affine.h>
#include <structs/models/entity_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

void draw_model(unsigned int shader, MODEL *model);
void draw_bones(MODEL *model);
void draw_axes(unsigned int shader, MODEL *model);
void set_vec3(char *loc, vec3 vec, unsigned int shader);
