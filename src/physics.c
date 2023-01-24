#include <physics.h>

/*
 * Implementation of the Gilbert-Johnson-Keerthi (GJK) algorithm for
 * collision detection. Refer to personal notes for in-depth details.
 */
int collision_check(COLLIDER *a, COLLIDER *b) {
  if (a == NULL || b == NULL || (a->type == POLY && a->data.num_used > 8) ||
      (b->type == POLY && b->data.num_used > 8) || (a->type == SPHERE &&
      a->data.radius < 0.0) || (b->type == SPHERE && b->data.radius < 0.0)) {
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
    glm_vec3_scale_as(dir, a->data.radius, dest);
    glm_vec3_add(dest, a->data.center, dest);
  } else {
    glm_vec3_copy(a->data.verts[max_dot(a, dir)], max_a);
  }

  glm_vec3_scale(dir, -1.0, temp);
  if (b->type == SPHERE) {
    glm_vec3_scale_as(dir, b->data.radius, dest);
    glm_vec3_add(dest, b->data.center, dest);
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

