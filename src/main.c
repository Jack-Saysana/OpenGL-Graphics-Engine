#include <main.h>

vec3 camera_pos = { 0.0, 0.0, 3.0 };
vec3 camera_front = { 0.0, 0.0, -1.0 };
vec3 camera_up = { 0.0, 1.0, 0.0 };
vec3 center = { 0.0, 0.0, 0.0 };

void keyboard_input(GLFWwindow *window);

int main() {
  GLFWwindow *window;

  if (!glfwInit()) {
    return -1;
  }

  window = glfwCreateWindow(640, 480, "Jack", NULL, NULL);
  if (window == NULL) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    glfwTerminate();
    return -1;
  }

  glViewport(0, 0, 640, 480);

  unsigned int shader = init_shader_prog(
      "/home/jack/projects/OpenGL-Graphics-Engine/src/shaders/phong/shader.vs",
      NULL,
      "/home/jack/projects/OpenGL-Graphics-Engine/src/shaders/phong/shader.fs"
      );

  MODEL *cube = load_model(
      "/home/jack/projects/OpenGL-Graphics-Engine/resources/Cube/cube.obj"
      );

  mat4 projection = {
    { 1.0, 0.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0, 0.0 },
    { 0.0, 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 0.0, 1.0 }
  };
  glm_perspective_default(640.0 / 480.0, projection);
  glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1,
                     GL_FALSE, (float *)projection);

  mat4 view = {
    { 1.0, 0.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0, 0.0 },
    { 0.0, 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 0.0, 1.0 }
  };

  mat4 model = {
    { 1.0, 0.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0, 0.0 },
    { 0.0, 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 0.0, 1.0 }
  };
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1,
                     GL_FALSE, (float *)model);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);

    keyboard_input(window);

    // Render
    glm_vec3_add(camera_pos, camera_front, center);
    glm_lookat(camera_pos, center, camera_up, view);

    //printf("==========\npos: %f %f %f\ncenter: %f %f %f\nup: %f %f %f\n",
    //      camera_pos[0], camera_pos[1], camera_pos[2], center[0], center[1],
    //      center[2], camera_up[0], camera_up[1], camera_up[2]);
    /*printf("==========\n%f %f %f\n% f %f %f\n%f %f %f\n", view[0][0],
           view[0][1], view[0][2], view[1][0], view[1][1], view[1][2],
           view[2][0], view[2][1], view[2][2]);*/
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1,
                       GL_FALSE, (float *)view);

    glUseProgram(shader);
    draw_model(cube);

    // Swap Buffers and Poll Events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  free(cube);
  glfwTerminate();
  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void keyboard_input(GLFWwindow *window) {
  vec3 movement = { 0.0, 0.0, 0.0 };
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    glm_vec3_scale(camera_front, 0.05, movement);
    glm_vec3_add(camera_pos, movement, camera_pos);
    printf("Cam pos: %f, %f, %f\n", camera_pos[0], camera_pos[1],
           camera_pos[2]);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    printf("s\n");
    glm_vec3_scale(camera_front, 0.05, movement);
    glm_vec3_sub(camera_pos, movement, camera_pos);
    printf("Cam pos: %f, %f, %f\n", camera_pos[0], camera_pos[1],
           camera_pos[2]);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    printf("a\n");
    glm_vec3_cross(camera_front, camera_up, movement);
    glm_vec3_normalize(movement);
    glm_vec3_scale(movement, 0.05, movement);
    glm_vec3_sub(camera_pos, movement, camera_pos);
    printf("Cam pos: %f, %f, %f\n", camera_pos[0], camera_pos[1],
           camera_pos[2]);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    printf("d\n");
    glm_vec3_cross(camera_front, camera_up, movement);
    glm_vec3_normalize(movement);
    glm_vec3_scale(movement, 0.05, movement);
    glm_vec3_add(camera_pos, movement, camera_pos);
    printf("Cam pos: %f, %f, %f\n", camera_pos[0], camera_pos[1],
           camera_pos[2]);
  }
}

/*void mouse_input(GLFWwidnow *window, double xpos, double ypos) {

}*/
