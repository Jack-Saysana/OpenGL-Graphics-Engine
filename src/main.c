#include <main.h>

int main()
{
  GLFWwindow *window;

  if (!glfwInit())
  {
    return -1;
  }

  if (!gladLoadGLLoader((GLADLoadproc) glfwGetProcAddress))
  {
    glfwTerminate();
    return -1;
  }

  window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
  if (window == NULL)
  {
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  glViewport(0, 0, 640, 480);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  while (!glfwWindowShouldClose(window))
  {
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

int framebuffer_size_callback(GLFWwindow window, int width, int height)
{
  glViewport(0, 0, width, height);
}
