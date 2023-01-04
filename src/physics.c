#include <physics.h>

int oct_tree_insert(OCT_TREE *tree, PHYS_OBJ *obj) {
  if (tree == NULL || obj == NULL) {
    printf("Tree or object null\n");
    return -1;
  }

  vec3 min_extent = { 16.0, -16.0, -16.0 };
  vec3 max_extent = { 16.0, 16.0, 16.0 };
  vec3 temp_extents[2];
  float child_size = 16.0;

  COLLIDER cur_oct;
  cur_oct.num_used = 8;

  int oct = -1;
  int num_inside = 0;

  int inserting = 1;
  while (inserting) {
    for (int i = 0; i < 8; i++) {
      
    }
  }
}

int oct_tree_delete(OCT_TREE *tree, int node_offset, int obj_offset) {
}

COLLISION_RES oct_tree_search(OCT_TREE *tree, COL_PLANE *hit_box,
                              unsigned int num_planes) {
}

void create_aabb(COLLIDER *a, vec3 min, vec3 max) {
  float diff = max.x - min.x;
  a->verts[0][0] = min[0];
  a->verts[0][1] = min[1];
  a->verts[0][2] = min[2];
  a->verts[1][0] = min[0] + diff;
  a->verts[1][1] = min[1];
  a->verts[1][2] = min[2];
  a->verts[2][0] = min[0];
  a->verts[2][1] = min[1];
  a->verts[2][2] = min[2] + diff;
  a->verts[3][0] = min[0] + diff;
  a->verts[3][1] = min[1];
  a->verts[3][2] = min[2] + diff;
  a->verts[4][0] = min[0];
  a->verts[4][1] = min[1] + diff;
  a->verts[4][2] = min[2];
  a->verts[5][0] = min[0] + diff;
  a->verts[5][1] = min[1] + diff;
  a->verts[5][2] = min[2];
  a->verts[6][0] = min[0];
  a->verts[6][1] = min[1] + diff;
  a->verts[6][2] = min[2] + diff;
  a->verts[7][0] = max[0];
  a->verts[7][1] = max[1];
  a->verts[7][2] = max[2];
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
  float dot_a = -FLT_MAX;
  vec3 max_b = { 0.0, 0.0, 0.0 };
  float dot_b = -FLT_MAX;

  float temp = 0.0;
  vec3 temp_vec = { 0.0, 0.0, 0.0 };

  for (unsigned int i = 0; i < a->num_used; i++) {
    temp = glm_vec3_dot(dir, a->verts[i]);
    if(temp > dot_a) {
      dot_a = temp;
      max_a[0] = a->verts[i][0];
      max_a[1] = a->verts[i][1];
      max_a[2] = a->verts[i][2];
    }
  }

  glm_vec3_scale(dir, -1.0, temp_vec);
  for (unsigned int i = 0; i < b->num_used; i++) {
    temp = glm_vec3_dot(temp_vec, b->verts[i]);
    if(temp > dot_b) {
      dot_b = temp;
      max_b[0] = b->verts[i][0];
      max_b[1] = b->verts[i][1];
      max_b[2] = b->verts[i][2];
    }
  }

  dest[0] = max_a[0] - max_b[0];
  dest[1] = max_a[1] - max_b[1];
  dest[2] = max_a[2] - max_b[2];
}
