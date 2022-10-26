#include <glad/glad.h>

typedef struct model {
  unsigned int VAO;
  unsigned int num_indicies;
} MODEL;

void draw_model(MODEL *model);
