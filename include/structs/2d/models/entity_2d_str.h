#ifndef __ENTITY_2D_STR_H__
#define __ENTITY_2D_STR_H__

#include <structs/2d/models/collider_2d_str.h>

typedef struct entity_2d {
  void *data;
  void (*move_cb)(struct entity_2d *, vec2);
  int (*is_moving_cb)(struct entity_2d *, size_t);
  COLLIDER_2D *cols;
  size_t num_cols;

  vec3 pos;
  float rot;
  float height;
  float width;
  int type;
} ENTITY_2D;

#endif
