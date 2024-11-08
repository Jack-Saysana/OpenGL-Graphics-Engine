#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <const.h>
#include <structs/font_str.h>
#include <structs/ui_component_str.h>
#include <cglm/cglm.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

void draw_glyph(F_GLYPH *glyph, unsigned int shader);

float get_next_x(vec2 pos, float line_width, float comp_width, float x_off,
                 TEXT_ANCHOR txt_anc);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int gen_texture_id(char *tex_path, unsigned int *dest);

void set_int(char *name, int val, unsigned int shader);
void set_mat4(char *name, mat4 matrix, unsigned int shader);
void set_vec3(char *name, vec3 v, unsigned int shader);

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
