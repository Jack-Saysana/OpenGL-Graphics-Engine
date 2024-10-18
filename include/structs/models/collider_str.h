#ifndef __COLLIDER_STR_H__
#define __COLLIDER_STR_H__

typedef enum collider_type {
  POLY,
  SPHERE
} COL_TYPE;

typedef struct collider {
  // Position in the model collider buffer of the colliders child list
  int children_offset;
  size_t num_children;
  // Verts given in bone space
  // center_of_mass and center given in entity space (origin in bone space)
  union {
    // POLYHEDRON DATA
    struct {
      vec3 verts[8];
      vec3 center_of_mass;
      unsigned int num_used;
    };
    // SPHERE DATA
    struct {
      vec3 center;
      float radius;
    };
  } data;
  COL_TYPE type;
  int category;
} COLLIDER;

#endif
