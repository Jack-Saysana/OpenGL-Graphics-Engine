#ifndef __OCT_TREE_STR_H__
#define __OCT_TREE_STR_H__

#include <structs/models/entity_str.h>

#define X_Y_Z          (0)
#define X_Y_negZ       (1)
#define X_negY_Z       (2)
#define X_negY_negZ    (3)
#define negX_Y_Z       (4)
#define negX_Y_negZ    (5)
#define negX_negY_Z    (6)
#define negX_negY_negZ (7)

#define OCT_X_Y_Z           (0x1)
#define OCT_X_Y_negZ        (0x2)
#define OCT_X_negY_Z        (0x4)
#define OCT_X_negY_negZ     (0x8)
#define OCT_negX_Y_Z       (0x10)
#define OCT_negX_Y_negZ    (0x20)
#define OCT_negX_negY_Z    (0x40)
#define OCT_negX_negY_negZ (0x80)

typedef struct physics_object {
#ifdef DEBUG_OCT_TREE
  COLLIDER add_state;
#endif
  ENTITY *entity;
  size_t collider_offset;
  // Index of oct tree node which contains the given PHYS_OBJ
  size_t node_offset;
  // Index of next PHYS_OBJ residing in the same node in data_buffer
  size_t next_offset;
  // Index of prev PHYS_OBJ residing in the same node in data_buffer
  size_t prev_offset;
  // Debugging info used to identify how/where the physics object was inseted
  // into the oct tree
#ifdef DEBUG_OCT_TREE
  int birthmark;
#endif
} PHYS_OBJ;

typedef struct oct_tree_node {
  // Index of first PHYS_OBJ residing at the current node in data-buffer
  size_t head_offset;
  // Index of last PHYS_OBJ residing at the current node in data_buffer
  size_t tail_offset;
  // Index of first child node in node_buffer. Other 7 children follow
  // immediately after
  //int next_offset;
  size_t next_offset;
  // Flag denoting if node and its children nodes are absent of physics objects
  int empty;
} OCT_NODE;

typedef struct oct_tree {
  // Mutex used for ensuring oct_tree_search() is thread_safe
  pthread_mutex_t search_lock;
  OCT_NODE *node_buffer;
  PHYS_OBJ *data_buffer;
  size_t node_buff_len;
  size_t node_buff_size;
  size_t data_buff_len;
  size_t data_buff_size;
//  unsigned int type;
  float max_extent;
  unsigned int max_depth;
} OCT_TREE;

typedef struct collision_result {
  PHYS_OBJ **list;
  size_t list_len;
  size_t list_buff_size;
} COLLISION_RES;

#endif
