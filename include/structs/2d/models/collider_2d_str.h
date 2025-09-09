#ifndef __COLLIDER_2D_STR_H__
#define __COLLIDER_2D_STR_H__

typedef enum collider_type_2d {
  SQUARE,
  CIRCLE
} COL_TYPE_2D;

typedef struct collider_2d {
  vec2 center;
  union {
    struct {
      float width;
      float height;
    };
    struct {
      float radius;
    };
  } data;

  COL_TYPE_2D type;
  int category;
} COLLIDER_2D;

#endif
