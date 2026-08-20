#include <collider.h>

// Validate collider is an 8-vertex polyhedra with coplanar faces and a valid
// winding order
int validate_collider(COLLIDER *col) {
  if (col->type == SPHERE) {
    return col->data.radius > ZERO_THRESHOLD;
  }

  if (col->data.num_used != 8) {
    return 0;
  }

  ivec4 faces[4] = {
                    { 0, 1, 2, 3 }, { 3, 4, 7, 0 },
                    { 1, 0, 7, 6 }, { 2, 1, 6, 5 }
                   };

  vec3 *verts = col->data.verts;
  vec3 temp = GLM_VEC3_ZERO_INIT;
  vec3 temp2 = GLM_VEC3_ZERO_INIT;
  vec3 a = GLM_VEC3_ZERO_INIT;
  vec3 b = GLM_VEC3_ZERO_INIT;
  vec3 c = GLM_VEC3_ZERO_INIT;
  vec3 d = GLM_VEC3_ZERO_INIT;
  for (int cur_face = 0; cur_face < 4; cur_face++) {
    // Ensure all points in face are distinct
    for (int pt_idx = 0; pt_idx < 4; pt_idx++) {
      for (int k = 1; k < 4; k++) {
        glm_vec3_sub(verts[faces[cur_face][pt_idx]],
                     verts[faces[cur_face][(pt_idx+k)%4]], temp);
        if (fabs(temp[X]) <= ZERO_THRESHOLD &&
            fabs(temp[Y]) <= ZERO_THRESHOLD &&
            fabs(temp[Z]) <= ZERO_THRESHOLD) {
          return 0;
        }
      }
    }

    // Ensure no 3 points are colinear
    for (int i = 0; i < 4; i++) {
      glm_vec3_copy(verts[faces[cur_face][i]], a);
      glm_vec3_copy(verts[faces[cur_face][(i+1)%4]], b);
      glm_vec3_copy(verts[faces[cur_face][(i+2)%4]], c);
      glm_vec3_sub(b, a, temp);
      glm_vec3_sub(c, a, temp2);
      glm_vec3_cross(temp, temp2, temp);
      if (fabs(glm_vec3_norm(temp)) <= ZERO_THRESHOLD) {
        return 0;
      }
    }

    // Ensure all 4 points are coplanar
    glm_vec3_copy(verts[faces[cur_face][0]], a);
    glm_vec3_copy(verts[faces[cur_face][1]], b);
    glm_vec3_copy(verts[faces[cur_face][2]], c);
    glm_vec3_copy(verts[faces[cur_face][2]], d);
    glm_vec3_sub(b, a, temp);
    glm_vec3_sub(c, a, temp2);
    glm_vec3_cross(temp, temp2, temp);
    glm_vec3_sub(d, a, temp2);
    if (fabs(glm_vec3_dot(temp, temp2)) > ZERO_THRESHOLD) {
      return 0;
    }
  }

  return 1;
}

// Brute force calculates the 3d convex hull of the 8 vertex collider
// (not a big deal since # of verts is small), then arranges the verts to
// fit the engine's winding order. Ensures each collider has exactly 6 faces
int sort_col_verts(COLLIDER *col) {
  // Each collider has 8 vertices. There are at most 56 groups of 3 vertices,
  // which is the upperbound for the number of possible faces
  COL_FACE faces[56];
  memset(faces, 0, sizeof(faces));
  size_t num_faces = 0;

  vec3 v_a = GLM_VEC3_ZERO_INIT;
  vec3 v_b = GLM_VEC3_ZERO_INIT;
  vec3 v_c = GLM_VEC3_ZERO_INIT;
  vec3 v_cur = GLM_VEC3_ZERO_INIT;
  vec3 norm = GLM_VEC3_ZERO_INIT;
  vec3 temp = GLM_VEC3_ZERO_INIT;
  for (int i = 0; i < 8; i++) {
  }
  for (unsigned int a = 0; a < 8; a++) {
    glm_vec3_copy(col->data.verts[a], v_a);
    for (unsigned int b = a+1; b < 8; b++) {
      glm_vec3_copy(col->data.verts[b], v_b);
      for (unsigned int c = b+1; c < 8; c++) {
        glm_vec3_copy(col->data.verts[c], v_c);

        glm_vec3_sub(v_b, v_a, temp);
        glm_vec3_sub(v_c, v_a, norm);
        glm_vec3_cross(temp, norm, norm);

        // Check if vertices are colinear
        float n = glm_vec3_norm(norm);
        if (n <= ZERO_THRESHOLD) {
          continue;
        }
        glm_vec3_normalize(norm);

        // Check if face is coplanar to any existing faces, if so, add it and
        // move on to next face
        int found_face = 0;
        for (size_t i = 0; i < num_faces; i++) {
          float n_dot = glm_vec3_dot(norm, faces[i].norm);
          if (fabs(fabs(n_dot) - 1.0) <= ZERO_THRESHOLD) {
            int num_found = 0;
            if (faces[i].verts[a]) {
              num_found++;
            }
            if (faces[i].verts[b]) {
              num_found++;
            }
            if (faces[i].verts[c]) {
              num_found++;
            }
            if (num_found >= 2) {
              faces[i].verts[a] = 1;
              faces[i].verts[b] = 1;
              faces[i].verts[c] = 1;
              found_face = 1;
              break;
            }
          }
        }
        if (found_face) {
          continue;
        }

        // Check if valid hull face, if so, create new face
        int pos = 0;
        int neg = 0;
        for (size_t i = 0; i < 8; i++) {
          if (i == a || i == b || i == c) {
            continue;
          }

          glm_vec3_copy(col->data.verts[i], v_cur);
          glm_vec3_sub(v_cur, v_a, temp);

          float dot = glm_vec3_dot(temp, norm);
          if (dot > ZERO_THRESHOLD) {
            pos = 1;
          } else if (dot < -ZERO_THRESHOLD) {
            neg = 1;
          }
        }

        if (!(pos && neg)) {
          faces[num_faces].verts[a] = 1;
          faces[num_faces].verts[b] = 1;
          faces[num_faces].verts[c] = 1;
          glm_vec3_copy(norm, faces[num_faces].norm);
          num_faces++;
        }
      }
    }
  }

  // Guarantees collider has exactly 6 faces with 4 verts each
  if (!validate_col_faces(faces, num_faces)) {
    return -1;
  }

  // Triangulate each rectangular face such that they each have triangles of
  // the form (0, 1, 2), (2, 3, 0)
  unsigned int face_inds[6][4];
  triangulate_col_faces(col, faces, face_inds);

  // Determine top, bottom, left, right, front and back faces
  unsigned int oriented_faces[6];
  orient_faces(face_inds, oriented_faces);

  // We now know which vertices are in the top and bottom faces, sort vertices
  // to match canonical collider topology
  vec3 sorted_verts[8];
  canonicalize_topology(col, face_inds, oriented_faces, sorted_verts);
  memcpy(col->data.verts, sorted_verts, sizeof(sorted_verts));
  return 0;
}

// Ensures each collider has exactly 6 faces with exactly 4 vertices
static int validate_col_faces(COL_FACE *faces, size_t num_faces) {
  // Collider is not a prism of 6 faces
  if (num_faces != 6) {
    return 0;
  }

  // Ensure each face has 4 vertices
  for (int i = 0; i < num_faces; i++) {
    int num_verts = 0;
    for (int j = 0; j < 8; j++) {
      if (faces[i].verts[j]) {
        num_verts++;
      }
    }

    if (num_verts != 4) {
      return 0;
    }
  }
  return 1;
}

static void triangulate_col_faces(COLLIDER *col, COL_FACE *faces,
                                  unsigned int (*face_inds)[4]) {
  // Calculate centroid of polyhedron
  vec3 centroid = GLM_VEC3_ZERO_INIT;
  for (int i = 0; i < 8; i++) {
    glm_vec3_add(col->data.verts[i], centroid, centroid);
  }
  glm_vec3_scale(centroid, 1.0 / 8.0, centroid);

  int cur_ind = 0;
  vec3 temp = GLM_VEC3_ZERO_INIT;
  for (int i = 0; i < 6; i++) {
    cur_ind = 0;
    for (int j = 0; j < 8; j++) {
      if (faces[i].verts[j]) {
        face_inds[i][cur_ind] = j;
        cur_ind++;
      }
    }
    glm_vec3_sub(col->data.verts[face_inds[i][0]], centroid, temp);

    if (glm_vec3_dot(temp, faces[i].norm) < -ZERO_THRESHOLD) {
      glm_vec3_negate(faces[i].norm);
    }

    triangulate_col_face(col, face_inds[i], faces[i].norm);
  }
}

static void triangulate_col_face(COLLIDER *col, unsigned int *unsorted,
                                 vec3 norm) {
  vec3 centroid = GLM_VEC3_ZERO_INIT;
  for (int i = 0; i < 4; i++) {
    glm_vec3_add(col->data.verts[unsorted[i]], centroid, centroid);
  }
  glm_vec3_scale(centroid, 1.0 / 4.0, centroid);

  vec3 u = GLM_VEC3_ZERO_INIT;
  glm_vec3_sub(col->data.verts[unsorted[0]], centroid, u);
  glm_vec3_normalize(u);

  vec3 v = GLM_VEC3_ZERO_INIT;
  glm_vec3_cross(u, norm, v);

  // Calculate angle of each point around vector from centroid to 1st pt
  float angles[4] = { 0.0, 0.0, 0.0, 0.0 };
  vec3 temp = GLM_VEC3_ZERO_INIT;
  for (int i = 1; i < 4; i++) {
    glm_vec3_sub(col->data.verts[unsorted[i]], centroid, temp);
    float x = glm_vec3_dot(temp, u);
    float y = glm_vec3_dot(temp, v);
    angles[i] = atan2(y, x);

    if (angles[i] < -ZERO_THRESHOLD) {
      angles[i] += (2.0 * GLM_PI);
    }
  }

  // Sort points by angle (first pt always has angle of 0.0)
  float min = 0.0;
  int min_idx = 0;
  unsigned int temp_pt = 0;
  for (int i = 1; i < 4; i++) {
    min = angles[i];
    min_idx = i;
    for (int j = i+1; j < 4; j++) {
      if (angles[j] < min) {
        min = angles[j];
        min_idx = j;
      }
    }

    temp_pt = unsorted[min_idx];
    angles[min_idx] = angles[i];
    unsorted[min_idx] = unsorted[i];
    angles[i] = min;
    unsorted[i] = temp_pt;
  }
}

static void orient_faces(unsigned int (*face_inds)[4],
                         unsigned int *oriented_faces) {
  int visited[6];
  memset(visited, 0, sizeof(visited));

  // Face made by face_inds[0] is base face (TOP)
  ivec2 ref_edge = { 0, 0 };
  oriented_faces[F_TOP] = 0;
  visited[0] = 1;

  // Face made by face_inds[0][0] and face_inds[0][1] is FRONT
  ref_edge[X] = face_inds[0][0];
  ref_edge[Y] = face_inds[0][1];
  oriented_faces[F_FRONT] = find_face_from_edge(face_inds, 0, ref_edge);
  visited[oriented_faces[F_FRONT]] = 1;

  // Face made by face_inds[0][2] and face_inds[0][3] is BACK
  ref_edge[X] = face_inds[0][2];
  ref_edge[Y] = face_inds[0][3];
  oriented_faces[F_BACK] = find_face_from_edge(face_inds, 0, ref_edge);
  visited[oriented_faces[F_BACK]] = 1;

  // Face made by face_inds[0][0] and face_inds[0][3] is RIGHT
  ref_edge[X] = face_inds[0][0];
  ref_edge[Y] = face_inds[0][3];
  oriented_faces[F_RIGHT] = find_face_from_edge(face_inds, 0, ref_edge);
  visited[oriented_faces[F_RIGHT]] = 1;

  // Face made by face_inds[0][1] and face_inds[0][2] is LEFT
  ref_edge[X] = face_inds[0][1];
  ref_edge[Y] = face_inds[0][2];
  oriented_faces[F_LEFT] = find_face_from_edge(face_inds, 0, ref_edge);
  visited[oriented_faces[F_LEFT]] = 1;

  // Remaining face is BOTTOM
  for (int i = 0; i < 6; i++) {
    if (!visited[i]) {
      oriented_faces[F_BOTTOM] = i;
    }
  }
}

static int find_face_from_edge(unsigned int (*faces)[4],
                               unsigned int src_idx, ivec2 edge) {
  for (int i = 0; i < 6; i++) {
    if (i == src_idx) {
      continue;
    }

    int found_x = 0;
    int found_y = 0;
    for (int j = 0; j < 4; j++) {
      if (faces[i][j] == edge[X]) {
        found_x = 1;
      }
      if (faces[i][j] == edge[Y]) {
        found_y = 1;
      }
    }

    if (found_x && found_y) {
      return i;
    }
  }

  return -1;
}

static void canonicalize_topology(COLLIDER *col, unsigned int (*face_inds)[4],
                                  unsigned int *oriented_faces,
                                  vec3 *sorted_verts) {
  // Vertex shared by the bottom, right and back face is the first vertex in
  // the canonical topology of the bottom face
  unsigned int r_idx = oriented_faces[F_RIGHT];
  unsigned int bot_idx = oriented_faces[F_BOTTOM];
  unsigned int back_idx = oriented_faces[F_BACK];

  unsigned int first_bot = -1;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 4; k++) {
        if (face_inds[r_idx][i] == face_inds[bot_idx][j] &&
            face_inds[back_idx][k] == face_inds[r_idx][i]) {
          first_bot = j;
          break;
        }
      }
    }
  }

  unsigned int top_idx = oriented_faces[F_TOP];
  for (int i = 0; i < 4; i++) {
    unsigned int b_idx = (first_bot + i) % 4;
    glm_vec3_copy(col->data.verts[face_inds[top_idx][i]], sorted_verts[i]);
    glm_vec3_copy(col->data.verts[face_inds[bot_idx][b_idx]],
                  sorted_verts[i+4]);
  }
}
