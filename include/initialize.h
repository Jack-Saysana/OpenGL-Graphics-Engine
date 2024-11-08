#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <globals.h>
#include <const.h>

static void (**fb_size_callbacks)(GLFWwindow *, int, int);
static size_t fbs_len = 0;
static size_t fbs_size = 0;

static void (**mouse_mov_callbacks)(GLFWwindow *, double, double);
static size_t mm_len = 0;
static size_t mm_size = 0;

static void (**scroll_callbacks)(GLFWwindow *, double, double);
static size_t s_len = 0;
static size_t s_size = 0;

static void (**mouse_button_callbacks)(GLFWwindow *, int, int, int);
static size_t mb_len = 0;
static size_t mb_size = 0;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

static void fbs_callback(GLFWwindow *window, int width, int height);
static void mm_callback(GLFWwindow *window, double x_pos, double y_pos);
static void s_callback(GLFWwindow *window, double x_offset, double y_offset);
static void mb_callback(GLFWwindow *window, int button, int action, int mods);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
