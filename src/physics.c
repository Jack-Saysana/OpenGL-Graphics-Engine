#include <physics.h>

/*
 * Implementation of the Gilbert-Johnson-Keerthi (GJK) algorithm for
 * collision detection. Refer to personal notes for in-depth details.
 */
int collision_check(COLLIDER *a, COLLIDER *b, vec3 *simplex) {
  if (a == NULL || b == NULL || (a->type == POLY && a->data.num_used > 8) ||
      (b->type == POLY && b->data.num_used > 8) || (a->type == SPHERE &&
      a->data.radius < 0.0) || (b->type == SPHERE && b->data.radius < 0.0)) {
    return 0;
  }

  vec3 dir = { 1.0, 0.0, 0.0 };
  //vec3 simplex[4];
  support_func(a, b, dir, simplex[0]);
  glm_vec3_negate_to(simplex[0], dir);
  unsigned int num_used = 1;

  while (true) {
    support_func(a, b, dir, simplex[num_used]);
    if (glm_vec3_dot(dir, simplex[num_used]) < 0.0) {
      return 0;
    }
    for (int i = 0; i < num_used; i++) {
      if (simplex[i][0] == simplex[num_used][0] &&
          simplex[i][1] == simplex[num_used][1] &&
          simplex[i][2] == simplex[num_used][2]) {
        return 0;
      }
    }
    num_used++;

    if (num_used == LINE) {
      calc_dir_line(simplex[1], simplex[0], dir);
    } else if (num_used == TRIANGLE) {
      if (triangle_check(simplex[2], simplex[0], simplex[1],
                         &num_used, dir) == COLLISION) {
        return 1;
      }
    } else if (num_used == TETRAHEDRON) {
      if (tetrahedron_check(simplex, &num_used, dir) == COLLISION) {
        return 1;
      }
    } else {
      // Should never occur
      return 0;
    }
  }
}

/*
 * (Slightly customized) Implementation of the Expanding Polytope Algorithm
 * (EPA) for identification of collision depth and direction. Refer to personal
 * notes for in-depth details
 */
int epa_response(COLLIDER *a, COLLIDER *b, vec3 *simplex, vec3 p_dir,
                 float *p_depth) {
  vec3 *polytope = malloc(sizeof(vec3) * BUFF_STARTING_LEN);
  if (polytope == NULL) {
    return -1;
  }
  size_t num_verts = 4;
  size_t polytope_buff_len = BUFF_STARTING_LEN;
  glm_vec3_copy(simplex[0], polytope[0]);
  glm_vec3_copy(simplex[1], polytope[1]);
  glm_vec3_copy(simplex[2], polytope[2]);
  glm_vec3_copy(simplex[3], polytope[3]);

  // Face heap, which stores the face of least distance from the origin at
  // the top (Also allows random deletion)
  F_HEAP *faces = init_face_heap();
  if (faces == NULL) {
    free(polytope);
    return -1;
  }

  // Buffer that will store the edges that need to be repaired when expanding
  // the polytope
  int (*unique_edges)[2] = malloc(sizeof(int) * 2 * BUFF_STARTING_LEN);
  if (unique_edges == NULL) {
    free(polytope);
    free_faces(faces);
    return -1;
  }
  size_t num_edges = 0;
  size_t e_buff_size = BUFF_STARTING_LEN;

  // insert initial simplex into heap
  vec3 cur_norm = { 0.0, 0.0, 0.0 };
  // ABC
  float dist = calc_face_dist(polytope[3], polytope[0], polytope[1], cur_norm);
  int status = insert_face(faces, 3, 0, 1, cur_norm, dist); 
  if (status != 0) {
    free(polytope);
    free_faces(faces);
    free(unique_edges);
    return -1;
  }
  // ACD
  dist = calc_face_dist(polytope[3], polytope[1], polytope[2], cur_norm);
  status = insert_face(faces, 3, 1, 2, cur_norm, dist); 
  if (status != 0) {
    free(polytope);
    free_faces(faces);
    free(unique_edges);
    return -1;
  }
  // ADB
  dist = calc_face_dist(polytope[3], polytope[2], polytope[0], cur_norm);
  status = insert_face(faces, 3, 2, 0, cur_norm, dist); 
  if (status != 0) {
    free(polytope);
    free_faces(faces);
    free(unique_edges);
    return -1;
  }
  // CBD
  dist = calc_face_dist(polytope[1], polytope[0], polytope[2], cur_norm);
  status = insert_face(faces, 1, 0, 2, cur_norm, dist); 
  if (status != 0) {
    free(polytope);
    free_faces(faces);
    free(unique_edges);
    return -1;
  }

  int j = -1;
  vec3 min_norm = { 0.0, 0.0, 0.0 };
  vec3 s_dir = { 0.0, 0.0, 0.0 };
  float min_dist = -1.0;
  ivec3 cur_face = { 0, 0, 0 };
  while (true) {
    // Get closest face (PULL FROM TOP OF HEAP)
    min_dist = faces->buffer[0].dist;

    remove_face(faces, 0, cur_face, min_norm);
    
    // Add edges to unique edges
    for (int i = 0; i < 3; i++) {
      j = (i + 1) % 3;
      status = add_unique_edges(&unique_edges, &num_edges, &e_buff_size,
                                cur_face[i], cur_face[j]);
      if (status != 0) {
        free(polytope);
        free_faces(faces);
        free(unique_edges);
        return -1;
      }
    }

    // Add support point
    support_func(a, b, min_norm, polytope[num_verts]);
    // Check if need to break
    dist = glm_vec3_dot(min_norm, polytope[num_verts]);
    if (fabs(dist - min_dist) <= 0.001) {
      break;
    }

    num_verts++;
    if (num_verts == polytope_buff_len) {
      status = double_buffer((void **) &polytope, &polytope_buff_len,
                             sizeof(vec3));
      if (status != 0) {
        free(polytope);
        free_faces(faces);
        free(unique_edges);
        return -1;
      }
    }

    // Remove appropriate faces
    for (int i = 0; i < faces->buff_len; i++) {
      glm_vec3_copy(faces->buffer[i].norm, cur_norm);

      glm_vec3_sub(polytope[num_verts - 1],
                   polytope[faces->buffer[i].indicies[0]], s_dir);
      if (glm_vec3_dot(cur_norm, /*polytope[num_verts - 1]*/s_dir) > 0) {
        remove_face(faces, i, cur_face, cur_norm);

        // ADD FACE EDGES TO UNIQUE EDGE LIST (IF EDGE IS NOT UNIQUE, REMOVE
        // IT FROM THE UNIQUE EDGE LIST)
        for (int z = 0; z < 3; z++) {
          j = (z + 1) % 3;
          status = add_unique_edges(&unique_edges, &num_edges, &e_buff_size,
                                    cur_face[z],
                                    cur_face[j]);
          if (status != 0) {
            free(polytope);
            free_faces(faces);
            free(unique_edges);
            return -1;
          }
        }
        i = -1;
      }
    }

    // Reconstruct faces using support point and unique edge list
    for (int i = 0; i < num_edges; i++) {
      dist = calc_face_dist(polytope[num_verts - 1],
                            polytope[unique_edges[i][0]],
                            polytope[unique_edges[i][1]],
                            cur_norm);
      status = insert_face(faces, num_verts - 1,  unique_edges[i][0],
                           unique_edges[i][1], cur_norm, dist);
      if (status != 0) {
        free(polytope);
        free_faces(faces);
        free(unique_edges);
        return -1;
      }
    }
    num_edges = 0;
  }

  glm_vec3_normalize_to(min_norm, p_dir);
  (*p_depth) = min_dist + 0.001f;

  free(polytope);
  free_faces(faces);
  free(unique_edges);
  return 0;
}

int tetrahedron_check(vec3 *simplex, unsigned int *num_used, vec3 dir) {
  vec3 b_min_a = { 0.0, 0.0, 0.0 };
  vec3 d_min_a = { 0.0, 0.0, 0.0 };
  vec3 c_min_a = { 0.0, 0.0, 0.0 };
  vec3 neg_a = { 0.0, 0.0, 0.0 };
  vec3 acb_norm = { 0.0, 0.0, 0.0 };
  vec3 abd_norm = { 0.0, 0.0, 0.0 };
  vec3 adc_norm = { 0.0, 0.0, 0.0 };

  glm_vec3_sub(simplex[3], simplex[0], b_min_a);
  glm_vec3_sub(simplex[3], simplex[1], c_min_a);
  glm_vec3_sub(simplex[3], simplex[2], d_min_a);
  glm_vec3_negate_to(simplex[3], neg_a);

  glm_vec3_cross(c_min_a, b_min_a, acb_norm);
  glm_vec3_cross(b_min_a, d_min_a, abd_norm);
  glm_vec3_cross(d_min_a, c_min_a, adc_norm);

  if (glm_vec3_dot(acb_norm, neg_a) > 0) {
    // REMOVE D AND RECALC
    simplex[2][0] = simplex[3][0];
    simplex[2][1] = simplex[3][1];
    simplex[2][2] = simplex[3][2];
    (*num_used)--;

    glm_vec3_copy(acb_norm, dir);
    return REMOVED;
  } else if (glm_vec3_dot(abd_norm, neg_a) > 0) {
    // REMOVE C AND RECALC
    simplex[1][0] = simplex[3][0];
    simplex[1][1] = simplex[3][1];
    simplex[1][2] = simplex[3][2];
    (*num_used)--;

    glm_vec3_copy(abd_norm, dir);
    return REMOVED;
  } else if (glm_vec3_dot(adc_norm, neg_a) > 0) {
    // REMOVE B AND RECALC
    simplex[0][0] = simplex[3][0];
    simplex[0][1] = simplex[3][1];
    simplex[0][2] = simplex[3][2];
    (*num_used)--;

    glm_vec3_copy(adc_norm, dir);
    return REMOVED;
  }

  return COLLISION;
}

int triangle_check(vec3 a, vec3 b, vec3 c, unsigned int *num_used, vec3 dir) {
  vec3 norm = { 0.0, 0.0, 0.0 };
  vec3 temp = { 0.0, 0.0, 0.0 };
  vec3 b_min_a = { 0.0, 0.0, 0.0 };
  vec3 c_min_a = { 0.0, 0.0, 0.0 };
  vec3 neg_a = { 0.0, 0.0, 0.0 };

  // A -> B
  glm_vec3_sub(b, a, b_min_a);
  // A -> C
  glm_vec3_sub(c, a, c_min_a);
  // A -> ORIGIN
  glm_vec3_negate_to(a, neg_a);
  //neg_a[1] = 0.0;

  // TRIANGLE NORMAL
  glm_vec3_cross(c_min_a, b_min_a, norm);

  glm_vec3_cross(norm, b_min_a, temp);
  if (glm_vec3_dot(temp, neg_a) > 0) {
    c[0] = a[0];
    c[1] = a[1];
    c[2] = a[2];
    (*num_used)--;

    calc_dir_line(a, b, dir);
    return REMOVED;
  }

  glm_vec3_negate(norm);

  glm_vec3_cross(norm, c_min_a, temp);
  if (glm_vec3_dot(temp, neg_a) > 0) {
    b[0] = a[0];
    b[1] = a[1];
    b[2] = a[2];
    (*num_used)--;

    calc_dir_line(a, c, dir);
    return REMOVED;
  }

  float dot = glm_vec3_dot(norm, neg_a);
  if (dot == 0) {
    return COLLISION;
  } else if (dot < 0) {
    glm_vec3_negate_to(norm, dir);
  } else {
    glm_vec3_copy(norm, dir);
  }

  return POSSIBLE;
}

void support_func(COLLIDER *a, COLLIDER *b, vec3 dir, vec3 dest) {
  if (a == NULL || b == NULL || dir == NULL || dest == NULL ||
      (a->type == POLY && a->data.num_used > 8) ||
      (b->type == POLY && b->data.num_used > 8) || (a->type == SPHERE &&
      a->data.radius < 0.0) || (b->type == SPHERE && b->data.radius < 0.0)) {
    return;
  }

  vec3 max_a = { 0.0, 0.0, 0.0 };
  vec3 max_b = { 0.0, 0.0, 0.0 };
  vec3 temp = { 0.0, 0.0, 0.0 };

  if (a->type == SPHERE) {
    glm_vec3_scale_as(dir, a->data.radius, max_a);
    glm_vec3_add(max_a, a->data.center, max_a);
  } else {
    glm_vec3_copy(a->data.verts[max_dot(a, dir)], max_a);
  }

  glm_vec3_negate_to(dir, temp);
  if (b->type == SPHERE) {
    glm_vec3_scale_as(temp, b->data.radius, max_b);
    glm_vec3_add(max_b, b->data.center, max_b);
  } else {
    glm_vec3_copy(b->data.verts[max_dot(b, temp)], max_b);
  } 

  glm_vec3_sub(max_a, max_b, dest);
}

int max_dot(COLLIDER *a, vec3 dir) {

  if (a->type != POLY) {
    printf("Input must be polyhedron\n");
    return -1;
  }

  float max = -FLT_MAX;
  float temp = 0.0;
  int index = 0;
  for (unsigned int i = 0; i < a->data.num_used; i++) {
    temp = glm_vec3_dot(dir, a->data.verts[i]);
    if (temp > max) {
      max = temp;
      index = i;
    }
  }

  return index;
}

void calc_dir_line(vec3 a, vec3 b, vec3 dir) {
  vec3 b_min_a = { 0, 0, 0 };
  vec3 neg_a = { 0, 0, 0 };

  glm_vec3_sub(b, a, b_min_a);
  glm_vec3_negate_to(a, neg_a);
  glm_vec3_cross(b_min_a, neg_a, dir);
  glm_vec3_cross(dir, b_min_a, dir);
}

float calc_face_dist(vec3 a, vec3 b, vec3 c, vec3 dest_norm) {
  // Get normal
  vec3 b_min_a = { 0.0, 0.0, 0.0 };
  vec3 c_min_a = { 0.0, 0.0, 0.0 };

  glm_vec3_sub(b, a, b_min_a);
  glm_vec3_sub(c, a, c_min_a);
  // MIGHT HAVE TO CHANGE
  glm_vec3_cross(c_min_a, b_min_a, dest_norm);
  glm_vec3_normalize(dest_norm);

  // project one of the points onto normal
  return glm_vec3_dot(dest_norm, a);
}

int add_unique_edges(int (**u_edges)[2], size_t *num_edges, size_t *e_buff_size,
                     int a, int b) {
  int (*buff)[2] = *u_edges;

  size_t e_len = *num_edges;
  int *cur_edge = NULL;
  for (int i = 0; i < e_len; i++) {
    cur_edge = buff[i];
    if((cur_edge[0] == a && cur_edge[1] == b) ||
       (cur_edge[1] == a && cur_edge[0] == b)) {
      cur_edge[0] = buff[e_len - 1][0];
      cur_edge[1] = buff[e_len - 1][1];
      (*num_edges)--;
      return 0;
    }
  }

  buff[e_len][0] = a;
  buff[e_len][1] = b;
  (*num_edges)++;
  if (*e_buff_size == e_len) {
    int status = double_buffer((void **) u_edges, e_buff_size,
                               sizeof(int) * 2);
    if (status != 0) {
      return -1;
    }
  }

  return 0;
}

F_HEAP *init_face_heap() {
  F_HEAP *heap = malloc(sizeof(F_HEAP));
  if (heap == NULL) {
    return NULL;
  }

  heap->buffer = malloc(sizeof(FACE) * BUFF_STARTING_LEN);
  if (heap->buffer == NULL) {
    free(heap);
    return NULL;
  }

  heap->buff_len = 0;
  heap->buff_size = BUFF_STARTING_LEN;

  return heap;
}

int insert_face(F_HEAP *heap, int a, int b, int c, vec3 norm, float dist) {
  ivec3 face = { a, b, c };
  glm_ivec3_copy(face, heap->buffer[heap->buff_len].indicies);
  glm_vec3_copy(norm, heap->buffer[heap->buff_len].norm);
  heap->buffer[heap->buff_len].dist = dist;

  size_t cur_index = heap->buff_len;
  FACE *cur = heap->buffer + cur_index;
  FACE *parent = heap->buffer + ((cur_index - 1) / 2);

  while (cur_index > 0 && parent->dist > dist) {
    glm_ivec3_copy(parent->indicies, cur->indicies);
    glm_vec3_copy(parent->norm, cur->norm);
    cur->dist = parent->dist;

    glm_ivec3_copy(face, parent->indicies);
    glm_vec3_copy(norm, parent->norm);
    parent->dist = dist;

    cur_index = (cur_index - 1) / 2;
    cur = parent;
    parent = heap->buffer + ((cur_index - 1) / 2);
  }

  heap->buff_len++;

  if (heap->buff_len == heap->buff_size) {
    int status = double_buffer((void **) &(heap->buffer), &(heap->buff_size),
                               sizeof(FACE));
    if (status != 0) {
      return -1;
    }
  }

  return 0;
}

void remove_face(F_HEAP *heap, size_t index, ivec3 d_ind, vec3 d_norm) {
  if (index >= heap->buff_len) {
    return;
  }

  glm_ivec3_copy(heap->buffer[index].indicies, d_ind);
  glm_vec3_copy(heap->buffer[index].norm, d_norm);
  heap->buff_len--;

  glm_ivec3_copy(heap->buffer[heap->buff_len].indicies,
                 heap->buffer[index].indicies);
  glm_vec3_copy(heap->buffer[heap->buff_len].norm, heap->buffer[index].norm);
  heap->buffer[index].dist = heap->buffer[heap->buff_len].dist;

  size_t cur_index = index;
  size_t left_index = (cur_index * 2) + 1;
  size_t right_index = (cur_index * 2) + 2;
  FACE *cur = heap->buffer + cur_index;
  FACE *left = heap->buffer + left_index;
  FACE *right = heap->buffer + right_index;
  FACE *swap = NULL;

  ivec3 temp_ind = { 0, 0, 0 };
  vec3 temp_norm = { 0.0, 0.0, 0.0 };
  float temp_dist = -1.0;
  while((left_index < heap->buff_len && left->dist < cur->dist) ||
        (right_index < heap->buff_len && right->dist < cur->dist)) {
    glm_ivec3_copy(cur->indicies, temp_ind);
    glm_vec3_copy(cur->norm, temp_norm);
    temp_dist = cur->dist;

    if (left_index < heap->buff_len && left->dist < cur->dist &&
        right_index < heap->buff_len && right->dist < cur->dist) { 
      swap = left->dist < right->dist ? left : right;
    } else if (left_index < heap->buff_len && left->dist < cur->dist) {
      swap = left;
    } else {
      swap = right;
    }

    glm_ivec3_copy(swap->indicies, cur->indicies);
    glm_ivec3_copy(temp_ind, swap->indicies);
    glm_vec3_copy(swap->norm, cur->norm);
    glm_vec3_copy(temp_norm, swap->norm);
    cur->dist = swap->dist;
    swap->dist = temp_dist;
    cur_index = swap == left ? left_index : right_index;

    left_index = (cur_index * 2) + 1;
    right_index = (cur_index * 2) + 2;
    cur = heap->buffer + cur_index;
    left = heap->buffer + left_index;
    right = heap->buffer + right_index;
  }
}

void free_faces(F_HEAP *heap) {
  free(heap->buffer);
  free(heap);
}