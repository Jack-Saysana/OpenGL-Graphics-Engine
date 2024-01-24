#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <globals.h>
#include <const.h>
#include <entity_str.h>
#include <ui_component_str.h>
#include <font_str.h>

#define STK_SIZE_INIT (10)

// ================================= GLOBALS =================================

UI_COMP ui_root = INVALID_COMP_INIT;
static MODEL *ui_quad = NULL;
static unsigned int ui_shader = 0;
static unsigned int text_shader = 0;
static UI_COMP **render_stack = NULL;
static size_t render_stk_top = 0;
static size_t render_stk_size = 0;

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

int init_ui_comp(UI_COMP *, char *, vec3, vec3, float, float, float, int,
                 PIVOT, TEXT_ANCHOR, int, int, int, int, unsigned int,
                 void (*)(UI_COMP *, void *), void (*)(UI_COMP *, void *),
                 void (*)(UI_COMP *, void *), void (*)(UI_COMP *, void *),
                 void *, void *, void *, void *);
void free_ui_comp(UI_COMP *);
void calc_pix_stats(UI_COMP *, UI_COMP *, vec2, vec2, float *);

static void check_hover_event(UI_COMP *);
static void on_click_callback(GLFWwindow *, int, int, int);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int register_mouse_button_callback(void (*)(GLFWwindow *, int, int, int));

void draw_model(unsigned int, MODEL *);
MODEL *load_model(char *);
void free_model(MODEL *);

int gen_texture_id(char *, unsigned int *);

unsigned int init_shader_prog(char *, char *, char *);

int draw_text(char *, size_t, vec3, TEXT_ANCHOR, vec3, float, float, float,
              float, F_GLYPH *, unsigned int);

void set_mat4(char *, mat4, unsigned int);
void set_int(char *, int, unsigned int);

void print_mat4(mat4);
int double_buffer(void **, size_t *, size_t);
