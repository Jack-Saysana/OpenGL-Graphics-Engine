#include <2d/physics/collision_2d.h>

int collision_check_2d(COLLIDER_2D *a, COLLIDER_2D *b, vec2 correction) {
  glm_vec2_zero(correction);
  if (a->type == SQUARE && b->type == SQUARE) {
    return aabb_collision(a->origin, a->data.width / 2.0, a->data.height / 2.0,
                          b->origin, b->data.width / 2.0, b->data.width / 2.0,
                          correction);
  }
  if (a->type == CIRCLE && b->type == CIRCLE) {
    return circle_collision(a->origin, a->data.radius, b->origin,
                            b->data.radius, correction);
  }
  if (a->type == LINE && b->type == LINE) {
    return line_collision(a->origin, a->data.end, b->origin, b->data.end);
  }
  if (a->type == SQUARE && b->type == CIRCLE) {
    return aabb_circle_collision(b->origin, b->data.radius, a->origin,
                                 a->data.width / 2.0, a->data.height / 2.0,
                                 correction);
  }
  if (a->type == SQUARE && b->type == LINE) {
    return line_aabb_collision(b->origin, b->data.end, a->origin,
                               a->data.width / 2.0, a->data.height / 2.0);
  }
  if (a->type == CIRCLE && b->type == SQUARE) {
    int col = aabb_circle_collision(a->origin, a->data.radius, b->origin,
                                    b->data.width / 2.0, b->data.height / 2.0,
                                    correction);
    // Ensure correction points from B into A
    glm_vec2_negate(correction);
    return col;
  }
  if (a->type == CIRCLE && b->type == LINE) {
    return line_circle_collision(b->origin, b->data.end, a->origin,
                                 a->data.radius);
  }
  if (a->type == LINE && b->type == SQUARE) {
    return line_aabb_collision(a->origin, a->data.end, b->origin,
                               b->data.width / 2.0, b->data.height / 2.0);
  }
  if (a->type == LINE && b->type == CIRCLE) {
    return line_circle_collision(a->origin, a->data.end, b->origin,
                                 b->data.radius);
  }

  return 0;
}

static int aabb_collision(vec2 a_center, float a_hw, float a_hh, vec2 b_center,
                          float b_hw, float b_hh, vec2 correction) {
  float overlap_x = a_hw + b_hw - fabs(a_center[X] - b_center[X]);
  float overlap_y = a_hh + b_hh - fabs(a_center[Y] - b_center[Y]);

  if (overlap_x > 0.0 && overlap_y > 0.0) {
    if (overlap_x > overlap_y) {
      correction[X] = overlap_x;
      correction[Y] = 0.0;
    } else {
      correction[X] = 0.0;
      correction[Y] = overlap_y;
    }
    return 1;
  }

  return 0;
}

static int circle_collision(vec2 a_center, float a_rad, vec2 b_center,
                            float b_rad, vec2 correction) {
  float dist = glm_vec2_distance(a_center, b_center);
  if (dist < a_rad + b_rad) {
    float correction_distance = a_rad + b_rad - dist;
    glm_vec2_sub(b_center, a_center, correction);
    glm_vec2_scale_as(correction, correction_distance, correction);
    return 1;
  }

  return 0;
}

static int line_collision(vec2 a1, vec2 a2, vec2 b1, vec2 b2) {
  float a2_sub_a1_x = a2[X] - a1[X];
  float a2_sub_a1_y = a2[Y] - a1[Y];
  float b2_sub_b1_x = b2[X] - b1[X];
  float b2_sub_b1_y = b2[Y] - b1[Y];
  float k = ((b1[Y] - a1[Y]) * a2_sub_a1_x) + ((a1[X] - b1[X]) * a2_sub_a1_y);
  float k_div = ((b2_sub_b1_x * a2_sub_a1_y) - (b2_sub_b1_y * a2_sub_a1_x));
  if (!k_div || !a2_sub_a1_x) {
    return 0;
  }
  k = k / k_div;

  float j = (b1[X] + (k * b2_sub_b1_x) - a1[X]) / a2_sub_a1_x;

  return j >= 0.0 && j <= 1.0 && k >= 0.0 && k<= 1.0;
}

static int aabb_circle_collision(vec2 c_center, float c_rad, vec2 b_center,
                                 float b_hw, float b_hh, vec2 correction) {
  float by_max = b_center[Y] + b_hh;
  float by_min = b_center[Y] - b_hh;
  float bx_max = b_center[X] + b_hw;
  float bx_min = b_center[X] - b_hw;

  vec2 closest_aabb_point = {
    fmax(bx_min, fmin(bx_max, c_center[X])),
    fmax(by_min, fmin(by_max, c_center[Y]))
  };

  float dist_circle_to_closest = glm_vec2_distance(closest_aabb_point,
                                                   c_center);
  float correction_distance = c_rad - dist_circle_to_closest;
  if (correction_distance > 0.0) {
    // Vector will point from the square, into the circle
    glm_vec2_sub(c_center, closest_aabb_point, correction);
    glm_vec2_scale_as(correction, correction_distance, correction);
    return 1;
  }

  return 0;
}

static int line_aabb_collision(vec2 a1, vec2 a2, vec2 b_center, float b_hw,
                               float b_hh) {
  return 0;
}

static int line_circle_collision(vec2 a1, vec2 a2, vec2 b_center,
                                 float b_rad) {
  float A = a2[X] - a1[X];
  float B = a1[X] - b_center[X];
  float C = a2[Y] - a1[Y];
  float D = a1[Y] - b_center[Y];

  // Quadratic formula
  float quad_a = (A * A) + (C * C);
  float quad_b = 2.0 * ((A * B) + (C * D));
  float quad_c = (D * D) + (B * B) - (b_rad * b_rad);

  float root = (quad_b * quad_b) - (4.0 * quad_a * quad_c);
  if (!root || !quad_a) {
    return 0;
  }

  float j1 = (-quad_b + sqrt(root)) / (2.0 * quad_a);
  float j2 = (-quad_b - sqrt(root)) / (2.0 * quad_a);

  if ((j1 >= 0.0 && j1 <= 1.0) || (j2 >= 0.0 && j2 <= 1.0) ||
      (j1 >= 1.0 && j2 <= 0.0) || (j2 >= 1.0 && j1 <= 0.0)){
    return 1;
  } else {
    return 0;
  }
}
