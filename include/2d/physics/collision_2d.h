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

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================


