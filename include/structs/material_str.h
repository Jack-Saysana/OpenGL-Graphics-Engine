#ifndef __MATERIAL_STR_H__
#define __MATERIAL_STR_H__

typedef enum {
  NO_OP = -2,
  NEWMTL = -1,
  AMB = 0,
  DIFF = 1,
  SPEC = 2,
  SPEC_EXPONENT = 3,
  BUMP = 4
} PROP_TYPE;

typedef struct material {
  uint64_t name;
  char *mat_paths[NUM_PROPS];
} MATERIAL;

#endif
