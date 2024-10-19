#ifndef __CONST_H__
#define __CONST_H__

#define BASE_RES_X (640.0)
#define BASE_RES_Y (400.0)

#define X (0)
#define Y (1)
#define Z (2)

#define LINE_BUFF_STARTING_LEN (50)
#define FILE_CONTENTS_STARTING_LEN (500)

#define HASH_MAP_STARTING_LEN (50)
#define OCT_TREE_STARTING_LEN (25)
#define BUFF_STARTING_LEN (10)

// Material constants
#define NUM_PROPS (5)

// Simulation constants
#define NUM_PROPS (5)
#define DEFAULT (0)
#define HIT_BOX (1)
#define HURT_BOX (2)

#define T_DYNAMIC (0x1)
#define T_DRIVING (0x2)
#define T_IMMUTABLE (0x4)

#define SIM_RANGE_INF (0xFFFFFFFF)
#define ALLOW_DEFAULT    (0x1)
#define ALLOW_HURT_BOXES (0x2)
#define ALLOW_HIT_BOXES  (0x4)

#define INVALID_INDEX (0xFFFFFFFFFFFFFFFF)
#define INVALID_TEX (0xFFFFFFFF)

#define GRAVITY (10.0)

#define L_TYPE_ENTITY (0)
#define L_TYPE_COLLIDER (1)

// Simulation hash-map constants
#define HASH_MAGIC_NUM (0.618033988749894)
#define LEDGER_FREE (0)
#define LEDGER_OCCUPIED (1)
#define LEDGER_DELETED (2)

// UI constants
#define ROOT_UI_DEPTH (-10.0)

// Misc constants
//#define DEBUG_OCT_TREE (1)

// Physics Constants
#define LINEAR_DAMP_FACTOR (0.99)
#define ANGULAR_DAMP_FACTOR (0.97)
#define JOINT_PRISMATIC (0)
#define JOINT_REVOLUTE (1)
#define MOVING_THRESHOLD (0.01)
#define CONSTRAINT_ERROR_THRESHOLD (0.4)

// Math Constants
#define MAX_SVD_ITERATIONS (30)
#define MAX_LCP_ITERATIONS (100)
#define ZERO_THRESHOLD (0.00001)
#define LCP_C (0)
#define LCP_NC (1)
#define LCP_EQ (0)
#define LCP_GEQ (1)

#endif
