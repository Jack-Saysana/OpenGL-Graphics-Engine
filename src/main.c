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
      "C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/src/shaders/test/shader.vs",
      NULL,
      "C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/src/shaders/test/shader.fs"
      );
  if (shader == -1) {
    printf("Error loading shaders\n");
    glfwTerminate();
    return -1;
  }

  MODEL *cube = load_model(
      "C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/resources/Cube/cube.obj"
      );
  if (cube == NULL) {
    printf("Unable to load model\n");
    glfwTerminate();
    return -1;
  }

  MODEL *floor = load_model(
      "C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/resources/Floor/floor.obj"
      );
  if (floor == NULL) {
    printf("Unable to load model\n");
    glfwTerminate();
    return -1;
  }

  MODEL *cross = load_model(
      "C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/resources/Cross/cross.obj"
      );
  if (cross == NULL) {
    printf("Unable to load model\n");
    glfwTerminate();
    return -1;
  }

  MODEL *dude = load_model(
      "C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/resources/char/low_poly_new.obj"
      );
  if (dude == NULL) {
    printf("Unable to load model\n");
    glfwTerminate();
    return -1;
  }

  MODEL *pack = load_model(
      "C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine/resources/backpack/backpack.obj"
      );
  if (pack == NULL) {
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

  /*mat4 last = {
    { 1.0, 0.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0, 0.0 },
    { 0.0, 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 0.0, 1.0 }
  };*/

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);

    keyboard_input(window);

    // Render
    glUseProgram(shader);
    glm_vec3_add(camera_front, camera_pos, center);
    glm_lookat(camera_pos, center, camera_up, view);

    //vec3 translation = { 2.0, 2.0, 2.0 };
    //glm_translate(model, translation);
    //glm_scale(model, translation);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1,
                       GL_FALSE, (float *) model);

    /*printf("==========\npos: %f %f %f\ncenter: %f %f %f\nup: %f %f %f\n",
          camera_pos[0], camera_pos[1], camera_pos[2], center[0], center[1],
          center[2], camera_up[0], camera_up[1], camera_up[2]);*/

    /*int equal = 1;
    float *v = (float *) view;
    float *l = (float *) last;
    for (int i = 0; i < 16; i++) {
      if (v[i] != l[i]) {
        equal = 0;
        l[i] = v[i];
      }
    }
    if (equal == 0) {
      printf("==========\n" \
             "%f %f %f %f\n" \
             "%f %f %f %f\n" \
             "%f %f %f %f\n" \
             "%f %f %f %f\n",
             view[0][0], view[0][1], view[0][2], view[0][3], view[1][0],
             view[1][1], view[1][2], view[1][3], view[2][0], view[2][1],
             view[2][2], view[2][3], view[3][0], view[3][1], view[3][2],
             view[3][3]);
      fflush(stdout);
    }*/
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1,
                       GL_FALSE, (float *) view);

    //draw_model(cube);
    //draw_model(floor);
    //draw_model(cross);
    //draw_model(dude);
    draw_model(pack);

    // Swap Buffers and Poll Events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  free(cube);
  free(floor);
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

  /*printf("Cam pos: %f, %f, %f\n", camera_pos[0], camera_pos[1],
         camera_pos[2]);*/
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
