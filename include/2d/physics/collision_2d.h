#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include <const.h>
#include <structs/2d/models/collider_2d_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

static int aabb_collision(vec2 a_center, float a_hw, float a_hh, vec2 b_center,
                          float b_hw, float b_hh, vec2 correction);
static int circle_collision(vec2 a_center, float a_rad, vec2 b_center,
                            float b_rad, vec2 correction);
static int aabb_circle_collision(vec2 c_center, float c_rad, vec2 b_center,
                                 float b_hw, float b_hh, vec2 correction);
static int line_collision(vec2 a1, vec2 a2, vec2 b1, vec2 b2);
static int line_aabb_collision(vec2 a1, vec2 a2, vec2 b_center, float b_hw,
                               float b_hh);
static int line_circle_collision(vec2 a1, vec2 a2, vec2 b_center, float b_rad);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================


