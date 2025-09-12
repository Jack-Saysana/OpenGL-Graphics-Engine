#include <2d/physics/collision_2d.h>

int collision_check_2d(COLLIDER_2D *a, COLLIDER_2D *b, vec2 correction) {
  if (a->type == SQUARE && b->type == SQUARE) {
    return aabb_collision(a->center, a->data.width / 2.0, a->data.height / 2.0,
                          b->center, b->data.width / 2.0, b->data.width / 2.0,
                          correction);
  }
  if (a->type == CIRCLE && b->type == CIRCLE) {
    return circle_collision(a->center, a->data.radius, b->center,
                            b->data.radius, correction);
  }
  if (a->type == SQUARE) {
    return aabb_circle_collision(b->center, b->data.radius, a->center,
                                 a->data.width / 2.0, a->data.height / 2.0,
                                 correction);
  }
  int col = aabb_circle_collision(a->center, a->data.radius, b->center,
                                  b->data.width / 2.0, b->data.height / 2.0,
                                  correction);
  // Ensure correction points from B into A
  glm_vec2_negate(correction);
  return col;
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

static int aabb_circle_collision(vec2 c_center, float c_rad, vec2 b_center,
                                 float b_hw, float b_hh, vec2 correction) {
  float by_max = b_center[Y] + b_hh;
  float by_min = b_center[Y] - b_hh;
  float bx_max = b_center[X] + b_hw;
  float bx_min = b_center[X] + b_hw;

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
