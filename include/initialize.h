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

static void fbs_callback(GLFWwindow *, int, int);
static void mm_callback(GLFWwindow *, double, double);
static void s_callback(GLFWwindow *, double, double);
static void mb_callback(GLFWwindow *, int, int, int);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int double_buffer(void **, size_t *, size_t);
