#include <physics.h>

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

  COLLIDER *obj = entity->model->colliders + collider_offset;

  vec3 max_extent = { 16.0, 16.0, 16.0 };
  vec3 min_extent = { -16.0, -16.0, -16.0 };
  float max_extents[6];
  max_extents[0] = obj->verts[max_dot(obj, X)][0];
  max_extents[1] = obj->verts[max_dot(obj, NEG_X)][0];
  max_extents[2] = obj->verts[max_dot(obj, Y)][1];
  max_extents[3] = obj->verts[max_dot(obj, NEG_Y)][1];
  max_extents[4] = obj->verts[max_dot(obj, Z)][2];
  max_extents[5] = obj->verts[max_dot(obj, NEG_Z)][2];
  float oct_len = 16.0;

  OCTANT cur_oct = MULTIPLE;
  size_t cur_offset = 0;

  int status = 0;
  int depth = 1;
  int inserting = 1;
  while (inserting) {
    if (depth == MAX_DEPTH) {
      inserting = 0;
      status = append_list(tree, cur_offset, entity, collider_offset);
      if (status != 0) {
        printf("Unable to allocate tree data\n");
        return -1;
      }
    } else {
      cur_oct = detect_octant(min_extent, max_extent, max_extents, &oct_len);
      if (cur_oct == MULTIPLE) {
        inserting = 0;
        status = append_list(tree, cur_offset, entity, collider_offset);
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

int oct_tree_delete(OCT_TREE *tree, size_t node_offset, size_t obj_offset) {
  if (tree == NULL || node_offset > tree->node_buff_len ||
      obj_offset > tree->data_buff_len) {
    printf("Invalid deletion input\n");
    return -1;
  }

  OCT_NODE *node = tree->node_buffer + node_offset;
  if (node->empty) {
    return 0;
  }

  PHYS_OBJ *obj = tree->data_buffer + obj_offset;

  // Extract from node
  if (obj->next_offset == obj->prev_offset) {
    node->empty = 1;
  } else {
    tree->data_buffer[obj->prev_offset].next_offset = obj->next_offset;
    tree->data_buffer[obj->next_offset].prev_offset = obj->prev_offset;
  }

  // Swap with end of the list
  size_t end_offset = tree->data_buff_len - 1;
  if (obj_offset != end_offset) {
    obj->prev_offset = tree->data_buffer[end_offset].prev_offset;
    obj->next_offset = tree->data_buffer[end_offset].next_offset;
    obj->entity = tree->data_buffer[end_offset].entity;
    obj->collider_offset = tree->data_buffer[end_offset].collider_offset;
    obj->node_offset = tree->data_buffer[end_offset].node_offset;

    obj->entity->tree_offsets[obj->collider_offset] = obj_offset;

    tree->data_buffer[obj->prev_offset].next_offset = obj_offset;
    tree->data_buffer[obj->next_offset].prev_offset = obj_offset;
  }

  // Delete node
  tree->data_buffer[end_offset].entity = NULL;
  tree->data_buffer[end_offset].collider_offset = 0xBAADF00D;
  tree->data_buffer[end_offset].node_offset = 0xBAADF00D;
  tree->data_buffer[end_offset].next_offset = 0xBAADF00D;
  tree->data_buffer[end_offset].prev_offset = 0xBAADF00D;
  (tree->data_buff_len)--;

  return 0;
}

COLLISION_RES oct_tree_search(OCT_TREE *tree, COLLIDER *hit_box) {
  COLLISION_RES res = { NULL, 0, 0 };
  if (tree == NULL || hit_box == NULL) {
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
  max_extents[0] = hit_box->verts[max_dot(hit_box, X)][0];
  max_extents[1] = hit_box->verts[max_dot(hit_box, NEG_X)][0];
  max_extents[2] = hit_box->verts[max_dot(hit_box, Y)][1];
  max_extents[3] = hit_box->verts[max_dot(hit_box, NEG_Y)][1];
  max_extents[4] = hit_box->verts[max_dot(hit_box, Z)][2];
  max_extents[5] = hit_box->verts[max_dot(hit_box, NEG_Z)][2];
  float oct_len = 16.0;

  OCTANT cur_oct = MULTIPLE;
  size_t cur_offset = 0;

  int status = 0;
  int depth = 1;
  int searching = 1;
  while (searching) {
    status = read_oct(tree, tree->node_buffer + cur_offset, &res);
    if (status != 0) {
      free(res.list);
      res.list = NULL;
      res.list_len = 0;
      res.list_buff_size = 0;
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
          free(res.list);
          res.list = NULL;
          res.list_len = 0;
          res.list_buff_size = 0;
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

/*
 * Implementation of the Gilbert-Johnson-Keerthi (GJK) algorithm for
 * collision detection. Refer to personal notes for in-depth details.
 */
int collision_check(COLLIDER *a, COLLIDER *b) {
  if (a == NULL || b == NULL || a->num_used > 8 || b->num_used > 8) {
    return 0;
  }

  vec3 dir = { 1.0, 0.0, 0.0 };
  vec3 simplex[4];
  support_func(a, b, dir, simplex[0]);
  glm_vec3_negate_to(simplex[0], dir);
  unsigned int num_used = 1;

  vec3 temp = { 0.0, 0.0, 0.0 };
  vec3 b_min_a = { 0.0, 0.0, 0.0 };
  vec3 neg_a = { 0.0, 0.0, 0.0 };

  while (true) {
    support_func(a, b, dir, simplex[num_used]);
    if (glm_vec3_dot(dir, simplex[num_used]) < 0.0) {
      return 0;
    }
    num_used++;

    if (num_used == LINE) {
      glm_vec3_sub(simplex[0], simplex[1], b_min_a);
      glm_vec3_scale(simplex[1], -1.0, neg_a);
      glm_vec3_cross(b_min_a, neg_a, temp);
      glm_vec3_cross(temp, b_min_a, dir);
    } else if (num_used == TRIANGLE) {
      triangle_check(simplex[2], simplex[0], simplex[1], &num_used);
    } else if (num_used == TETRAHEDRON) {
      triangle_check(simplex[3], simplex[0], simplex[1], &num_used);
      triangle_check(simplex[3], simplex[1], simplex[2], &num_used);
      triangle_check(simplex[3], simplex[2], simplex[0], &num_used);

      if (num_used == TETRAHEDRON) {
        return 1;
      }
    } else {
      // Should never occur
      return 0;
    }
  }
}

void triangle_check(vec3 a, vec3 b, vec3 c, unsigned int *num_used) {
  vec3 temp = { 0.0, 0.0, 0.0 };
  vec3 temp_2 = { 0.0, 0.0, 0.0 };
  vec3 b_min_a = { 0.0, 0.0, 0.0 };
  vec3 c_min_a = { 0.0, 0.0, 0.0 };
  vec3 neg_a = { 0.0, 0.0, 0.0 };

  glm_vec3_sub(b, a, b_min_a);
  glm_vec3_sub(c, a, c_min_a);
  glm_vec3_scale(a, -1.0, neg_a);

  glm_vec3_cross(c_min_a, b_min_a, temp);
  glm_vec3_cross(temp, b_min_a, temp);

  if (glm_vec3_dot(temp, neg_a) > 0) {
    c[0] = a[0];
    c[1] = a[1];
    c[2] = a[2];
    (*num_used)--;
  } else {
    glm_vec3_cross(b_min_a, c_min_a, temp_2);
    glm_vec3_cross(temp_2, c_min_a, temp_2);

    if (glm_vec3_dot(temp_2, neg_a) > 0) {
      b[0] = a[0];
      b[1] = a[1];
      b[2] = a[2];
      (*num_used)--;
    }
  }
}

void support_func(COLLIDER *a, COLLIDER *b, vec3 dir, vec3 dest) {
  if (a == NULL || b == NULL || a->num_used > 8 || b->num_used > 8) {
    return;
  }

  vec3 max_a = { 0.0, 0.0, 0.0 };
  vec3 max_b = { 0.0, 0.0, 0.0 };
  vec3 temp = { 0.0, 0.0, 0.0 };

  glm_vec3_copy(a->verts[max_dot(a, dir)], max_a);

  glm_vec3_scale(dir, -1.0, temp);
  glm_vec3_copy(b->verts[max_dot(b, temp)], max_b);

  glm_vec3_sub(max_a, max_b, dest);
}

int max_dot(COLLIDER *a, vec3 dir) {
  float max = -FLT_MAX;
  float temp = 0.0;
  int index = 0;
  for (unsigned int i = 0; i < a->num_used; i++) {
    temp = glm_vec3_dot(dir, a->verts[i]);
    if (temp > max) {
      max = temp;
      index = i;
    }
  }

  return index;
}

int init_node(OCT_TREE *tree, OCT_NODE *parent) {
  if (tree == NULL || parent == NULL) {
    printf("Invalid node inputs\n");
    return -1;
  }

  parent->next_offset = tree->node_buff_len;

  for (size_t i = tree->node_buff_len; i < tree->node_buff_len + 8; i++) {
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
      status = double_buffer((void **) res->list, &res->list_buff_size,
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
  int status = read_oct(tree, node, res);

  for (OCTANT i = X_Y_Z; i < negX_negY_negZ; i++) {
    status = read_all_children(tree, tree->node_buffer + node->next_offset + i,
                               res);
    if (status != 0) {
      return -1;
    }
  }
  return 0;
}

int append_list(OCT_TREE *tree, size_t node_offset, ENTITY *entity,
                size_t collider_offset) {
  size_t buff_len = tree->data_buff_len;
  OCT_NODE *node = tree->node_buffer + node_offset;
  if (node->empty) {
    node->head_offset = buff_len;
    node->tail_offset = buff_len;
    tree->data_buffer[buff_len].prev_offset = buff_len;
    tree->data_buffer[buff_len].next_offset = buff_len;
    node->empty = 0;
  } else {
    tree->data_buffer[node->tail_offset].next_offset = buff_len;
    tree->data_buffer[buff_len].prev_offset = node->tail_offset;
    tree->data_buffer[buff_len].next_offset = node->head_offset;
    tree->data_buffer[node->head_offset].prev_offset = buff_len;
    node->tail_offset = buff_len;
  }

  tree->data_buffer[node->tail_offset].node_offset = node_offset;
  tree->data_buffer[node->tail_offset].collider_offset = collider_offset;
  tree->data_buffer[node->tail_offset].entity = entity;

  entity->tree_offsets[collider_offset] = node->tail_offset;

  (tree->data_buff_len)++;
  if (tree->data_buff_len == tree->data_buff_size) {
    return double_buffer((void **) tree->data_buffer, &tree->data_buff_size,
                         sizeof(PHYS_OBJ));
  }

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
