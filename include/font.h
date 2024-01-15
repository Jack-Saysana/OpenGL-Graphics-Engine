#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <const.h>
#include <font_str.h>
#include <ui_component_str.h>
#include <cglm/cglm.h>

extern F_GLYPH *font;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

void draw_glyph(F_GLYPH *, unsigned int);

float get_next_x(vec2, float, float, float, TEXT_ANCHOR);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

unsigned int gen_texture_id(char *);

void set_int(char *, int, unsigned int);
void set_mat4(char *, mat4, unsigned int);
void set_vec3(char *, vec3, unsigned int);

int double_buffer(void **, size_t *, size_t);
