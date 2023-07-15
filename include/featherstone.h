#include <entity_str.h>

static const vec3 BASIS_VECTORS[3] = {
  {1.0, 0.0, 0.0},
  {0.0, 1.0, 0.0},
  {0.0, 0.0, 1.0},
};

#define GRAVITY (10.0)
static const vec3 G_VEC = { 0.0, -GRAVITY, 0.0 };
