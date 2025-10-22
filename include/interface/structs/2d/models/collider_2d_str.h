#ifndef __COLLIDER_2D_STR_H__
#define __COLLIDER_2D_STR_H__

typedef enum collider_type_2d {
  CT_SQUARE,
  CT_CIRCLE,
  CT_LINE
} COL_TYPE_2D;

typedef struct collider_2d {
  vec2 origin;
  union {
    struct { // Square
      float width;
      float height;
    };
    struct { // Circle
      float radius;
    };
    struct { // Line
      vec2 end;
    };
  } data;

  COL_TYPE_2D type;
  int category;
} COLLIDER_2D;

#endif
