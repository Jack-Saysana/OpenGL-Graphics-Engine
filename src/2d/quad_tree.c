#include <2d/quad_tree.h>

QUAD_TREE *init_quad_tree(float max_extent, unsigned int max_depth) {
  if (max_extent <= 0.0) {
    return NULL;
  }

  QUAD_TREE *tree = malloc(sizeof(QUAD_TREE));
  if (tree == NULL) {
    fprintf(stderr, "Error: Unable to allocate quad-tree\n");
    goto TREE_ERR;
  }

  if (pthread_mutex_init(&tree->search_lock, NULL)) {
    fprintf(stderr, "Error: Unable to initailize quad tree mutex\n");
    goto LOCK_ERR;
  }

  tree->node_buffer = malloc(sizeof(QUAD_NODE) * QUAD_TREE_STARTING_LEN);
  if (tree->node_buffer == NULL) {
    fprintf(stderr, "Error: Unable to allocate quad nodes\n");
    goto NODE_ERR;
  }

  tree->data_buffer = malloc(sizeof(PHYS_OBJ_2D) * BUFF_STARTING_LEN);
  if (tree->data_buffer == NULL) {
    fprintf(stderr, "Error: Unable to allocate node data\n");
    goto POBJ_ERR;
  }

  tree->node_buff_len = 1;
  tree->data_buff_len = 0;
  tree->node_buff_size = QUAD_TREE_STARTING_LEN;
  tree->data_buff_size = BUFF_STARTING_LEN;

  tree->node_buffer[0].empty = 1;
  tree->node_buffer[0].head_offset = INVALID_INDEX;
  tree->node_buffer[0].tail_offset = INVALID_INDEX;
  tree->node_buffer[0].next_offset = INVALID_INDEX;

  tree->max_extent = max_extent;
  tree->max_depth = max_depth;

  return tree;

POBJ_ERR:
  free(tree->node_buffer);
NODE_ERR:
  pthread_mutex_destroy(&tree->search_lock);
LOCK_ERR:
  free(tree);
TREE_ERR:
  return NULL;
}

void free_quad_tree(QUAD_TREE *tree) {
  pthread_mutex_destroy(&tree->search_lock);
  free(tree->data_buffer);
  free(tree->node_buffer);
  free(tree);
}

int quad_tree_insert(QUAD_TREE *tree, ENTITY_2D *entity,
                     size_t collider_offset) {
  if (tree == NULL || entity == NULL ||
      collider_offset >= entity->num_cols) {
    fprintf(stderr, "Error: Invalid quad-tree insertion input\n");
    return -1;
  }

  COLLIDER_2D obj;
  memcpy(&obj, entity->cols + collider_offset, sizeof(COLLIDER_2D));
  glm_vec2_add((vec2) { entity->pos[X], entity->pos[Y] }, obj.center,
               obj.center);

  vec2 max_extent = { tree->max_extent, tree->max_extent };
  vec2 min_extent = { -tree->max_extent, -tree->max_extent };

  float quad_len = tree->max_extent;

  int cur_quad = 0;
  int lsb = 0;
  size_t cur_offset = 0;

  int status = 0;
  int depth = 1;
  int inserting = 1;
  while (inserting) {
    tree->node_buffer[cur_offset].empty = 0;
    if (depth == tree->max_depth) {
      inserting = 0;
      status = append_quad_buffer(tree, cur_offset, entity, collider_offset);
      if (status != 0) {
        return -1;
      }
    } else {
      cur_quad = detect_quadrant(min_extent, max_extent, quad_len, &obj);
      lsb = get_lsb(cur_quad);
      // If multiple bits are set, object collides with multiple quadants
      if (!cur_quad || cur_quad ^ lsb ) {
        inserting = 0;
        status = append_quad_buffer(tree, cur_offset, entity, collider_offset);
        if (status != 0) {
          return -1;
        }
      } else {
        if (tree->node_buffer[cur_offset].next_offset == INVALID_INDEX) {
          status = init_quad_node(tree, tree->node_buffer + cur_offset);
          if (status != 0) {
            return -1;
          }
        }

        cur_offset = tree->node_buffer[cur_offset].next_offset +
                     update_quad_extents(cur_quad, min_extent, max_extent,
                                         quad_len);
        quad_len /= 2.0;
        depth++;
      }
    }
  }

  return 0;
}

int quad_tree_delete(QUAD_TREE *tree, ENTITY_2D *entity,
                     size_t collider_offset) {
  if (tree == NULL || entity == NULL ||
      collider_offset >= entity->num_cols) {
    fprintf(stderr, "Error: Invalid quad-tree deletion input\n");
    return -1;
  }

  COLLIDER_2D obj;
  memcpy(&obj, entity->cols + collider_offset, sizeof(COLLIDER_2D));
  glm_vec2_add((vec2) { entity->pos[X], entity->pos[Y] }, obj.center,
               obj.center);

  COLLISION_RES_2D candidates = quad_tree_search(tree, &obj);
  if (candidates.list == NULL) {
    return -1;
  }

  PHYS_OBJ_2D *cur_cand = NULL;
  for (size_t i = 0; i < candidates.list_len; i++) {
    cur_cand = candidates.list[i];
    if (cur_cand->ent == entity &&
        cur_cand->collider_offset == collider_offset) {
      size_t obj_offset = tree->data_buffer[cur_cand->next_offset].prev_offset;

      // Extract from node
      remove_from_list(tree, obj_offset);

      // Swap with end of the list
      size_t end_offset = tree->data_buff_len - 1;
      if (obj_offset != end_offset) {
        *cur_cand = tree->data_buffer[end_offset];
        remove_from_list(tree, end_offset);
        add_to_list(tree, obj_offset, cur_cand->node_offset);
      }

      // Delete node
      tree->data_buffer[end_offset].ent = NULL;
      tree->data_buffer[end_offset].collider_offset = INVALID_INDEX;
      tree->data_buffer[end_offset].node_offset = INVALID_INDEX;
      tree->data_buffer[end_offset].next_offset = INVALID_INDEX;
      tree->data_buffer[end_offset].prev_offset = INVALID_INDEX;
      (tree->data_buff_len)--;

      break;
    }
  }

  free(candidates.list);

  return 0;
}

COLLISION_RES_2D quad_tree_search(QUAD_TREE *tree, COLLIDER_2D *col) {
  COLLISION_RES_2D res = { NULL, 0, 0 };
  if (tree == NULL || col == NULL) {
    fprintf(stderr, "Error: Invalid quad-tree search input\n");
    return res;
  }

  res.list = malloc(sizeof(PHYS_OBJ_2D) * BUFF_STARTING_LEN);
  if (res.list == NULL) {
    fprintf(stderr, "Error: Unable to allocate quad-tree search list\n");
    return res;
  }
  res.list_buff_size = BUFF_STARTING_LEN;

  typedef struct search_node {
    vec2 min_extent;
    size_t offset;
    float quad_len;
    int depth;
  } SEARCH_NODE;
  SEARCH_NODE *search_stack = malloc(sizeof(SEARCH_NODE) * BUFF_STARTING_LEN);
  if (!search_stack) {
    res.list = NULL;
    res.list_len = 0;
    return res;
  }
  size_t top = 1;
  size_t stack_size = BUFF_STARTING_LEN;
  search_stack[0].offset = 0;
  search_stack[0].quad_len = tree->max_extent;
  search_stack[0].depth = 1;
  glm_vec2_copy((vec2) { -tree->max_extent, -tree->max_extent },
                search_stack[0].min_extent);

  float quad_len = 0.0;
  size_t cur_offset = 0;
  vec2 min_extent = GLM_VEC2_ZERO_INIT;
  vec2 max_extent = GLM_VEC2_ZERO_INIT;
  vec2 child_min = GLM_VEC2_ZERO_INIT;
  vec2 child_max = GLM_VEC2_ZERO_INIT;
  int quads = 0;
  int cur_quad = 0;
  int depth = 0;
  int status = 0;
  while (top) {
    top--;
    cur_offset = search_stack[top].offset;
    quad_len = search_stack[top].quad_len;
    depth = search_stack[top].depth;
    glm_vec2_copy(search_stack[top].min_extent, min_extent);
    glm_vec2_copy(search_stack[top].min_extent, max_extent);
    max_extent[X] += (2.0 * quad_len);
    max_extent[Y] += (2.0 * quad_len);

    status = read_quad(tree, tree->node_buffer + cur_offset, &res);
    if (status != 0) {
      return res;
    }

    // Lazily update the current node's empty flag
    pthread_mutex_lock(&tree->search_lock);
    update_node_emptiness(tree, cur_offset);
    if (depth != tree->max_depth &&
        tree->node_buffer[cur_offset].next_offset != INVALID_INDEX &&
        !tree->node_buffer[cur_offset].empty) {
      pthread_mutex_unlock(&tree->search_lock);
      quads = detect_quadrant(min_extent, max_extent, quad_len, col);
      while (quads) {
        glm_vec2_copy(min_extent, child_min);
        glm_vec2_copy(max_extent, child_max);
        cur_quad = get_lsb(quads);
        search_stack[top].quad_len = quad_len / 2.0;
        search_stack[top].depth = depth + 1;
        search_stack[top].offset = tree->node_buffer[cur_offset].next_offset +
                                   update_quad_extents(cur_quad, child_min,
                                                       child_max, quad_len);
        glm_vec2_copy(child_min, search_stack[top].min_extent);

        top++;
        if (top == stack_size) {
          int status = double_buffer((void **) &search_stack, &stack_size,
                                     sizeof(SEARCH_NODE));
          if (status) {
            free(search_stack);
            res.list = NULL;
            res.list_len = 0;
            return res;
          }
        }
        quads ^= cur_quad;
      }
    } else {
      pthread_mutex_unlock(&tree->search_lock);
    }
  }
  free(search_stack);

  return res;
}

size_t get_all_quad_colliders(QUAD_TREE *tree, PHYS_OBJ_2D **dest) {
  *dest = tree->data_buffer;
  return tree->data_buff_len;
}

int init_quad_node(QUAD_TREE *tree, QUAD_NODE *parent) {
  if (tree == NULL || parent == NULL) {
    fprintf(stderr, "Error: Invalid quad-node inputs\n");
    return -1;
  }

  parent->next_offset = tree->node_buff_len;

  for (size_t i = tree->node_buff_len; i < tree->node_buff_len + 4; i++) {
    tree->node_buffer[i].head_offset = INVALID_INDEX;
    tree->node_buffer[i].tail_offset = INVALID_INDEX;
    tree->node_buffer[i].empty = 1;
    tree->node_buffer[i].next_offset = INVALID_INDEX;
  }
  tree->node_buff_len += 4;

  if (tree->node_buff_len + 4 >= tree->node_buff_size) {
    return double_buffer((void **) &(tree->node_buffer), &tree->node_buff_size,
                         sizeof(QUAD_NODE));
  }

  return 0;
}

int read_quad(QUAD_TREE *tree, QUAD_NODE *node, COLLISION_RES_2D *res) {
  if (node->head_offset == INVALID_INDEX) {
    return 0;
  }

  int status = 0;
  size_t cur = node->head_offset;
  do {
    res->list[res->list_len] = tree->data_buffer + cur;
    res->list_len++;
    if (res->list_len == res->list_buff_size) {
      status = double_buffer((void **) &(res->list), &res->list_buff_size,
                             sizeof(PHYS_OBJ_2D *));
      if (status != 0) {
        fprintf(stderr,
                "Error: Unable to reallocate collision response list\n");
        return -1;
      }
    }

    cur = tree->data_buffer[cur].next_offset;
  } while (cur != node->head_offset);

  return 0;
}

int append_quad_buffer(QUAD_TREE *tree, size_t node_offset, ENTITY_2D *entity,
                       size_t collider_offset) {
  size_t buff_len = tree->data_buff_len;
  add_to_list(tree, buff_len, node_offset);

  tree->data_buffer[buff_len].node_offset = node_offset;
  tree->data_buffer[buff_len].collider_offset = collider_offset;
  tree->data_buffer[buff_len].ent = entity;

  (tree->data_buff_len)++;
  if (tree->data_buff_len == tree->data_buff_size) {
    return double_buffer((void **) &(tree->data_buffer), &tree->data_buff_size,
                         sizeof(PHYS_OBJ_2D));
  }

  return 0;
}

int add_to_list(QUAD_TREE *tree, size_t obj_offset, size_t node_offset) {
  PHYS_OBJ_2D *obj = tree->data_buffer + obj_offset;
  QUAD_NODE *node = tree->node_buffer + node_offset;
  if (node->head_offset == INVALID_INDEX) {
    node->head_offset = obj_offset;
    node->tail_offset = obj_offset;
    obj->prev_offset = obj_offset;
    obj->next_offset = obj_offset;
    return 0;
  }
  node->empty = 0;

  PHYS_OBJ_2D *tail = tree->data_buffer + node->tail_offset;
  PHYS_OBJ_2D *head = tree->data_buffer + node->head_offset;
  tail->next_offset = obj_offset;
  head->prev_offset = obj_offset;
  obj->next_offset = node->head_offset;
  obj->prev_offset = node->tail_offset;
  node->tail_offset = obj_offset;

  return 0;
}

int remove_from_list(QUAD_TREE *tree, size_t obj_offset) {
  PHYS_OBJ_2D *obj = tree->data_buffer + obj_offset;
  size_t node_offset = obj->node_offset;
  QUAD_NODE *node = tree->node_buffer + node_offset;

  if (node->head_offset == node->tail_offset) {
    obj->node_offset = INVALID_INDEX;
    node->head_offset = INVALID_INDEX;
    node->tail_offset = INVALID_INDEX;
    return 0;
  }
  // Lazily update the emptiness flag of the node. Parent nodes will be
  // updated on subseqent calls to quad_tree_search()
  update_node_emptiness(tree, node_offset);

  if (node->head_offset == obj_offset) {
    node->head_offset = obj->next_offset;
  } else if (node->tail_offset == obj_offset) {
    node->tail_offset = obj->prev_offset;
  }

  PHYS_OBJ_2D *next = tree->data_buffer + obj->next_offset;
  PHYS_OBJ_2D *prev = tree->data_buffer + obj->prev_offset;

  next->prev_offset = obj->prev_offset;
  prev->next_offset = obj->next_offset;

  return 0;
}

int detect_quadrant(vec2 min_extent, vec2 max_extent, float quad_len,
                    COLLIDER_2D *obj) {
  COLLIDER_2D quads[4];
  float hw = quad_len / 2.0;
  memset(quads, 0, sizeof(quads));
  //  X  Y
  quads[0].center[X] = max_extent[X] - hw;
  quads[0].center[Y] = max_extent[Y] - hw;
  //  X -Y
  quads[1].center[X] = max_extent[X] - hw;
  quads[1].center[Y] = min_extent[Y] + hw;
  // -X  Y
  quads[2].center[X] = min_extent[X] + hw;
  quads[2].center[Y] = max_extent[Y] - hw;
  // -X -Y
  quads[3].center[X] = min_extent[X] + hw;
  quads[3].center[Y] = min_extent[Y] + hw;
  for (int i = 0; i < 4; i++) {
    quads[i].data.width = quad_len;
    quads[i].data.height = quad_len;
    quads[i].type = SQUARE;
  }

  int ret = 0;
  vec2 temp = { 0.0, 0.0 };
  for (int i = 0; i < 4; i++) {
    if (collision_check_2d(quads + i, obj, temp)) {
      ret |= (0x1 << i);
    }
  }

  return ret;
}

size_t update_quad_extents(int quad, vec2 min_extent, vec2 max_extent,
                           float quad_len) {
  if (quad == QUAD_X_Y) {
    min_extent[X] += quad_len;
    min_extent[Y] += quad_len;
    return X_Y;
  } else if (quad == QUAD_X_negY) {
    min_extent[X] += quad_len;
    max_extent[Y] -= quad_len;
    return X_negY;
  } else if (quad == QUAD_negX_Y) {
    max_extent[X] -= quad_len;
    min_extent[Y] += quad_len;
    return negX_Y;
  } else {
    max_extent[X] -= quad_len;
    max_extent[Y] -= quad_len;
    return negX_negY;
  }
}

void update_node_emptiness(QUAD_TREE *tree, size_t node_offset) {
  QUAD_NODE *cur = tree->node_buffer + node_offset;
  if (cur->head_offset != INVALID_INDEX) {
    cur->empty = 0;
    return;
  }

  cur->empty = 1;
  if (cur->next_offset == INVALID_INDEX) {
    return;
  }

  // Check the empty status of all children. Set empty = 1 if all children
  // are empty
  QUAD_NODE *children = tree->node_buffer + cur->next_offset;
  for (int i = 0; i < 4; i++) {
    if (!children[i].empty) {
      cur->empty = 0;
      return;
    }
  }
}

vec3 quad_translate[4] = {
                       {  1.0,  1.0 }, //  X,  Y
                       {  1.0, -1.0 }, //  X, -Y
                       { -1.0,  1.0 }, // -X,  Y
                       { -1.0, -1.0 }, // -X, -Y
                      };
void draw_quad_tree(QUAD_TREE *tree, vec2 pos, float scale,
                   unsigned int shader, size_t offset, int depth) {
  if (tree->node_buffer[offset].head_offset == INVALID_INDEX &&
      tree->node_buffer[offset].tail_offset == INVALID_INDEX) {
    set_vec3("col", (vec3) { 1.0, 1.0, 1.0 }, shader);
  } else {
    set_vec3("col", (vec3) { 0.0, 1.0, 1.0 }, shader);
  }

  draw_square(pos, scale, scale);

  vec2 temp = { 0.0, 0.0 };
  if (!tree->node_buffer[offset].empty &&
      tree->node_buffer[offset].next_offset != INVALID_INDEX && depth < 5) {
    for (int i = 0; i < 4; i++) {
      glm_vec2_scale(quad_translate[i], scale / 2.0, temp);
      glm_vec2_add(pos, temp, temp);
      draw_quad_tree(tree, temp, scale / 2.0, shader,
                     tree->node_buffer[offset].next_offset + i, depth + 1);
    }
  }
}

