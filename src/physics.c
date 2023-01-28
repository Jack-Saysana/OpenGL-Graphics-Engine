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

  glm_vec3_scale(dir, -1.0, temp);
  if (b->type == SPHERE) {
    glm_vec3_scale_as(dir, b->data.radius, max_b);
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
