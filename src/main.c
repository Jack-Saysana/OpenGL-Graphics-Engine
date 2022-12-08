#include <main.h>

void keyboard_input(GLFWwindow *window);
void mouse_input(GLFWwindow *window, double xpos, double ypos);

vec3 camera_pos = { 0.0, 0.0, 3.0 };
vec3 camera_front = { 0.0, 0.0, -1.0 };
vec3 camera_up = { 0.0, 1.0, 0.0 };
vec3 center = { 0.0, 0.0, 0.0 };

float lastX = 400;
float lastY = 300;

float pitch = 0;
float yaw = 0;

int firstMouse = 1;

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

  glfwSetInputMode(window, GLFW_CURSOR,GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_input);

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    glfwTerminate();
    return -1;
  }

  glViewport(0, 0, 640, 480);

  unsigned int shader = init_shader_prog(
      "C:/Users/Jack/Documents/C/OpenGL-Graphics-Engine/src/shaders/test/shader.vs",
      //"C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/src/shaders/test/shader.vs",
      //"C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/src/shaders/phong/shader.vs",
      NULL,
      "C:/Users/Jack/Documents/C/OpenGL-Graphics-Engine/src/shaders/test/shader.fs"
      //"C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/src/shaders/test/shader.fs"
      //"C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/src/shaders/phong/shader.fs"
      );
  if (shader == -1) {
    printf("Error loading shaders\n");
    glfwTerminate();
    return -1;
  }

  unsigned int b_shader = init_shader_prog(
      "C:/Users/Jack/Documents/C/OpenGL-Graphics-Engine/src/shaders/bone/shader.vs",
      //"C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/src/shaders/bone/shader.vs",
      NULL,
      "C:/Users/Jack/Documents/C/OpenGL-Graphics-Engine/src/shaders/bone/shader.fs"
      //"C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/src/shaders/bone/shader.fs"
      );
  if (b_shader == -1) {
    printf("Error loading bone shaders\n");
    glfwTerminate();
    return -1;
  }

  MODEL *cube = load_model(
      "C:/Users/Jack/Documents/C/OpenGL-Graphics-Engine/resources/cube/cube.obj"
      //"C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/resources/cube/cube.obj"
      );
  if (cube == NULL) {
    printf("Unable to load model\n");
    glfwTerminate();
    return -1;
  }

  MODEL *cross = load_model(
      "C:/Users/Jack/Documents/C/OpenGL-Graphics-Engine/resources/cross/cross.obj"
      //"C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/resources/cross/cross.obj"
      );
  if (cross == NULL) {
    printf("Unable to load model\n");
    glfwTerminate();
    return -1;
  }

  MODEL *dude = load_model(
      "C:/Users/Jack/Documents/C/OpenGL-Graphics-Engine/resources/low_poly_new/low_poly_new.obj"
      //"C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/resources/low_poly_new/low_poly_new.obj"
      );
  if (dude == NULL) {
    printf("Unable to load model\n");
    glfwTerminate();
    return -1;
  }

  MODEL *test = load_model(
      "C:/Users/Jack/Documents/C/OpenGL-Graphics-Engine/resources/test/test.obj"
      //"C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/resources/test/test.obj"
      );
  if (test == NULL) {
    printf("Unable to load model\n");
    glfwTerminate();
    return -1;
  }

  mat4 projection = {
    { 1.0, 0.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0, 0.0 },
    { 0.0, 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 0.0, 1.0 }
  };
  glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, 100.0f, projection);
  glUseProgram(shader);
  glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1,
                     GL_FALSE, (float *)projection);

  glUseProgram(b_shader);
  glUniformMatrix4fv(glGetUniformLocation(b_shader, "projection"), 1,
                     GL_FALSE, (float *)projection);

  glEnable(GL_DEPTH_TEST);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  while (!glfwWindowShouldClose(window)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    keyboard_input(window);

    // Render
    glUseProgram(shader);

    mat4 view = {
      { 1.0, 0.0, 0.0, 0.0 },
      { 0.0, 1.0, 0.0, 0.0 },
      { 0.0, 0.0, 1.0, 0.0 },
      { 0.0, 0.0, 0.0, 1.0 }
    };
    glm_vec3_add(camera_front, camera_pos, center);
    glm_lookat(camera_pos, center, camera_up, view);

    mat4 model = {
      { 1.0, 0.0, 0.0, 0.0 },
      { 0.0, 1.0, 0.0, 0.0 },
      { 0.0, 0.0, 1.0, 0.0 },
      { 0.0, 0.0, 0.0, 1.0 }
    };
    vec3 translation = { 0.25, 0.25, 0.25 };
    glm_scale(model, translation);
    //glm_translate(model, translation);

    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1,
                       GL_FALSE, (float *) model);

    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1,
                       GL_FALSE, (float *) view);

    draw_model(shader, test);
    //draw_model(shader, cube);
    //draw_model(shader, cross);
    //draw_model(shader, dude);

    glUseProgram(b_shader);
    glUniformMatrix4fv(glGetUniformLocation(b_shader, "model"), 1,
                       GL_FALSE, (float *) model);
    glUniformMatrix4fv(glGetUniformLocation(b_shader, "view"), 1,
                       GL_FALSE, (float *) view);
    glPointSize(2.0);
    draw_bones(test);

    // Swap Buffers and Poll Events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  free_model(cube);
  free_model(test);
  free_model(cross);
  free_model(dude);

  glfwTerminate();
  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void keyboard_input(GLFWwindow *window) {
  vec3 movement = { 0.0, 0.0, 0.0 };
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    glm_vec3_scale(camera_front, 0.005, movement);
    glm_vec3_add(camera_pos, movement, camera_pos);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    glm_vec3_scale(camera_front, 0.005, movement);
    glm_vec3_sub(camera_pos, movement, camera_pos);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    glm_vec3_cross(camera_front, camera_up, movement);
    glm_vec3_normalize(movement);
    glm_vec3_scale(movement, 0.005, movement);
    glm_vec3_sub(camera_pos, movement, camera_pos);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    glm_vec3_cross(camera_front, camera_up, movement);
    glm_vec3_normalize(movement);
    glm_vec3_scale(movement, 0.005, movement);
    glm_vec3_add(camera_pos, movement, camera_pos);
  }
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
}

void mouse_input(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = yaw;
    firstMouse = 0;
  }

  float xOffset = xpos -lastX;
  float yOffset = lastY - ypos;

  lastX = xpos;
  lastY = ypos;

  const float sensitivity = 0.1f;
  xOffset *= sensitivity;
  yOffset *= sensitivity;

  yaw += xOffset;
  pitch += yOffset;

  if (pitch > 89.0f) {
    pitch = 89.0f;
  } else if (pitch < -89.0f) {
    pitch = -89.0f;
  }

  camera_front[0] = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
  camera_front[1] = sin(glm_rad(pitch));
  camera_front[2] = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
  glm_vec3_normalize(camera_front);
}
