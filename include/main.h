#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/cam.h>
#include <math.h>
#include <stdlib.h>
#include <entity_str.h>

#define RES_X (640.0)
#define RES_Y (400.0)

extern vec3 U_DIR;
extern vec3 D_DIR;
extern vec3 L_DIR;
extern vec3 R_DIR;
extern vec3 F_DIR;
extern vec3 B_DIR;

typedef struct physics_object {
  ENTITY *entity;
  size_t collider_offset;
  size_t node_offset;
  size_t next_offset;
  size_t prev_offset;
} PHYS_OBJ;

typedef struct oct_tree_node {
  size_t head_offset;
  size_t tail_offset;
  int next_offset;
  int empty;
} OCT_NODE;

typedef struct oct_tree {
  OCT_NODE *node_buffer;
  PHYS_OBJ *data_buffer;
  size_t node_buff_len;
  size_t node_buff_size;
  size_t data_buff_len;
  size_t data_buff_size;
} OCT_TREE;

typedef struct collision_result {
  PHYS_OBJ **list;
  size_t list_len;
  size_t list_buff_size;
} COLLISION_RES;

extern float delta_time;
extern float last_frame;
extern OCT_TREE *physics_tree;

int init_scene();

void keyboard_input(GLFWwindow *window);
void mouse_input(GLFWwindow *widnow, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow *, int, int);
void draw_oct_tree(MODEL *cube, OCT_TREE *tree, vec3 pos, float scale,
                   unsigned int shader, size_t offset, int depth);

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

int init_simulation();
int simulate_frame();
int insert_entity(ENTITY *entity);
int remove_entity(ENTITY *entity);
void end_simulation();

OCT_TREE *init_tree();
int oct_tree_insert(OCT_TREE *tree, ENTITY *entity, size_t collider_offset);
int oct_tree_delete(OCT_TREE *tree, size_t obj_offset);
COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *hit_box);
void free_oct_tree(OCT_TREE *tree);
void get_model_mat(ENTITY *entity, mat4 model);
void global_collider(mat4 model_mat, COLLIDER *source, COLLIDER *dest);

int max_dot(vec3 *verts, unsigned int len, vec3 dir);
void vec3_remove_noise(vec3 v, float threshold);
