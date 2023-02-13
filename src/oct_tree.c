#include <oct_tree.h>

OCT_TREE *init_tree() {
  OCT_TREE *tree = malloc(sizeof(OCT_TREE));
  if (tree == NULL) {
    printf("Unable to allocate oct-tree\n");
    return NULL;
  }

  tree->node_buffer = malloc(sizeof(OCT_NODE) * OCT_TREE_STARTING_LEN);
  if (tree->node_buffer == NULL) {
    printf("Unable to allocate oct nodes\n");
    free(tree);
    return NULL;
  }

  tree->data_buffer = malloc(sizeof(PHYS_OBJ) * BUFF_STARTING_LEN);
  if (tree->data_buffer == NULL) {
    printf("Unable to allocate node data\n");
    free(tree->node_buffer);
    free(tree);
    return NULL;
  }

  tree->node_buff_len = 1;
  tree->data_buff_len = 0;
  tree->node_buff_size = OCT_TREE_STARTING_LEN;
  tree->data_buff_size = BUFF_STARTING_LEN;

  tree->node_buffer[0].empty = 1;
  tree->node_buffer[0].next_offset = -1;

  return tree;
}

int oct_tree_insert(OCT_TREE *tree, ENTITY *entity, size_t collider_offset) {
  if (tree == NULL || entity == NULL ||
      collider_offset >= entity->model->num_colliders) {
    printf("Invalid insertion input\n");
    return -1;
  }

  vec3 max_extent = { 16.0, 16.0, 16.0 };
  vec3 min_extent = { -16.0, -16.0, -16.0 };
  float max_extents[6];

  COLLIDER obj;
  obj.data = entity->model->colliders[collider_offset].data;
  obj.type = entity->model->colliders[collider_offset].type;
  int bone = entity->model->collider_bone_links[collider_offset];
  if (obj.type == POLY) {
    for (unsigned int i = 0; i < obj.data.num_used; i++) {
      if (bone >= 0 && bone < entity->model->num_colliders) {
        glm_mat4_mulv3(entity->final_b_mats[bone],
                       entity->model->colliders[collider_offset].data.verts[i],
                       1.0, obj.data.verts[i]);
      }
      glm_mat4_mulv3(entity->model_mat, obj.data.verts[i], 1.0,
                     obj.data.verts[i]);
    }

    max_extents[0] = obj.data.verts[max_dot(&obj, X)][0];
    max_extents[1] = obj.data.verts[max_dot(&obj, NEG_X)][0];
    max_extents[2] = obj.data.verts[max_dot(&obj, Y)][1];
    max_extents[3] = obj.data.verts[max_dot(&obj, NEG_Y)][1];
    max_extents[4] = obj.data.verts[max_dot(&obj, Z)][2];
    max_extents[5] = obj.data.verts[max_dot(&obj, NEG_Z)][2];
  } else {
    if (bone >= 0 && bone < entity->model->num_colliders) {
      glm_mat4_mulv3(entity->final_b_mats[bone],
                     entity->model->colliders[collider_offset].data.center,
                     1.0, obj.data.center);
    }
    glm_mat4_mulv3(entity->model_mat, obj.data.center, 1.0, obj.data.center);
    float radius = obj.data.radius;

    max_extents[0] = obj.data.center[0] + radius;
    max_extents[1] = obj.data.center[0] - radius;
    max_extents[2] = obj.data.center[1] + radius;
    max_extents[3] = obj.data.center[1] - radius;
    max_extents[4] = obj.data.center[2] + radius;
    max_extents[5] = obj.data.center[2] - radius;
  }

  float oct_len = 16.0;

  OCTANT cur_oct = MULTIPLE;
  size_t cur_offset = 0;

  int status = 0;
  int depth = 1;
  int inserting = 1;
  while (inserting) {
    if (depth == MAX_DEPTH) {
      inserting = 0;
      status = append_buffer(tree, cur_offset, entity, collider_offset);
      if (status != 0) {
        printf("Unable to allocate tree data\n");
        return -1;
      }
    } else {
      cur_oct = detect_octant(min_extent, max_extent, max_extents, &oct_len);
      if (cur_oct == MULTIPLE) {
        inserting = 0;
        status = append_buffer(tree, cur_offset, entity, collider_offset);
        if (status != 0) {
          printf("Unable to allocate tree data\n");
          return -1;
        }
      } else {
        if (tree->node_buffer[cur_offset].next_offset == -1) {
          status = init_node(tree, tree->node_buffer + cur_offset);
          if (status != 0) {
            printf("Unable to allocate node children\n");
            return -1;
          }
        }

        cur_offset = tree->node_buffer[cur_offset].next_offset + cur_oct;
        depth++;
      }
    }
  }

  return 0;
}

int oct_tree_delete(OCT_TREE *tree, size_t obj_offset) {
  if (tree == NULL || obj_offset > tree->data_buff_len) {
    printf("Invalid deletion input\n");
    return -1;
  }

  OCT_NODE *node = tree->node_buffer +
                   tree->data_buffer[obj_offset].node_offset;
  if (node->empty) {
    return 0;
  }

  PHYS_OBJ *obj = tree->data_buffer + obj_offset;

  // Extract from node
  remove_from_list(tree, obj_offset);
  obj->entity->tree_offsets[obj->collider_offset] = INVALID;

  // Swap with end of the list
  size_t end_offset = tree->data_buff_len - 1;
  if (obj_offset != end_offset) {
    obj->entity = tree->data_buffer[end_offset].entity;
    obj->collider_offset = tree->data_buffer[end_offset].collider_offset;
    obj->node_offset = tree->data_buffer[end_offset].node_offset;

    obj->entity->tree_offsets[obj->collider_offset] = obj_offset;

    remove_from_list(tree, end_offset);
    add_to_list(tree, obj_offset, obj->node_offset);
  }

  // Delete node
  tree->data_buffer[end_offset].entity = NULL;
  tree->data_buffer[end_offset].collider_offset = INVALID;
  tree->data_buffer[end_offset].node_offset = INVALID;
  tree->data_buffer[end_offset].next_offset = INVALID;
  tree->data_buffer[end_offset].prev_offset = INVALID;
  (tree->data_buff_len)--;

  return 0;
}

COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *col) {
  COLLISION_RES res = { NULL, 0, 0 };
  if (tree == NULL || col == NULL) {
    printf("Invalid search input\n");
    return res;
  }

  res.list = malloc(sizeof(PHYS_OBJ) * BUFF_STARTING_LEN);
  if (res.list == NULL) {
    printf("Unable to allocate search list\n");
    return res;
  }
  res.list_buff_size = BUFF_STARTING_LEN;

  vec3 max_extent = { 16.0, 16.0, 16.0 };
  vec3 min_extent = { -16.0, -16.0, -16.0 };

  float max_extents[6];
  if (col->type == POLY) {
    max_extents[0] = col->data.verts[max_dot(col, X)][0];
    max_extents[1] = col->data.verts[max_dot(col, NEG_X)][0];
    max_extents[2] = col->data.verts[max_dot(col, Y)][1];
    max_extents[3] = col->data.verts[max_dot(col, NEG_Y)][1];
    max_extents[4] = col->data.verts[max_dot(col, Z)][2];
    max_extents[5] = col->data.verts[max_dot(col, NEG_Z)][2];
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
  float oct_len = 16.0;

  OCTANT cur_oct = MULTIPLE;
  size_t cur_offset = 0;

  int status = 0;
  int depth = 1;
  int searching = 1;
  while (searching) {
    status = read_oct(tree, tree->node_buffer + cur_offset, &res);
    if (status != 0) {
      return res;
    }

    if (depth == MAX_DEPTH ||
        tree->node_buffer[cur_offset].next_offset == -1) {
      searching = 0;
    } else {
      cur_oct = detect_octant(min_extent, max_extent, max_extents, &oct_len);
      if (cur_oct == MULTIPLE) {
        searching = 0;
        status = read_all_children(tree, tree->node_buffer + cur_offset, &res);
        if (status != 0) {
          return res;
        }
      } else {
        cur_offset = tree->node_buffer[cur_offset].next_offset + cur_oct;
        depth++;
      }
    }
  }

  return res;
}

void free_oct_tree(OCT_TREE *tree) {
  free(tree->data_buffer);
  free(tree->node_buffer);
  free(tree);
}

int init_node(OCT_TREE *tree, OCT_NODE *parent) {
  if (tree == NULL || parent == NULL) {
    printf("Invalid node inputs\n");
    return -1;
  }

  parent->next_offset = tree->node_buff_len;

  for (size_t i = tree->node_buff_len; i < tree->node_buff_len + 8; i++) {
    tree->node_buffer[i].head_offset = INVALID;
    tree->node_buffer[i].tail_offset = INVALID;
    tree->node_buffer[i].empty = 1;
    tree->node_buffer[i].next_offset = -1;
  }
  tree->node_buff_len += 8;

  if (tree->node_buff_len + 8 >= tree->node_buff_size) {
    return double_buffer((void **) &(tree->node_buffer), &tree->node_buff_size,
                         sizeof(OCT_NODE));
  }

  return 0;
}

int read_oct(OCT_TREE *tree, OCT_NODE *node, COLLISION_RES *res) {
  if (node->empty) {
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
        printf("Unable to reallocate collision response list\n");
        return -1;
      }
    }

    cur = tree->data_buffer[cur].next_offset;
  } while (cur != node->head_offset);

  return 0;
}

int read_all_children(OCT_TREE *tree, OCT_NODE *node, COLLISION_RES *res) {
  OCT_NODE **stack = malloc(sizeof(OCT_NODE *) * BUFF_STARTING_LEN);
  if (stack == NULL) {
    return -1;
  }

  size_t top = 0;
  size_t stack_size = BUFF_STARTING_LEN;
  int status = -1;
  for (OCTANT i = X_Y_Z; i < negX_negY_negZ; i++) {
    stack[top] = tree->node_buffer + (node->next_offset + i);
    top++;
    if (top == stack_size) {
      status = double_buffer((void **) &stack, &stack_size,
                             sizeof(OCT_NODE *));
      if (status != 0) {
        free(stack);
        return -1;
      }
    }
  }

  OCT_NODE *cur = NULL;
  while (top) {
    cur = stack[top - 1];
    status = read_oct(tree, cur, res);
    if (status != 0) {
      free(stack);
      return -1;
    }
    top--;

    if (cur->next_offset != -1) {
      for (OCTANT i = X_Y_Z; i< negX_negY_negZ; i++) {
        stack[top] = tree->node_buffer + (cur->next_offset + i);
        top++;
        if (top == stack_size) {
          status = double_buffer((void **) &stack, &stack_size,
                                 sizeof(OCT_NODE *));
          if (status != 0) {
            free(stack);
            return -1;
          }
        }
      }
    }
  }

  free(stack);
  return 0;
}

int append_buffer(OCT_TREE *tree, size_t node_offset, ENTITY *entity,
                  size_t collider_offset) {
  size_t buff_len = tree->data_buff_len;
  add_to_list(tree, buff_len, node_offset);

  tree->data_buffer[buff_len].node_offset = node_offset;
  tree->data_buffer[buff_len].collider_offset = collider_offset;
  tree->data_buffer[buff_len].entity = entity;

  entity->tree_offsets[collider_offset] = buff_len;

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
  if (node->empty) {
    node->head_offset = obj_offset;
    node->tail_offset = obj_offset;
    obj->prev_offset = obj_offset;
    obj->next_offset = obj_offset;
    node->empty = 0;
    return 0;
  }

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
  OCT_NODE *node = tree->node_buffer + obj->node_offset;

  if (node->head_offset == node->tail_offset) {
    obj->node_offset = INVALID;
    node->head_offset = INVALID;
    node->tail_offset = INVALID;
    node->empty = 1;
    return 0;
  }

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

OCTANT detect_octant(vec3 min_extent, vec3 max_extent, float *obj_extents,
                     float *oct_len) {
  float len = *oct_len;
  *oct_len = len / 2.0;
  if (obj_extents[0] <= max_extent[0] &&
      obj_extents[1] >= min_extent[0] + len) {
    if (obj_extents[2] <= max_extent[1] &&
        obj_extents[3] >= min_extent[1] + len) {
      if (obj_extents[4] <= max_extent[2] &&
          obj_extents[5] >= min_extent[2] + len) {
        min_extent[0] += len;
        min_extent[1] += len;
        min_extent[2] += len;
        return X_Y_Z;
      } else if (obj_extents[4] <= min_extent[2] + len &&
                 obj_extents[5] >= min_extent[2]) {
        min_extent[0] += len;
        min_extent[1] += len;
        max_extent[2] -= len;
        return X_Y_negZ;
      }
    } else if (obj_extents[2] <= min_extent[1] + len &&
               obj_extents[3] >= min_extent[1]) {
      if (obj_extents[4] <= max_extent[2] &&
          obj_extents[5] >= min_extent[2] + len) {
        min_extent[0] += len;
        min_extent[2] += len;
        max_extent[1] -= len;
        return X_negY_Z;
      } else if (obj_extents[4] <= min_extent[2] + len &&
                 obj_extents[5] >= min_extent[2]) {
        min_extent[0] += len;
        max_extent[1] -= len;
        max_extent[2] -= len;
        return X_negY_negZ;
      }
    }
  } else if(obj_extents[0] <= min_extent[0] + len &&
            obj_extents[1] >= min_extent[0]) {
    if (obj_extents[2] <= max_extent[1] &&
        obj_extents[3] >= min_extent[1] + len) {
      if (obj_extents[4] <= max_extent[2] &&
          obj_extents[5] >= min_extent[2] + len) {
        max_extent[0] -= len;
        min_extent[1] += len;
        min_extent[2] += len;
        return negX_Y_Z;
      } else if (obj_extents[4] <= min_extent[2] + len &&
                 obj_extents[5] >= min_extent[2]) {
        max_extent[0] -= len;
        min_extent[1] += len;
        max_extent[2] -= len;
        return negX_Y_negZ;
      }
    } else if (obj_extents[2] <= min_extent[1] + len &&
               obj_extents[3] >= min_extent[1]) {
      if (obj_extents[4] <= max_extent[2] &&
          obj_extents[5] >= min_extent[2] + len) {
        max_extent[0] -= len;
        max_extent[1] -= len;
        min_extent[2] += len;
        return negX_negY_Z;
      } else if (obj_extents[4] <= min_extent[2] + len &&
                 obj_extents[5] >= min_extent[2]) {
        max_extent[0] -= len;
        max_extent[1] -= len;
        max_extent[2] -= len;
        return negX_negY_negZ;
      }
    }
 }

  return MULTIPLE;
}
