#include <glad/glad.h>
#include <cglm/cglm.h>
#include <globals.h>
#include <const.h>
#include <entity_str.h>
#include <ui_component_str.h>

#define STK_SIZE_INIT (10)

// ================================= GLOBALS =================================

UI_COMP ui_root = INVALID_COMP_INIT;
MODEL *ui_quad = NULL;
unsigned int ui_shader = 0;
UI_COMP **render_stack = NULL;
size_t render_stk_top = 0;
size_t render_stk_size = 0;

vec2 UI_PIVOT_OFFSETS[9] = {
  { 0.0,  0.0 }, // CENTER
  { 0.0, -1.0 }, // TOP
  { 0.0,  1.0 }, // BOTTOM
  { 1.0,  0.0 }, // LEFT
  {-1.0,  0.0 }, // RIGHT
  { 1.0, -1.0 }, // TOP LEFT
  {-1.0, -1.0 }, // TOP RIGHT
  { 1.0,  1.0 }, // BOTTOM LEFT
  {-1.0,  1.0 }  // BOTTOM RIGHT
};

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

int init_ui_comp(UI_COMP *, vec2, float, float, PIVOT, TEXT_ANCHOR, int, int,
                 int);
void free_ui_comp(UI_COMP *);
void calc_pix_stats(UI_COMP *, UI_COMP *, vec2, vec2, float *);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

void draw_model(unsigned int, MODEL *);
MODEL *load_model(char *);
void free_model(MODEL *);

unsigned int init_shader_prog(char *, char *, char *);

void set_mat4(char *, mat4, unsigned int);

void print_mat4(mat4);
int double_buffer(void **, size_t *, size_t);
