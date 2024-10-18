#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <structs/models/entity_str.h>

typedef enum {
  AMB = 0,
  DIFF = 1,
  SPEC = 2,
  SPEC_EXPONENT = 3,
  BUMP = 4
} TEX_TYPE;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

void set_vec3(char *loc, vec3 vec, unsigned int shader);
