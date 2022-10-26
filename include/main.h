#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <math.h>

typedef struct model {
  unsigned int VAO;
  unsigned int num_indicies;
} MODEL;

void framebuffer_size_callback(GLFWwindow *, int, int);
unsigned int init_shader_prog(char *, char *, char *);
MODEL *load_model(char *path);
void draw_model(MODEL *model);
