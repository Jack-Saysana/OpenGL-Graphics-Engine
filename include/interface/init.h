#ifndef __ENGINE_INIT_H__
#define __ENGINE_INIT_H__

#include <GLFW/glfw3.h>

GLFWwindow *init_gl(char *name);
void cleanup_gl();
int register_fb_size_callback(void (*cb)(GLFWwindow *, int, int));
int register_mouse_movement_callback(void (*cb)(GLFWwindow *, double, double));
int register_scroll_callback(void (*cb)(GLFWwindow *, double, double));
int register_mouse_button_callback(void (*cb)(GLFWwindow *, int, int, int));

#endif
