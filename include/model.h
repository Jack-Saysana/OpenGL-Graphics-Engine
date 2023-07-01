#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <entity_str.h>

typedef enum {
  AMB = 0,
  DIFF = 1,
  SPEC = 2,
  SPEC_EXPONENT = 3,
  BUMP = 4
} TEX_TYPE;

void draw_model(unsigned int shader, MODEL *model);
void draw_bones(MODEL *model);
void free_model(MODEL *model);
