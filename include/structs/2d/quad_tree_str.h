#ifndef __QUAD_TREE_STR_H__
#define __QUAD_TREE_STR_H__

#include <structs/2d/models/entity_2d_str.h>

ypedef struct physics_object_2d {
  ENTITY_2D *ent;
  size_t collider_offset;
  // Index of oct tree node which contains the given PHYS_OBJ
  size_t node_offset;
  // Index of next PHYS_OBJ residing in the same node in data_buffer
  size_t next_offset;
  // Index of prev PHYS_OBJ residing in the same node in data_buffer
  size_t prev_offset;
} PHYS_OBJ_2D;

typedef struct quad_tree_node {
  // Index of first PHYS_OBJ_2D residing at the current node in data-buffer
  size_t head_offset;
  // Index of last PHYS_OBJ_2D residing at the current node in data_buffer
  size_t tail_offset;
  // Index of first child node in node_buffer. Other 3 children follow
  // immediately after
  size_t next_offset;
  // Flag denoting if node and its children nodes are absent of physics objects
  int empty;
} QUAD_NODE;

typedef struct quad_tree {
  // Mutex used for ensuring quad_tree_search() is thread_safe
  pthread_mutex_t search_lock;
  QUAD_NODE *node_buffer;
  PHYS_OBJ_2D *data_buffer;
  size_t node_buff_len;
  size_t node_buff_size;
  size_t data_buff_len;
  size_t data_buff_size;
  float max_extent;
  unsigned int max_depth;
} QUAD_TREE;

typedef struct 2d_collision_result {
  PHYS_OBJ_2D **list;
  size_t list_len;
  size_t list_buff_size;
} 2D_COLLISION_RES;

#endif
