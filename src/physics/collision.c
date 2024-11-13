#include <physics/collision.h>

/*
  Implementation of the Gilbert-Johnson-Keerthi (GJK) algorithm for
  collision detection. Refer to personal notes for in-depth details.
 */
int collision_check(COLLIDER *a, COLLIDER *b, vec3 *simplex) {
  // TODO Num used is always 8
  if (a == NULL || b == NULL || (a->type == POLY && a->data.num_used > 8) ||
      (b->type == POLY && b->data.num_used > 8) || (a->type == SPHERE &&
      a->data.radius < 0.0) || (b->type == SPHERE && b->data.radius < 0.0)) {
    return 0;
  }

  // Test simple sphere-sphere case
  if (a->type == SPHERE && b->type == SPHERE) {
    float dist = glm_vec3_distance(a->data.center, b->data.center);
    return dist <= a->data.radius + b->data.radius;
  }

  vec3 dir = { 1.0, 0.0, 0.0 };
  //vec3 simplex[4];
  support_func(a, b, dir, simplex[0]);
  glm_vec3_negate_to(simplex[0], dir);
  unsigned int num_used = 1;

  for (size_t cur_iter = 0; cur_iter < MAX_GJK_ITERATIONS; cur_iter++) {
    support_func(a, b, dir, simplex[num_used]);
    if (glm_vec3_dot(dir, simplex[num_used]) <= 0.0) {
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
  // Should never occur
  return 0;
}

/*
 * (Slightly customized) Implementation of the Expanding Polytope Algorithm
 * (EPA) for identification of collision depth and direction. Refer to personal
 * notes for in-depth details
 */
int epa_response(COLLIDER *a, COLLIDER *b, vec3 *simplex, vec3 p_dir,
                 float *p_depth) {
  // Simple sphere-sphere case
  if (a->type == SPHERE && b->type == SPHERE) {
    float dist = glm_vec3_distance(a->data.center, b->data.center);
    glm_vec3_sub(b->data.center, a->data.center, p_dir);
    glm_vec3_normalize(p_dir);
    *p_depth = (a->data.radius + b->data.radius) - dist;
    return 0;
  }

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
  for (size_t cur_iter = 0; cur_iter < MAX_EPA_ITERATIONS; cur_iter++) {
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
      glm_vec3_normalize(s_dir);
      glm_vec3_normalize(cur_norm);
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

// Expects a p_vec pointing in the direction of penetration, not the direction
// of penetration repair.
// Utilized Sutherland-Hodgman clipping algortihm
void collision_point(COLLIDER *a, COLLIDER *b, vec3 p_vec, vec3 dest) {
  // Simple sphere case
  if (a->type == SPHERE) {
    glm_vec3_scale_as(p_vec, a->data.radius, dest);
    glm_vec3_add(a->data.center, dest, dest);
    return;
  } else if (b->type == SPHERE) {
    glm_vec3_scale_as(p_vec, b->data.radius * -1.0, dest);
    glm_vec3_add(b->data.center, dest, dest);
    return;
  }

  vec3 ap_vec = GLM_VEC3_ZERO_INIT;
  glm_vec3_normalize_to(p_vec, ap_vec);
  float face_test = 0.0;
  // TODO Num used is always 8
  unsigned int num_used = a->data.num_used;
  unsigned int starting_index = max_dot(a->data.verts, num_used, ap_vec);
  unsigned int cur_index = 0;
  vec3 cur_vert = GLM_VEC3_ZERO_INIT;

  // Generate the surface of collision for the first collider
  vec3 a_face[4];
  vec3 a_norm = GLM_VEC3_ZERO_INIT;
  glm_vec3_copy(a->data.verts[starting_index], a_face[0]);

  // TODO In the event of an infinite, uncomment this stuff to check if the
  // face test threshold needs adjusting
  //fprintf(stderr, "a[0]: %f, %f, %f\n", a_face[0][X], a_face[0][Y], a_face[0][Z]);

  unsigned int a_face_len = 1;
  for (unsigned int i = 1; i < num_used; i++) {
    cur_index = (starting_index + i) % num_used;
    glm_vec3_sub(a->data.verts[cur_index], a_face[0], cur_vert);
    glm_vec3_normalize(cur_vert);
    face_test = glm_vec3_dot(ap_vec, cur_vert);

    // TODO In the event of an infinite, uncomment this stuff to check if the
    // face test threshold needs adjusting
    //fprintf(stderr, "cur_index: %d, %f, %f, %f => %f, %f, %f => %f\n",
    //        cur_index, a->data.verts[cur_index][X],
    //        a->data.verts[cur_index][Y], a->data.verts[cur_index][Z],
    //        cur_vert[X], cur_vert[Y], cur_vert[Z], face_test);

    if ((face_test >= -0.0005 && face_test <= 0.0005) &&
        (cur_vert[0] != 0.0 || cur_vert[1] != 0.0 || cur_vert[2] != 0.0)) {
      glm_vec3_copy(a->data.verts[cur_index], a_face[a_face_len]);
      a_face_len++;
      if (a_face_len > FACE_COL) {
        break;
      }
    }
  }

  // Surface of collision is a face
  vec3 edge1 = GLM_VEC3_ZERO_INIT;
  vec3 edge2 = GLM_VEC3_ZERO_INIT;
  if (a_face_len > FACE_COL) {
    glm_vec3_sub(a_face[3], a_face[0], edge1);
    glm_vec3_sub(a_face[1], a_face[0], edge2);
    glm_vec3_cross(edge1, edge2, a_norm);
  }

  // Surface of collision is just a point
  if (a_face_len == POINT_COL) {
    glm_vec3_copy(a_face[0], dest);
    return;
  }

  vec3 bp_vec;
  glm_vec3_negate_to(ap_vec, bp_vec);
  // TODO Num used is always 8
  num_used = b->data.num_used;
  starting_index = max_dot(b->data.verts, num_used, bp_vec);

  // Generate the surface of collision for the second collider
  // b_face is of 8 verticies because it will later be clipped to a face up to
  // 8 verticies large
  vec3 b_face[8];
  vec3 b_norm = GLM_VEC3_ZERO_INIT;
  glm_vec3_copy(b->data.verts[starting_index], b_face[0]);

  // TODO In the event of an infinite, uncomment this stuff to check if the
  // face test threshold needs adjusting
  //fprintf(stderr, "b[0]: %f, %f, %f\n", b_face[0][X], b_face[0][Y],
  //        b_face[0][Z]);

  unsigned int b_face_len = 1;
  for (unsigned int i = 1; i < num_used; i++) {
    cur_index = (starting_index + i) % num_used;
    glm_vec3_sub(b->data.verts[cur_index], b_face[0], cur_vert);
    glm_vec3_normalize(cur_vert);
    face_test = glm_vec3_dot(bp_vec, cur_vert);

    // TODO In the event of an infinite, uncomment this stuff to check if the
    // face test threshold needs adjusting
    //fprintf(stderr, "cur_index: %d, %f, %f, %f => %f, %f, %f => %f\n",
    //        cur_index, b->data.verts[cur_index][X],
    //        b->data.verts[cur_index][Y], b->data.verts[cur_index][Z],
    //        cur_vert[X], cur_vert[Y], cur_vert[Z], face_test);

    if ((face_test >= -0.0005 && face_test <= 0.0005) &&
        (cur_vert[0] != 0.0 || cur_vert[1] != 0.0 || cur_vert[2] != 0.0)) {
      glm_vec3_copy(b->data.verts[cur_index], b_face[b_face_len]);
      b_face_len++;
      if (b_face_len > FACE_COL) {
        break;
      }
    }
  }

  // Surface of collision is a face
  if (b_face_len > FACE_COL) {
    glm_vec3_sub(b_face[3], b_face[0], edge1);
    glm_vec3_sub(b_face[1], b_face[0], edge2);
    glm_vec3_cross(edge1, edge2, b_norm);
  }

  // Surface of collision is just a point
  if (b_face_len == POINT_COL) {
    glm_vec3_copy(b_face[0], dest);
    return;
  }

  vec3 clip_dir = GLM_VEC3_ZERO_INIT;
  vec3 clip_edge = GLM_VEC3_ZERO_INIT;
  vec3 cur_test_dir = GLM_VEC3_ZERO_INIT;
  vec3 prev_test_dir = GLM_VEC3_ZERO_INIT;
  int next_vert = 0;
  int prev_vert = 0;


  if (a_face_len == EDGE_COL || b_face_len == EDGE_COL) {
    // Edge - Face case
    vec3 *face = NULL;
    unsigned int face_len = 0;
    vec3 *edge = NULL;
    vec3 norm = GLM_VEC3_ZERO_INIT;
    if (a_face_len == EDGE_COL) {
      face = b_face;
      edge = a_face;
      face_len = b_face_len;
      glm_vec3_copy(b_norm, norm);
    } else {
      face = a_face;
      edge = b_face;
      face_len = a_face_len;
      glm_vec3_copy(a_norm, norm);
    }

    vec3 col_edge[2];
    glm_vec3_copy(edge[0], col_edge[0]);
    glm_vec3_copy(edge[1], col_edge[1]);

    // Clip edge to fit inside face
    for (unsigned int i = 0; i < face_len; i++) {
      next_vert = (i + 1) % face_len;
      glm_vec3_sub(face[next_vert], face[i], clip_edge);
      glm_vec3_normalize(clip_edge);
      glm_vec3_cross(norm, clip_edge, clip_dir);
      glm_vec3_normalize(clip_dir);

      glm_vec3_sub(col_edge[0], face[i], cur_test_dir);
      if (glm_vec3_dot(cur_test_dir, clip_dir) > 0.0) {
        intersection_point(col_edge[1], col_edge[0],
                           face[i], face[next_vert],
                           clip_dir, col_edge[0]);
      }

      glm_vec3_sub(col_edge[1], face[i], cur_test_dir);
      if (glm_vec3_dot(cur_test_dir, clip_dir) > 0.0) {
        intersection_point(col_edge[0], col_edge[1],
                           face[i], face[next_vert],
                           clip_dir, col_edge[1]);
      }
    }

    glm_vec3_center(col_edge[0], col_edge[1], dest);

    if (isnan(dest[X]) || isnan(dest[Y]) || isnan(dest[Y]) ||
        isinf(dest[X]) || isinf(dest[Y]) || isinf(dest[Z])) {
      // FAILSAFE: Due to floating point inaccuracy, one of the collision faces
      // is not correct, so the collision point is turning out to be NaN.
      // Prevent a crash/inf. loop by simply returning an approximation, which
      // is just one of the points on the collision face.
      glm_vec3_copy(a_face[0], dest);
    }
    return;
  }

  // Face - Face case
  vec3 col_face[8];
  unsigned int col_face_len = 0;

  // Clip face b to fit into face a using Sutherland-Hodgeman
  for (unsigned int i = 0; i < a_face_len; i++) {
    next_vert = (i + 1) % a_face_len;
    glm_vec3_sub(a_face[next_vert], a_face[i], clip_edge);
    glm_vec3_normalize(clip_edge);
    glm_vec3_cross(a_norm, clip_edge, clip_dir);
    glm_vec3_normalize(clip_dir);

    for (unsigned int j = 0; j < b_face_len; j++) {
      if (j == 0) {
        prev_vert = b_face_len - 1;
      } else {
        prev_vert = j - 1;
      }
      glm_vec3_sub(b_face[j], a_face[i], cur_test_dir);
      glm_vec3_sub(b_face[prev_vert], a_face[i], prev_test_dir);
      if (glm_vec3_dot(cur_test_dir, clip_dir) <= 0.0) {
        if (glm_vec3_dot(prev_test_dir, clip_dir) > 0.0) {
          intersection_point(b_face[j], b_face[prev_vert],
                             a_face[i], a_face[next_vert],
                             clip_dir, col_face[col_face_len]);
          col_face_len++;
        }

        glm_vec3_copy(b_face[j], col_face[col_face_len]);
        col_face_len++;
      } else if (glm_vec3_dot(prev_test_dir, clip_dir) <= 0.0) {
        intersection_point(b_face[prev_vert], b_face[j],
                           a_face[i], a_face[next_vert],
                           clip_dir, col_face[col_face_len]);
        col_face_len++;
      }
    }

    for (unsigned int j = 0; j < col_face_len; j++) {
      glm_vec3_copy(col_face[j], b_face[j]);
    }
    /*
    if (col_face_len == 0) {
      fprintf(stderr,
              "Error: Collision detection mesh with invalid winding order\n");
      exit(1);
    }
    */
    b_face_len = col_face_len;
    col_face_len = 0;
  }

  glm_vec3_zero(dest);
  for (unsigned int i = 0; i < b_face_len; i++) {
    glm_vec3_add(b_face[i], dest, dest);
  }
  dest[0] /= b_face_len;
  dest[1] /= b_face_len;
  dest[2] /= b_face_len;

  if (isnan(dest[X]) || isnan(dest[Y]) || isnan(dest[Y]) ||
      isinf(dest[X]) || isinf(dest[Y]) || isinf(dest[Z])) {
    // FAILSAFE: Due to floating point inaccuracy, one of the collision faces
    // is not correct, so the collision point is turning out to be NaN.
    // Prevent a crash/inf. loop by simply returning an approximation, which is
    // just one of the points on the collision face.
    glm_vec3_copy(a_face[0], dest);
  }

  return;
}

// Finds intersection point of line ab and cd. Norm is perpendicular to cd.
// ASSUMES LINES ARE INTERSECTING
void intersection_point(vec3 a, vec3 b,
                        vec3 c, vec3 d,
                        vec3 norm, vec3 dest) {
  vec3 b_min_a = GLM_VEC3_ZERO_INIT;
  vec3 d_min_c = GLM_VEC3_ZERO_INIT;
  vec3 b_min_d = GLM_VEC3_ZERO_INIT;
  glm_vec3_sub(b, a, b_min_a);
  glm_vec3_sub(d, c, d_min_c);
  glm_vec3_sub(b, d, b_min_d);
  float bd_proj = glm_vec3_dot(b_min_d, norm);
  float sub_mag = 0.0;
  if (glm_vec3_dot(b_min_a, d_min_c) != 0.0) {
    sub_mag = bd_proj / sin(glm_vec3_angle(b_min_a, d_min_c));
  } else {
    sub_mag = bd_proj;
  }
  glm_vec3_scale_as(b_min_a, sub_mag, dest);
  glm_vec3_sub(b_min_a, dest, dest);
  glm_vec3_add(dest, a, dest);
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
  // TODO Num used is always 8
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
    // TODO Num used is always 8
    glm_vec3_copy(a->data.verts[max_dot(a->data.verts, a->data.num_used, dir)],
                  max_a);
  }

  glm_vec3_negate_to(dir, temp);
  if (b->type == SPHERE) {
    glm_vec3_scale_as(temp, b->data.radius, max_b);
    glm_vec3_add(max_b, b->data.center, max_b);
  } else {
    // TODO Num used is always 8
    glm_vec3_copy(b->data.verts[max_dot(b->data.verts, b->data.num_used,
                  temp)], max_b);
  }

  glm_vec3_sub(max_a, max_b, dest);
}

int max_dot(vec3 *verts, unsigned int len, vec3 dir) {
  float max = -FLT_MAX;
  float temp = 0.0;
  int index = 0;
  for (unsigned int i = 0; i < len; i++) {
    temp = glm_vec3_dot(dir, verts[i]);
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
  glm_vec3_cross(c_min_a, b_min_a, dest_norm);
  glm_vec3_normalize(dest_norm);

  // Ensure normal is always point of triangle
  // (Utilizes properties of triangles which contain the origin)
  float dist = remove_noise(glm_vec3_dot(dest_norm, a), 0.00001);
  if (dist < 0.0) {
    dist *= -1.0;
    glm_vec3_negate(dest_norm);
  }

  // project one of the points onto normal
  return dist;
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
      (*num_edges)--;
      e_len--;
      cur_edge[0] = buff[e_len][0];
      cur_edge[1] = buff[e_len][1];
      return 0;
    }
  }

  buff[e_len][0] = a;
  buff[e_len][1] = b;
  (*num_edges)++;
  if (*e_buff_size == *num_edges) {
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
