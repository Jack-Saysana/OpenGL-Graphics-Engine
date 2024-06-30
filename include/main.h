#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/cam.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <globals.h>
#include <const.h>
#include <structs/ui_component_str.h>
#include <structs/entity_str.h>
#include <structs/simulation_str.h>
#include <structs/font_str.h>

extern vec3 U_DIR;
extern vec3 D_DIR;
extern vec3 L_DIR;
extern vec3 R_DIR;
extern vec3 F_DIR;
extern vec3 B_DIR;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

void draw_oct_tree(MODEL *cube, OCT_TREE *tree, vec3 pos, float scale,
                   unsigned int shader, size_t offset, int depth);
void framebuffer_size_callback(GLFWwindow *, int, int);
void keyboard_input(GLFWwindow *window);
void mouse_input(GLFWwindow *widnow, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void test_callback_click(UI_COMP *, void *);
void test_callback_release(UI_COMP *, void *);
void test_callback_hover(UI_COMP *, void *);
void test_callback_no_hover(UI_COMP *, void *);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

GLFWwindow *init_gl(char *);
void cleanup_gl();
int register_fb_size_callback(void (*)(GLFWwindow *, int, int));
int register_mouse_movement_callback(void (*)(GLFWwindow *, double, double));
int register_scroll_callback(void (*)(GLFWwindow *, double, double));
int register_mouse_button_callback(void (*)(GLFWwindow *, int, int, int));

int init_scene();

unsigned int init_shader_prog(char *, char *, char *);
MODEL *load_model(char *path);
ENTITY *init_entity(MODEL *model);
int animate(ENTITY *entity, unsigned int animation_index, unsigned int frame);
void draw_entity(unsigned int shader, ENTITY *entity);
void draw_skeleton(unsigned int shader, ENTITY *entity);
void draw_colliders(unsigned int shader, ENTITY *entity, MODEL *sphere);
void draw_model(unsigned int shader, MODEL *model);
void free_model(MODEL *model);
void free_entity(ENTITY *entity);

int simulate_frame(SIMULATION *sim);
SIMULATION *init_sim(float, unsigned int);
void free_sim(SIMULATION *sim);
int sim_add_entity(SIMULATION *sim, ENTITY *entity, int collider_filter);
int sim_remove_entity(SIMULATION *sim, ENTITY *entity);
void sim_add_force(SIMULATION *sim, vec3 force);
void sim_clear_force(SIMULATION *sim);
void prep_sim_movement(SIMULATION *);
//void update_sim_movement(SIMULATION *, int);
void update_sim_movement(SIMULATION *);
void integrate_sim(SIMULATION *sim, vec3 origin, float range);
size_t get_sim_collisions(SIMULATION *sim, COLLISION **dest,
                          vec3 origin, float range, int get_col_info);
void impulse_resolution(SIMULATION *sim, COLLISION col);

OCT_TREE *init_tree(float max_extent, unsigned int max_depth);
//int oct_tree_insert(OCT_TREE *tree, ENTITY *entity, size_t collider_offset,
//                    int birthmark);
//int oct_tree_delete(OCT_TREE *tree, ENTITY *entity, size_t collider_offset);
//COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *hit_box);
void free_oct_tree(OCT_TREE *tree);

int init_ui(char *quad_path, char *ui_vs, char *ui_fs, char *text_vs,
            char *text_fs);
int free_ui();
UI_COMP *add_ui_comp(UI_COMP *, vec2, float, float, int);
int render_ui();
void set_ui_pos(UI_COMP *, vec2);
void set_ui_width(UI_COMP *, float);
void set_ui_height(UI_COMP *, float);
void set_manual_layer(UI_COMP *, float);
void disable_manual_layer(UI_COMP *);
void set_ui_pivot(UI_COMP *, PIVOT);
void set_ui_enabled(UI_COMP *, int);
void set_ui_display(UI_COMP *, int);
void set_ui_text(UI_COMP *, char *, float, TEXT_ANCHOR, F_GLYPH *, vec3);
void update_ui_text(UI_COMP *, char *);
void set_ui_text_col(UI_COMP *, vec3);
void set_ui_texture(UI_COMP *, char *);
void set_ui_options(UI_COMP *c, int);
void set_ui_enabled(UI_COMP *, int);
void set_ui_on_click(UI_COMP *, void (*)(UI_COMP *, void *), void *);
void set_ui_on_release(UI_COMP *, void (*)(UI_COMP *, void *), void *);
void set_ui_on_hover(UI_COMP *, void (*)(UI_COMP *, void *), void *);
void set_ui_no_hover(UI_COMP *, void (*)(UI_COMP *, void *), void *);

void draw_glyph(F_GLYPH *, unsigned int);

int max_dot(vec3 *verts, unsigned int len, vec3 dir);
void vec3_remove_noise(vec3 v, float threshold);
float remove_noise(float, float);

void set_mat4(char *, mat4, unsigned int);
void set_mat3(char *, mat3, unsigned int);
void set_vec4(char *, vec4, unsigned int);
void set_vec3(char *, vec3, unsigned int);
void set_vec2(char *, vec2, unsigned int);
void set_float(char *, float, unsigned int);
void set_int(char *, int, unsigned int);
void set_uint(char *, unsigned int, unsigned int);
void set_iarr(char *, int *, size_t, unsigned int);
