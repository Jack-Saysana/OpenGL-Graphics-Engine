#ifndef __ENGINE_CONST_H__
#define __ENGINE_CONST_H__

// Base settings for window resolution
#define BASE_RES_X (640.0)
#define BASE_RES_Y (400.0)

// Human-readable constants for accessing vectors
#define X (0)
#define Y (1)
#define Z (2)
#define W (3)

// Base lenge for allocating buffers
#define OCT_TREE_STARTING_LEN (25)
#define BUFF_STARTING_LEN (10)

// Entity constants
#define NUM_PROPS (5)
#define DEFAULT (0)
#define HIT_BOX (1)
#define HURT_BOX (2)

#define T_DYNAMIC (0x1)
#define T_DRIVING (0x2)
#define T_IMMUTABLE (0x4)

// Simulation constants
#define MAX_LINKED_SIMS (8)
#define SIM_RANGE_INF (~0)
#define ALLOW_DEFAULT    (0x1)
#define ALLOW_HURT_BOXES (0x2)
#define ALLOW_HIT_BOXES  (0x4)

// Physics constants
#define JOINT_PRISMATIC (0)
#define JOINT_REVOLUTE (1)
#define GRAVITY (10.0)

// Misc
#define INVALID_INDEX (~0)

#endif
