#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <globals.h>
#include <const.h>
#include <structs/models/entity_str.h>
#include <structs/ui_component_str.h>
#include <structs/font_str.h>

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

int init_ui_comp(UI_COMP *comp, char *text, vec3 text_col, vec3 pos,
                 float width, float height, float line_height,
                 int manual_layer, PIVOT pivot, TEXT_ANCHOR txt_anc,
                 int opts, int enabled, int display, int textured,
                 unsigned int texture,
                 void (*on_click)(UI_COMP *, void *),
                 void (*on_release)(UI_COMP *, void *),
                 void (*on_hover)(UI_COMP *, void *),
                 void (*on_no_hover)(UI_COMP *, void *),
                 void *click_args, void *release_args, void *hover_args,
                 void *no_hover_args);
void free_ui_comp(UI_COMP *comp);
void calc_pix_stats(UI_COMP *parent, UI_COMP *child, vec2 top_left,
                    vec2 next_rel_pos, float *next_line_y);
int sort_ui_components(UI_COMP **comp, size_t comp_len);

static void check_hover_event(UI_COMP *comp);
static void on_click_callback(GLFWwindow *window, int button, int action,
                              int mods);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int register_mouse_button_callback(void (*cb)(GLFWwindow *, int, int, int));

void draw_model(unsigned int shader, MODEL *model);
MODEL *load_model(char *path);
void free_model(MODEL *model);

int gen_texture_id(char *path, unsigned int *dest);

unsigned int init_shader_prog(char *v_path, char *g_path, char *f_path);

int draw_text(char *str, size_t str_len, vec3 col, TEXT_ANCHOR txt_anc,
              vec3 pos, float screen_width, float screen_height, float width,
              float line_height, F_GLYPH *font, unsigned int shader);

void set_mat4(char *name, mat4 matrix, unsigned int shader);
void set_int(char *name, int val, unsigned int shader);

void print_mat4(mat4 mat);
int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
