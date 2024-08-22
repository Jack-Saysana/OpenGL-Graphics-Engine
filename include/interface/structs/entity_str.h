#ifndef __ENGINE_ENTITY_STR_H__
#define __ENGINE_ENTITY_STR_H__

#include <cglm/mat4.h>
#include "../math/spatial_algebra.h"
#include "../const.h"
#include "./physics/constraint_str.h"

typedef enum chain_type {
  LOCATION = 0,
  ROTATION = 1,
  SCALE = 2
} C_TYPE;

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

typedef struct keyframe {
  float offset[4];
  int frame;
} KEYFRAME;

typedef struct keyframe_chain {
  KEYFRAME *chain;
  int *sled;
  size_t num_frames;
  C_TYPE type;
  unsigned int b_id;
} K_CHAIN;

typedef struct animation {
  K_CHAIN *keyframe_chains;
  size_t num_chains;
  size_t duration;
} ANIMATION;

typedef struct bone {
  // Rotates vectors in bone space to entity space
  mat3 coordinate_matrix;
  // in entity space
  vec3 head;
  vec3 base;
  int parent;
  int num_children;
} BONE;

typedef struct model {
  ANIMATION *animations;
  // Buffer containing each keyframe "chain" that corresponds to a bone and
  // type of animation (Scale, rotation, translation)
  K_CHAIN *k_chain_block;
  // Buffer containing the keyframes that make-up keyframe chains
  KEYFRAME *keyframe_block;
  // Buffer containing "sleds" which define, for each frame of animation, the
  // index of the keyframe that corresponds to the previous keyframe. Basically
  // a resource for easy keyframe interpolation
  int *sled_block;
  BONE *bones;
  COLLIDER *colliders;
  // Each element's index corresponds to the collider of the same index.
  // Element value corresponds to root bone represneted by collider
  int *collider_bone_links;
  // Each element's index corresponds to the bone of the same index.
  // Element value corresponds to collider represneted by bone
  int *bone_collider_links;
  size_t num_animations;
  size_t num_bones;
  size_t num_colliders;
  unsigned int textures[NUM_PROPS];
  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
  unsigned int num_indicies;
} MODEL;

typedef struct zero_joint_data {
  mat6 I_hat_A;
  mat6 ST_to_parent;
  mat6 ST_from_parent;
  mat3 bone_to_world;
  mat3 rot_mat;
  vec6 s_hat;
  vec6 Z_hat;
  vec6 Z_hat_A;
  vec6 c_hat;
  vec6 a_hat;
  vec6 v_hat;
  vec6 s_inner_I;
  vec3 dof;
  vec3 joint_to_com;
  size_t next_offset;
  int joint_type;
  float s_inner_I_dot_s;
  float SZI;
  float accel_angle;
  float vel_angle;
  float joint_angle;
} ZERO_JOINT;

typedef struct p_data {
  // Spatial inertia
  mat6 I_hat;
  // Articulated spatial inertia
  mat6 I_hat_A;
  // Spatial tranformation from the current links inertial frame to its
  // parent's inertial frame
  mat6 ST_to_parent;
  // Spatial transformation to the current link's inertial frame from its
  // parent's inertial frame
  mat6 ST_from_parent;
  // Inverse inertia tensor, given in bone space
  mat4 inv_inertia;
  // Spatial joint axis
  vec6 s_hat;
  // Spatial zero acceleration
  vec6 Z_hat;
  // Articulated spatial zero acceleration
  vec6 Z_hat_A;
  // Coriolis vector
  vec6 coriolis_vector;
  // Spatial acceleration
  vec6 a_hat;
  // Spatial velocity
  vec6 v_hat;
  // Shortcut of s'I_hat_A
  vec6 s_inner_I;
  // Read-only world-space linear/angular accleration/velocity
  vec3 a;
  vec3 ang_a;
  vec3 v;
  vec3 ang_v;
  // Buffer specifiying the degree of freedom for link
  vec3 dof;
  // Vector pointing from link parent's COM to current link's COM in bone space
  vec3 from_parent_lin;
  // Vector pointing from link's joint to link's COM in bone space
  vec3 joint_to_com;
  // External forces exerted on the joint
  vec3 e_force;
  // Offset in entity zero-joint buffer to first zero joint
  size_t zero_joint_offset;
  size_t num_z_joints;
  // Specifies if the joint is revolute or prismatic
  int joint_type;
  float inv_mass;
  // Shortcut for dot(s'I_hat_A, s_hat)
  float s_inner_I_dot_s;
  // Shortcut for s'(Z_hat_A + I_hat(coriolis))
  float SZI;
  // Joint angle acceleration
  float accel_angle;
  // Joint angle velocity
  float vel_angle;
  // Joint angle
  float joint_angle;
} P_DATA;

typedef struct entity {
  // Pointer to miscelaneous data to link entity to other information
  void *data;
  MODEL *model;
  // Physics constraint buffer to be applied for the current frame
  J_CONS *p_cons;
  size_t num_cons;
  size_t cons_size;
  // Location, rotation and scale matricies for each bone
  mat4 (*bone_mats)[3];
  // "Narrow" physics data for each collider
  P_DATA *np_data;
  // Physics data for zero-joints that provide additional dofs to colliders
  ZERO_JOINT *zj_data;
  // Model matrix for each bone, including those inherited by parent bones
  mat4 *final_b_mats;
  // Physics system status
  // Bit layout: 0...0[MUTABLE/IMMUTABLE][DRIVEN/DRIVING][STATIC/DYNAMIC]
  int type;
} ENTITY;

#endif
