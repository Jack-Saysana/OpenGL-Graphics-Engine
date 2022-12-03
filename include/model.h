#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>


#define NUM_PROPS (5)
typedef enum {
  AMB = 0,
  DIFF = 1,
  SPEC = 2,
  SPEC_EXPONENT = 3,
  BUMP = 4
} TEX_TYPE;

typedef struct bone {
  float coords[3];
  int num_children;
} BONE;

typedef struct model {
  unsigned int textures[NUM_PROPS];
  BONE *bones;
  unsigned int VAO;
  size_t num_bones;
  unsigned int num_indicies;
} MODEL;

void draw_model(unsigned int shader, MODEL *model);
void draw_bones(MODEL *model);
void free_model(MODEL *model);
