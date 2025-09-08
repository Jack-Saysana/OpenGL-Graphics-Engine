#ifndef __COLLIDER_2D_STR_H__
#ifndef __COLLIDER_2D_STR_H__

typedef enum collider_type {
  POLY,
  SPHERE
} COL_TYPE;

typedef struct collider_2d {
  vec2 center;
  float radius;

  COL_TYPE type;
  int category;
} COLLIDER_2D;

#endif
