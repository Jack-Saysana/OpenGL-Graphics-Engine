#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <cglm/mat4.h>
#include <model_str.h>

typedef enum {
  AMB = 0,
  DIFF = 1,
  SPEC = 2,
  SPEC_EXPONENT = 3,
  BUMP = 4
} TEX_TYPE;

typedef struct entity {
  MODEL *model;
  mat4 (*bone_mats)[3];
  mat4 model_mat;
} ENTITY;

void draw_model(unsigned int shader, MODEL *model);
void draw_bones(MODEL *model);
void free_model(MODEL *model);
