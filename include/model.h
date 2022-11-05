#include <glad/glad.h>

#define NUM_PROPS (5)
typedef enum {
  AMB = 0,
  DIFF = 1,
  SPEC = 2,
  SPEC_EXPONENT = 3,
  BUMP = 4
} TEX_TYPE;

typedef struct model {
  unsigned int textures[NUM_PROPS];
  unsigned int VAO;
  unsigned int num_indicies;
} MODEL;

void draw_model(unsigned int shader, MODEL *model);
