#ifndef __COLLIDER_STR_H__
#define __COLLIDER_STR_H__

typedef enum collider_type {
  POLY,
  SPHERE
} COL_TYPE;

typedef struct collider {
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
  // Vectors specifying the degrees of freedom for the collider. Layout:
  // [ DOF.X, DOF.Y, DOF.Z, TYPE ]
  // The first 3 floats indicate the actual axis of motion, while the last
  // indicates the type of joint, i.e JOINT_REVOLUTE or JOINT_PRISMATIC
  vec4 dofs[6];
  size_t num_children;
  // Position in the model collider buffer of the colliders child list
  int children_offset;
  // Must be at least 1 but no more than 6
  int num_dofs;
  COL_TYPE type;
  int category;
} COLLIDER;

#endif
