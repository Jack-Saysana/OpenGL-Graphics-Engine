#include <stdio.h>
#include <stdlib.h>
#include <cglm/vec3.h>
#include <entity_str.h>

#define BUFF_STARTING_LEN (10)

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

static ENTITY **dynamic_ents = NULL;
static size_t d_ent_buff_len = 0;
static size_t d_ent_buff_size = 0;

static ENTITY **static_ents = NULL;
static size_t s_ent_buff_len = 0;
static size_t s_ent_buff_size = 0;

static OCT_TREE *physics_tree = NULL;

int init_simulation();
int simulate_frame();
int insert_entity(ENTITY *entity);
ENTITY *remove_entity(size_t location);
void end_simulation();

OCT_TREE *init_tree();
int oct_tree_insert(OCT_TREE *tree, ENTITY *entity, size_t collider_offset);
int oct_tree_delete(OCT_TREE *tree, size_t obj_offset);
COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *hit_box);
void free_oct_tree(OCT_TREE *tree);
int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
