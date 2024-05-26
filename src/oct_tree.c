#include <oct_tree.h>

// ======================= INITIALIZATION AND CLEANUP ========================

OCT_TREE *init_tree(float max_extent, unsigned int max_depth) {
  if (max_extent <= 0.0) {
    return NULL;
  }

  OCT_TREE *tree = malloc(sizeof(OCT_TREE));
  if (tree == NULL) {
    fprintf(stderr, "Error: Unable to allocate oct-tree\n");
    return NULL;
  }

  if (pthread_mutex_init(&tree->search_lock, NULL)) {
    fprintf(stderr, "Error: Unable to initailize oct tree mutex\n");
    free(tree);
    return NULL;
  }

  tree->node_buffer = malloc(sizeof(OCT_NODE) * OCT_TREE_STARTING_LEN);
  if (tree->node_buffer == NULL) {
    fprintf(stderr, "Error: Unable to allocate oct nodes\n");
    pthread_mutex_destroy(&tree->search_lock);
    free(tree);
    return NULL;
  }

  tree->data_buffer = malloc(sizeof(PHYS_OBJ) * BUFF_STARTING_LEN);
  if (tree->data_buffer == NULL) {
    fprintf(stderr, "Error: Unable to allocate node data\n");
    free(tree->node_buffer);
    pthread_mutex_destroy(&tree->search_lock);
    free(tree);
    return NULL;
  }

  tree->node_buff_len = 1;
  tree->data_buff_len = 0;
  tree->node_buff_size = OCT_TREE_STARTING_LEN;
  tree->data_buff_size = BUFF_STARTING_LEN;

  tree->node_buffer[0].empty = 1;
  tree->node_buffer[0].head_offset = INVALID_INDEX;
  tree->node_buffer[0].tail_offset = INVALID_INDEX;
  tree->node_buffer[0].next_offset = INVALID_INDEX;

  tree->max_extent = max_extent;
  tree->max_depth = max_depth;

  return tree;
}

void free_oct_tree(OCT_TREE *tree) {
  pthread_mutex_destroy(&tree->search_lock);
  free(tree->data_buffer);
  free(tree->node_buffer);
  free(tree);
}

// =========================== OCT TREE OPERATIONS ===========================

#ifdef DEBUG_OCT_TREE
int oct_tree_insert(OCT_TREE *tree, ENTITY *entity, size_t collider_offset,
                    int birthmark) {
#else
int oct_tree_insert(OCT_TREE *tree, ENTITY *entity, size_t collider_offset) {
#endif
  if (tree == NULL || entity == NULL ||
      collider_offset >= entity->model->num_colliders) {
    fprintf(stderr, "Error: Invalid oct-tree insertion input\n");
    return -1;
  }

  vec3 max_extent = { tree->max_extent, tree->max_extent, tree->max_extent };
  vec3 min_extent = { -tree->max_extent, -tree->max_extent,
                      -tree->max_extent };
  float max_extents[6];

  COLLIDER obj;
  memset(&obj, 0, sizeof(COLLIDER));
  global_collider(entity, collider_offset, &obj);

  if (obj.type == POLY) {
    vec3 *verts = obj.data.verts;
    unsigned int num_used = obj.data.num_used;
    max_extents[0] = obj.data.verts[max_dot(verts, num_used, X_DIR)][0];
    max_extents[1] = obj.data.verts[max_dot(verts, num_used, NEG_X_DIR)][0];
    max_extents[2] = obj.data.verts[max_dot(verts, num_used, Y_DIR)][1];
    max_extents[3] = obj.data.verts[max_dot(verts, num_used, NEG_Y_DIR)][1];
    max_extents[4] = obj.data.verts[max_dot(verts, num_used, Z_DIR)][2];
    max_extents[5] = obj.data.verts[max_dot(verts, num_used, NEG_Z_DIR)][2];
  } else {
    float radius = obj.data.radius;

    max_extents[0] = obj.data.center[0] + radius;
    max_extents[1] = obj.data.center[0] - radius;
    max_extents[2] = obj.data.center[1] + radius;
    max_extents[3] = obj.data.center[1] - radius;
    max_extents[4] = obj.data.center[2] + radius;
    max_extents[5] = obj.data.center[2] - radius;
  }

  float oct_len = tree->max_extent;

  int cur_oct = 0;
  int lsb = 0;
  size_t cur_offset = 0;

  int status = 0;
  int depth = 1;
  int inserting = 1;
  while (inserting) {
    tree->node_buffer[cur_offset].empty = 0;
    if (depth == tree->max_depth) {
      inserting = 0;
#ifdef DEBUG_OCT_TREE
      status = append_buffer(tree, cur_offset, entity, collider_offset,
                             birthmark, obj);
#else
      status = append_buffer(tree, cur_offset, entity, collider_offset);
#endif
      if (status != 0) {
        printf("Unable to allocate tree data\n");
        return -1;
      }
    } else {
      cur_oct = detect_octant(min_extent, max_extent, max_extents, oct_len);
      lsb = get_lsb(cur_oct);
      // If multiple bits are set, object collides with multiple octants
      if (!cur_oct || cur_oct ^ lsb ) {
        inserting = 0;
#ifdef DEBUG_OCT_TREE
        status = append_buffer(tree, cur_offset, entity, collider_offset,
                               birthmark, obj);
#else
        status = append_buffer(tree, cur_offset, entity, collider_offset);
#endif
        if (status != 0) {
          printf("Unable to allocate tree data\n");
          return -1;
        }
      } else {
        if (tree->node_buffer[cur_offset].next_offset == INVALID_INDEX) {
          status = init_node(tree, tree->node_buffer + cur_offset);
          if (status != 0) {
            printf("Unable to allocate node children\n");
            return -1;
          }
        }

        cur_offset = tree->node_buffer[cur_offset].next_offset +
                     update_extents(cur_oct, min_extent, max_extent,
                                    oct_len);
        oct_len /= 2.0;
        depth++;
      }
    }
  }

  return 0;
}

int oct_tree_delete(OCT_TREE *tree, ENTITY *entity, size_t collider_offset) {
  if (tree == NULL || entity == NULL ||
      collider_offset >= entity->model->num_colliders) {
    fprintf(stderr, "Error: Invalid oct-tree deletion input\n");
    return -1;
  }

  COLLIDER obj;
  memset(&obj, 0, sizeof(COLLIDER));
  global_collider(entity, collider_offset, &obj);

  COLLISION_RES candidates = oct_tree_search(tree, &obj);
  if (candidates.list == NULL) {
    return -1;
  }

  PHYS_OBJ *cur_cand = NULL;
  for (size_t i = 0; i < candidates.list_len; i++) {
    cur_cand = candidates.list[i];
    if (cur_cand->entity == entity &&
        cur_cand->collider_offset == collider_offset) {
      size_t obj_offset = tree->data_buffer[cur_cand->next_offset].prev_offset;

      // Extract from node
      remove_from_list(tree, obj_offset);

      // Swap with end of the list
      size_t end_offset = tree->data_buff_len - 1;
      if (obj_offset != end_offset) {
        cur_cand->entity = tree->data_buffer[end_offset].entity;
        cur_cand->collider_offset = tree->data_buffer[end_offset].
                                    collider_offset;
        cur_cand->node_offset = tree->data_buffer[end_offset].node_offset;

        remove_from_list(tree, end_offset);
        add_to_list(tree, obj_offset, cur_cand->node_offset);
      }

      // Delete node
      tree->data_buffer[end_offset].entity = NULL;
      tree->data_buffer[end_offset].collider_offset = INVALID_INDEX;
      tree->data_buffer[end_offset].node_offset = INVALID_INDEX;
      tree->data_buffer[end_offset].next_offset = INVALID_INDEX;
      tree->data_buffer[end_offset].prev_offset = INVALID_INDEX;
      (tree->data_buff_len)--;

      break;
    }
  }

#ifdef DEBUG_OCT_TREE
  for (size_t i = 0; i < tree->data_buff_len; i++) {
    if (tree->data_buffer[i].entity == entity &&
        tree->data_buffer[i].collider_offset == collider_offset) {
      fprintf(stderr, "Error: Oct tree deletion failure\n");
    }
  }
#endif

  free(candidates.list);

  return 0;
}

COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *col) {
  COLLISION_RES res = { NULL, 0, 0 };
  if (tree == NULL || col == NULL) {
    fprintf(stderr, "Error: Invalid oct-tree search input\n");
    return res;
  }

  res.list = malloc(sizeof(PHYS_OBJ) * BUFF_STARTING_LEN);
  if (res.list == NULL) {
    fprintf(stderr, "Error: Unable to allocate oct-tree search list\n");
    return res;
  }
  res.list_buff_size = BUFF_STARTING_LEN;

  float max_extents[6];
  if (col->type == POLY) {
    vec3 *verts = col->data.verts;
    unsigned int num_used = col->data.num_used;
    max_extents[0] = col->data.verts[max_dot(verts, num_used, X_DIR)][0];
    max_extents[1] = col->data.verts[max_dot(verts, num_used, NEG_X_DIR)][0];
    max_extents[2] = col->data.verts[max_dot(verts, num_used, Y_DIR)][1];
    max_extents[3] = col->data.verts[max_dot(verts, num_used, NEG_Y_DIR)][1];
    max_extents[4] = col->data.verts[max_dot(verts, num_used, Z_DIR)][2];
    max_extents[5] = col->data.verts[max_dot(verts, num_used, NEG_Z_DIR)][2];
  } else {
    vec3 center = { 0.0, 0.0, 0.0 };
    glm_vec3_copy(col->data.center, center);
    float radius = col->data.radius;

    max_extents[0] = center[0] + radius;
    max_extents[1] = center[0] - radius;
    max_extents[2] = center[1] + radius;
    max_extents[3] = center[1] - radius;
    max_extents[4] = center[2] + radius;
    max_extents[5] = center[2] - radius;
  }

  typedef struct search_node {
    vec3 min_extent;
    size_t offset;
    float oct_len;
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
  search_stack[0].oct_len = tree->max_extent;
  search_stack[0].depth = 1;
  glm_vec3_copy((vec3) { -tree->max_extent, -tree->max_extent,
                         -tree->max_extent }, search_stack[0].min_extent);

  float oct_len = 0.0;
  size_t cur_offset = 0;
  vec3 min_extent = GLM_VEC3_ZERO_INIT;
  vec3 max_extent = GLM_VEC3_ZERO_INIT;
  vec3 child_min = GLM_VEC3_ZERO_INIT;
  vec3 child_max = GLM_VEC3_ZERO_INIT;
  int octs = 0;
  int cur_oct = 0;
  int depth = 0;
  int status = 0;
  while (top) {
    top--;
    cur_offset = search_stack[top].offset;
    oct_len = search_stack[top].oct_len;
    depth = search_stack[top].depth;
    glm_vec3_copy(search_stack[top].min_extent, min_extent);
    glm_vec3_copy(search_stack[top].min_extent, max_extent);
    max_extent[X] += (2.0 * oct_len);
    max_extent[Y] += (2.0 * oct_len);
    max_extent[Z] += (2.0 * oct_len);

    status = read_oct(tree, tree->node_buffer + cur_offset, &res);
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
      octs = detect_octant(min_extent, max_extent, max_extents, oct_len);
      while (octs) {
        glm_vec3_copy(min_extent, child_min);
        glm_vec3_copy(max_extent, child_max);
        cur_oct = get_lsb(octs);
        search_stack[top].oct_len = oct_len / 2.0;
        search_stack[top].depth = depth + 1;
        search_stack[top].offset = tree->node_buffer[cur_offset].next_offset +
                                   update_extents(cur_oct, child_min,
                                                  child_max, oct_len);
        glm_vec3_copy(child_min, search_stack[top].min_extent);

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
        octs ^= cur_oct;
      }
    } else {
      pthread_mutex_unlock(&tree->search_lock);
    }
  }
  free(search_stack);

  return res;
}

// ============================= MISC OPERATIONS ==--=========================

size_t get_all_colliders(OCT_TREE *tree, PHYS_OBJ **dest) {
  *dest = tree->data_buffer;
  return tree->data_buff_len;
}

// ================================= HELPERS =================================

int init_node(OCT_TREE *tree, OCT_NODE *parent) {
  if (tree == NULL || parent == NULL) {
    fprintf(stderr, "Error: Invalid oct-node inputs\n");
    return -1;
  }

  parent->next_offset = tree->node_buff_len;

  for (size_t i = tree->node_buff_len; i < tree->node_buff_len + 8; i++) {
    tree->node_buffer[i].head_offset = INVALID_INDEX;
    tree->node_buffer[i].tail_offset = INVALID_INDEX;
    tree->node_buffer[i].empty = 1;
    tree->node_buffer[i].next_offset = INVALID_INDEX;
  }
  tree->node_buff_len += 8;

  if (tree->node_buff_len + 8 >= tree->node_buff_size) {
    return double_buffer((void **) &(tree->node_buffer), &tree->node_buff_size,
                         sizeof(OCT_NODE));
  }

  return 0;
}

int read_oct(OCT_TREE *tree, OCT_NODE *node, COLLISION_RES *res) {
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
                             sizeof(PHYS_OBJ *));
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

#ifdef DEBUG_OCT_TREE
int append_buffer(OCT_TREE *tree, size_t node_offset, ENTITY *entity,
                  size_t collider_offset, int birthmark, COLLIDER col) {
#else
int append_buffer(OCT_TREE *tree, size_t node_offset, ENTITY *entity,
                  size_t collider_offset) {
#endif
  size_t buff_len = tree->data_buff_len;
  add_to_list(tree, buff_len, node_offset);

  tree->data_buffer[buff_len].node_offset = node_offset;
  tree->data_buffer[buff_len].collider_offset = collider_offset;
  tree->data_buffer[buff_len].entity = entity;
#ifdef DEBUG_OCT_TREE
  tree->data_buffer[buff_len].birthmark = birthmark;
  tree->data_buffer[buff_len].add_state = col;
#endif

  (tree->data_buff_len)++;
  if (tree->data_buff_len == tree->data_buff_size) {
    return double_buffer((void **) &(tree->data_buffer), &tree->data_buff_size,
                         sizeof(PHYS_OBJ));
  }

  return 0;
}

int add_to_list(OCT_TREE *tree, size_t obj_offset, size_t node_offset) {
  PHYS_OBJ *obj = tree->data_buffer + obj_offset;
  OCT_NODE *node = tree->node_buffer + node_offset;
  if (node->head_offset == INVALID_INDEX) {
    node->head_offset = obj_offset;
    node->tail_offset = obj_offset;
    obj->prev_offset = obj_offset;
    obj->next_offset = obj_offset;
    return 0;
  }
  node->empty = 0;

  PHYS_OBJ *tail = tree->data_buffer + node->tail_offset;
  PHYS_OBJ *head = tree->data_buffer + node->head_offset;
  tail->next_offset = obj_offset;
  head->prev_offset = obj_offset;
  obj->next_offset = node->head_offset;
  obj->prev_offset = node->tail_offset;
  node->tail_offset = obj_offset;

  return 0;
}

int remove_from_list(OCT_TREE *tree, size_t obj_offset) {
  PHYS_OBJ *obj = tree->data_buffer + obj_offset;
  size_t node_offset = obj->node_offset;
  OCT_NODE *node = tree->node_buffer + node_offset;

  if (node->head_offset == node->tail_offset) {
    obj->node_offset = INVALID_INDEX;
    node->head_offset = INVALID_INDEX;
    node->tail_offset = INVALID_INDEX;
    return 0;
  }
  // Lazily update the emptiness flag of the node. Parent nodes will be
  // updated on subseqent calls to oct_tree_search()
  update_node_emptiness(tree, node_offset);

  if (node->head_offset == obj_offset) {
    node->head_offset = obj->next_offset;
  } else if (node->tail_offset == obj_offset) {
    node->tail_offset = obj->prev_offset;
  }

  PHYS_OBJ *next = tree->data_buffer + obj->next_offset;
  PHYS_OBJ *prev = tree->data_buffer + obj->prev_offset;

  next->prev_offset = obj->prev_offset;
  prev->next_offset = obj->next_offset;

  return 0;
}

int detect_octant(vec3 min_extent, vec3 max_extent, float *obj_extents,
                   float oct_len) {
  int ret = 0;

  //  X,  Y,  Z
  if (obj_extents[0] >= min_extent[X] + oct_len &&
      obj_extents[1] <= max_extent[X] &&
      obj_extents[2] >= min_extent[Y] + oct_len &&
      obj_extents[3] <= max_extent[Y] &&
      obj_extents[4] >= min_extent[Z] + oct_len &&
      obj_extents[5] <= max_extent[Z]) {
    ret |= OCT_X_Y_Z;
  }
  //  X,  Y, -Z
  if (obj_extents[0] >= min_extent[X] + oct_len &&
      obj_extents[1] <= max_extent[X] &&
      obj_extents[2] >= min_extent[Y] + oct_len &&
      obj_extents[3] <= max_extent[Y] &&
      obj_extents[4] >= min_extent[Z] &&
      obj_extents[5] <= max_extent[Z] - oct_len) {
    ret |= OCT_X_Y_negZ;
  }
  //  X, -Y,  Z
  if (obj_extents[0] >= min_extent[X] + oct_len &&
      obj_extents[1] <= max_extent[X] &&
      obj_extents[2] >= min_extent[Y] &&
      obj_extents[3] <= max_extent[Y] - oct_len &&
      obj_extents[4] >= min_extent[Z] + oct_len &&
      obj_extents[5] <= max_extent[Z]) {
    ret |= OCT_X_negY_Z;
  }
  //  X, -Y, -Z
  if (obj_extents[0] >= min_extent[X] + oct_len &&
      obj_extents[1] <= max_extent[X] &&
      obj_extents[2] >= min_extent[Y] &&
      obj_extents[3] <= max_extent[Y] - oct_len &&
      obj_extents[4] >= min_extent[Z] &&
      obj_extents[5] <= max_extent[Z] - oct_len) {
    ret |= OCT_X_negY_negZ;
  }
  // -X,  Y,  Z
  if (obj_extents[0] >= min_extent[X] &&
      obj_extents[1] <= max_extent[X] - oct_len &&
      obj_extents[2] >= min_extent[Y] + oct_len &&
      obj_extents[3] <= max_extent[Y] &&
      obj_extents[4] >= min_extent[Z] + oct_len &&
      obj_extents[5] <= max_extent[Z]) {
    ret |= OCT_negX_Y_Z;
  }
  // -X,  Y, -Z
  if (obj_extents[0] >= min_extent[X] &&
      obj_extents[1] <= max_extent[X] - oct_len &&
      obj_extents[2] >= min_extent[Y] + oct_len &&
      obj_extents[3] <= max_extent[Y] &&
      obj_extents[4] >= min_extent[Z] &&
      obj_extents[5] <= max_extent[Z] - oct_len) {
    ret |= OCT_negX_Y_negZ;
  }
  // -X, -Y,  Z
  if (obj_extents[0] >= min_extent[X] &&
      obj_extents[1] <= max_extent[X] - oct_len &&
      obj_extents[2] >= min_extent[Y] &&
      obj_extents[3] <= max_extent[Y] - oct_len &&
      obj_extents[4] >= min_extent[Z] + oct_len &&
      obj_extents[5] <= max_extent[Z]) {
    ret |= OCT_negX_negY_Z;
  }
  // -X, -Y, -Z
  if (obj_extents[0] >= min_extent[X] &&
      obj_extents[1] <= max_extent[X] - oct_len &&
      obj_extents[2] >= min_extent[Y] &&
      obj_extents[3] <= max_extent[Y] - oct_len &&
      obj_extents[4] >= min_extent[Z] &&
      obj_extents[5] <= max_extent[Z] - oct_len) {
    ret |= OCT_negX_negY_negZ;
  }

  return ret;
}

size_t update_extents(int oct, vec3 min_extent, vec3 max_extent,
                      float oct_len) {
  if (oct == OCT_X_Y_Z) {
    min_extent[X] += oct_len;
    min_extent[Y] += oct_len;
    min_extent[Z] += oct_len;
    return X_Y_Z;
  } else if (oct == OCT_X_Y_negZ) {
    min_extent[X] += oct_len;
    min_extent[Y] += oct_len;
    max_extent[Z] -= oct_len;
    return X_Y_negZ;
  } else if (oct == OCT_X_negY_Z) {
    min_extent[X] += oct_len;
    max_extent[Y] -= oct_len;
    min_extent[Z] += oct_len;
    return X_negY_Z;
  } else if (oct == OCT_X_negY_negZ) {
    min_extent[X] += oct_len;
    max_extent[Y] -= oct_len;
    max_extent[Z] -= oct_len;
    return X_negY_negZ;
  } else if (oct == OCT_negX_Y_Z) {
    max_extent[X] -= oct_len;
    min_extent[Y] += oct_len;
    min_extent[Z] += oct_len;
    return negX_Y_Z;
  } else if (oct == OCT_negX_Y_negZ) {
    max_extent[X] -= oct_len;
    min_extent[Y] += oct_len;
    max_extent[Z] -= oct_len;
    return negX_Y_negZ;
  } else if (oct == OCT_negX_negY_Z) {
    max_extent[X] -= oct_len;
    max_extent[Y] -= oct_len;
    min_extent[Z] += oct_len;
    return negX_negY_Z;
  } else {
    max_extent[X] -= oct_len;
    max_extent[Y] -= oct_len;
    max_extent[Z] -= oct_len;
    return negX_negY_negZ;
  }
}

void update_node_emptiness(OCT_TREE *tree, size_t node_offset) {
  OCT_NODE *cur = tree->node_buffer + node_offset;
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
  OCT_NODE *children = tree->node_buffer + cur->next_offset;
  for (int i = 0; i < 8; i++) {
    if (!children[i].empty) {
      cur->empty = 0;
      return;
    }
  }
}

vec3 quad_translate[8] = {
                       { 1.0, 1.0, 1.0 }, //  X, Y, Z
                       { 1.0, 1.0, -1.0 }, //  X, Y,-Z
                       { 1.0, -1.0, 1.0 }, //  X,-Y, Z
                       { 1.0, -1.0, -1.0 }, //  X,-Y,-Z
                       { -1.0, 1.0, 1.0 }, // -X, Y, Z
                       { -1.0, 1.0, -1.0 }, // -X, Y,-Z
                       { -1.0, -1.0, 1.0 }, // -X,-Y, Z
                       { -1.0, -1.0, -1.0 }  // -X,-Y,-Z
                      };
void draw_oct_tree(MODEL *cube, OCT_TREE *tree, vec3 pos, float scale,
                   unsigned int shader, size_t offset, int depth) {
  if (tree->node_buffer[offset].head_offset == INVALID_INDEX &&
      tree->node_buffer[offset].tail_offset == INVALID_INDEX) {
    glUniform3f(glGetUniformLocation(shader, "col"), 1.0, 1.0, 0.0);
  } else {
    glUniform3f(glGetUniformLocation(shader, "col"), 0.0, 1.0, 1.0);
  }
  mat4 model = GLM_MAT4_IDENTITY_INIT;
  glm_translate(model, pos);
  glm_scale_uni(model, scale);
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1,
                     GL_FALSE, (float *) model);
  draw_model(shader, cube);

  vec3 temp = { 0.0, 0.0, 0.0 };
  if (!tree->node_buffer[offset].empty &&
      tree->node_buffer[offset].next_offset != INVALID_INDEX && depth < 5) {
    for (int i = 0; i < 8; i++) {
      glm_vec3_scale(quad_translate[i], scale / 2.0, temp);
      glm_vec3_add(pos, temp, temp);
      draw_oct_tree(cube, tree, temp, scale / 2.0, shader,
                    tree->node_buffer[offset].next_offset + i, depth + 1);
    }
  }
}

