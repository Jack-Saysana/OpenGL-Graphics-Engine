#ifndef __CONST_H__
#define __CONST_H__

#define BASE_RES_X (640.0)
#define BASE_RES_Y (400.0)

#define X (0)
#define Y (1)
#define Z (2)

#define BUFF_START_LEN (5)

#define HASH_MAP_STARTING_LEN (50)
#define OCT_TREE_STARTING_LEN (25)
#define BUFF_STARTING_LEN (10)

#define NUM_PROPS (5)
#define DEFAULT (0)
#define HIT_BOX (1)
#define HURT_BOX (2)

#define T_DYNAMIC (0x1)
#define T_DRIVING (0x2)
#define T_IMMUTABLE (0x4)

#define PHYS_TREE (0)
#define HIT_TREE (1)
#define EVENT_TREE (2)

#define SIM_RANGE_INF (0xFFFFFFFF)
#define ALLOW_DEFAULT    (0x1)
#define ALLOW_HURT_BOXES (0x2)
#define ALLOW_HIT_BOXES  (0x4)

#define INVALID_INDEX (0xFFFFFFFFFFFFFFFF)

#define GRAVITY (10.0)

// Simulation hash-map constants
#define HASH_MAGIC_NUM (0.618033988749894)
#define HASH_CONST_1 (11)
#define HASH_CONST_2 (29)
#define LEDGER_FREE (0)
#define LEDGER_OCCUPIED (1)
#define LEDGER_DELETED (2)

#endif
