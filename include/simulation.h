#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <cglm/vec3.h>
#include <cglm/quat.h>
#include <cglm/affine.h>
#include <entity_str.h>

#define BUFF_STARTING_LEN (10)

#define DYNAMIC (0)
#define DRIVING (1)

// Masks for determining entity type
#define T_DYNAMIC (1)
#define T_DRIVING (2)
#define T_IMMUTABLE (4)

// Masks for determining collider categoty
#define DEFAULT (0)
#define HIT_BOX (1)
#define HURT_BOX (2)

#define GRAVITY (0.5)
vec3 G_VEC = { 0.0, GRAVITY, 0.0 };

#define DAMP_FACTOR (0.9f)

vec3 U_DIR = { 0.0, 1.0, 0.0 };
vec3 D_DIR = { 0.0, -1.0, 0.0 };
vec3 L_DIR = { 1.0, 0.0, 0.0 };
vec3 R_DIR = { -1.0, 1.0, 0.0 };
vec3 F_DIR = { 0.0, 1.0, 1.0 };
vec3 B_DIR = { 0.0, 1.0, -1.0 };

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

float last_frame = 0.0;
float delta_time = 0.0;

static ENTITY **dynamic_ents = NULL;
static size_t dy_ent_buff_len = 0;
static size_t dy_ent_buff_size = 0;

static ENTITY **driving_ents = NULL;
static size_t dr_ent_buff_len = 0;
static size_t dr_ent_buff_size = 0;

OCT_TREE *physics_tree = NULL;

// Front Facing
int init_simulation();
int simulate_frame();
int insert_entity(ENTITY *entity);
int remove_entity(ENTITY *entity);
void end_simulation();

// Back Facing
int collision_test(ENTITY *subject, size_t offset);
void solve_collision(ENTITY *a, COLLIDER *a_col, ENTITY *b, COLLIDER *b_col,
                     vec3 p_dir, vec3 p_loc);
void remove_from_elist(ENTITY **list, int type, size_t index, size_t *len);
int add_to_elist(ENTITY **list, size_t *len, size_t *buff_size,
                 ENTITY *entity);
void global_collider(mat4 model_mat, COLLIDER *source, COLLIDER *dest);
void vec3_remove_noise(vec3 vec, float threshold);
void vec4_remove_noise(vec4 vec, float threshold);

// Outside helpers
OCT_TREE *init_tree();
int oct_tree_insert(OCT_TREE *tree, ENTITY *entity, size_t collider_offset);
int oct_tree_delete(OCT_TREE *tree, size_t obj_offset);
COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *hit_box);
void free_oct_tree(OCT_TREE *tree);
int collision_check(COLLIDER *a, COLLIDER *b, vec3 *simplex);
int epa_response(COLLIDER *a, COLLIDER *b, vec3 *simplex, vec3 p_dir,
                 float *p_depth);
void collision_point(COLLIDER *a, COLLIDER *b, vec3 p_vec, vec3 dest);
int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
void get_model_mat(ENTITY *entity, mat4 model);
int max_dot(vec3 *verts, unsigned int len, vec3 dir);
