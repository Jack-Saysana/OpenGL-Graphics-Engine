#define NUM_PROPS (5)
#define DEFAULT (0)
#define HIT_BOX (1)
#define HURT_BOX (2)

#define T_DYNAMIC (1)
#define T_DRIVING (2)
#define T_IMMUTABLE (4)

#define PHYS_TREE (0)
#define HIT_TREE (1)
#define EVENT_TREE (2)

#define INVALID (0)
#define INVALID_VAL (0xBAADF00D)

#define NUM_OCT_TREES (3)

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
  // Position of bone base (point at which parent connects to the bone)
  // in entity space
  vec3 coords;
  // Position of bone tail (point at which children extend from the bone)
  // in entity space
  vec3 tail;
  int parent;
  int num_children;
} BONE;

typedef struct model {
  ANIMATION *animations;
  /* Buffer containing each keyframe "chain" that corresponds to a bone and
     type of animation (Scale, rotation, translation) */
  K_CHAIN *k_chain_block;
  /* Buffer containing the keyframes that make-up keyframe chains */
  KEYFRAME *keyframe_block;
  /* Buffer containing "sleds" which define, for each frame of animation, the
     index of the keyframe that corresponds to the previous keyframe. Basically
     a resource for easy keyframe interpolation */
  int *sled_block;
  BONE *bones;
  /* Each element's index corresponds to the bone of the same index. Element
     value consists of an array containing indicies of children */
  int **bone_relations;
  COLLIDER *colliders;
  /* Each element's index corresponds to the collider of the same index.
     Element value corresponds to bone represneted by collider */
  int *collider_bone_links;
  size_t num_animations;
  size_t num_bones;
  size_t num_colliders;
  unsigned int textures[NUM_PROPS];
  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
  unsigned int num_indicies;
} MODEL;

typedef struct p_data {
  mat4 inv_inertia;
  vec3 velocity;
  vec3 ang_velocity;
  float inv_mass;
} P_DATA;

typedef struct entity {
  MODEL *model;
  /* Index corresponds to the model collider of the same index.
     Values of the 3-sized array correspond to the offset of the collider in
     the given oct tree (0: Physics, 1: Hit-Hurt, 2: Misc. Event) */
  size_t (*tree_offsets)[3];
  /* Positions of entity in the physics system's dynamic entity buffer and
     driving entity buffer (if applicable) */
  size_t list_offsets[2];
  /* Location, rotation and scale matricies for each bone */
  mat4 (*bone_mats)[3];
  /* "Narrow" physics data for each bone */
  P_DATA *np_data;
  /* Model matrix for each bone, including those inherited by parent bones */
  mat4 *final_b_mats;
  mat4 inv_inertia;
  /* Broad entity-based transformations */
  versor rotation;
  vec3 scale;
  vec3 translation;
  /* "Broad" physics data. Used when entire entity is a single physics
     object */
  vec3 velocity;
  vec3 ang_velocity;
  float inv_mass;
  /* Physics system status
     Bit layout: 0...0[MUTABLE/IMMUTABLE][DRIVEN/DRIVING][STATIC/DYNAMIC] */
  int type;
} ENTITY;
