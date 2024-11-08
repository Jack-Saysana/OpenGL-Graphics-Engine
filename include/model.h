#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <structs/models/entity_str.h>
#include <structs/models/render_primitives_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

void set_vec3(char *loc, vec3 vec, unsigned int shader);
void draw_lines(L_VBO *lines, size_t num_lines);
