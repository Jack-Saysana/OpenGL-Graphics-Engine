#include <initialize.h>

GLFWwindow *init_gl(char *win_name) {
  fb_size_callbacks = malloc(sizeof(void (**)(GLFWwindow *, int, int)) *
                             BUFF_START_LEN);
  if (fb_size_callbacks == NULL) {
    fprintf(stderr, "Error: Failed to allocate callback buffer\n");
    return NULL;
  }
  fbs_len = 0;
  fbs_size = BUFF_START_LEN;

  mouse_mov_callbacks = malloc(sizeof(void (**)(GLFWwindow *, double, double)) *
                               BUFF_START_LEN);
  if (mouse_mov_callbacks == NULL) {
    fprintf(stderr, "Error: Failed to allocate callback buffer\n");
    free(fb_size_callbacks);
    return NULL;
  }
  mm_len = 0;
  mm_size = BUFF_START_LEN;

  scroll_callbacks = malloc(sizeof(void (**)(GLFWwindow *, double, double)) *
                            BUFF_START_LEN);
  if (scroll_callbacks == NULL) {
    fprintf(stderr, "Error: Failed to allocate callback buffer\n");
    free(fb_size_callbacks);
    free(mouse_mov_callbacks);
    return NULL;
  }
  s_len = 0;
  s_size = BUFF_START_LEN;

  mouse_button_callbacks = malloc(sizeof(void(**)
                                         (GLFWwindow *, int, int, int)) *
                                  BUFF_START_LEN);
  if (mouse_button_callbacks == NULL) {
    fprintf(stderr, "Error: Failed to allocate callback buffer\n");
    free(fb_size_callbacks);
    free(mouse_mov_callbacks);
    free(scroll_callbacks);
    return NULL;
  }
  mb_len = 0;
  mb_size = BUFF_START_LEN;

  GLFWwindow *window = NULL;

  if (!glfwInit()) {
    free(fb_size_callbacks);
    free(mouse_mov_callbacks);
    free(scroll_callbacks);
    free(mouse_button_callbacks);
    return NULL;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(RES_X, RES_Y, win_name, NULL, NULL);
  if (window == NULL) {
    fprintf(stderr, "Error: Failed to create GLFW window\n");
    free(fb_size_callbacks);
    free(mouse_mov_callbacks);
    free(scroll_callbacks);
    free(mouse_button_callbacks);
    glfwTerminate();
    return NULL;
  }

  glfwSetFramebufferSizeCallback(window, fbs_callback);
  glfwSetCursorPosCallback(window, mm_callback);
  glfwSetScrollCallback(window, s_callback);
  glfwSetMouseButtonCallback(window, mb_callback);

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    fprintf(stderr, "Error: Failed to initialize GLAD\n");
    free(fb_size_callbacks);
    free(mouse_mov_callbacks);
    free(scroll_callbacks);
    free(mouse_button_callbacks);
    glfwTerminate();
    return NULL;
  }

  glViewport(0, 0, RES_X, RES_Y);

  return window;
}

void cleanup_gl() {
  free(fb_size_callbacks);
  free(mouse_mov_callbacks);
  free(scroll_callbacks);
  free(mouse_button_callbacks);
  glfwTerminate();
}

// ========================== CALLBACK REGISTRATION ==========================

int register_fb_size_callback(void (*cb)(GLFWwindow *, int, int)) {
  fb_size_callbacks[fbs_len] = cb;
  fbs_len++;
  if (fbs_len == fbs_size) {
    int status = double_buffer((void **) &fb_size_callbacks, &fbs_size,
                               sizeof(void (*)(GLFWwindow *, int, int)));
    if (status) {
      fbs_len--;
      fprintf(stderr, "Error: Unable to reallocate callback buffer\n");
      return -1;
    }
  }
  return 0;
}

int register_mouse_movement_callback(void (*cb)(GLFWwindow *, double,
                                                double)) {
  mouse_mov_callbacks[mm_len] = cb;
  mm_len++;
  if (mm_len == mm_size) {
    int status = double_buffer((void **) &mouse_mov_callbacks, &mm_size,
                               sizeof(void (*)(GLFWwindow *, double, double)));
    if (status) {
      mm_len--;
      fprintf(stderr, "Error: Unable to reallocate callback buffer\n");
      return -1;
    }
  }
  return 0;
}

int register_scroll_callback(void (*cb)(GLFWwindow *, double, double)) {
  scroll_callbacks[s_len] = cb;
  s_len++;
  if (s_len == s_size) {
    int status = double_buffer((void **) &scroll_callbacks, &s_size,
                               sizeof(void (*)(GLFWwindow *, double, double)));
    if (status) {
      s_len--;
      fprintf(stderr, "Error: Unable to reallocate callback buffer\n");
      return -1;
    }
  }
  return 0;
}

int register_mouse_button_callback(void (*cb)(GLFWwindow *, int, int, int)) {
  mouse_button_callbacks[mb_len] = cb;
  mb_len++;
  if (mb_len == mb_size) {
    int status = double_buffer((void **) &mouse_button_callbacks, &mb_size,
                               sizeof(void (*)(GLFWwindow *, int, int, int)));
    if (status) {
      mb_len--;
      fprintf(stderr, "Error: Unable to reallocate callback buffer\n");
      return -1;
    }
  }
  return 0;
}

// ============================ CALLBACK HANDLING ============================

void fbs_callback(GLFWwindow *window, int width, int height) {
  for (size_t i = 0; i < fbs_len; i++) {
    fb_size_callbacks[i](window, width, height);
  }
}

void mm_callback(GLFWwindow *window, double x_pos, double y_pos) {
  for (size_t i = 0; i < mm_len; i++) {
    mouse_mov_callbacks[i](window, x_pos, y_pos);
  }
}

void s_callback(GLFWwindow *window, double x_offset, double y_offset) {
  for (size_t i = 0; i < s_len; i++) {
    scroll_callbacks[i](window, x_offset, y_offset);
  }
}

void mb_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  for (size_t i = 0; i < mb_len; i++) {
    mouse_button_callbacks[i](window, button, action, mods);
  }
}
