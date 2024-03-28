#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <model_data_str.h>

// ============================= LOCAL CONSTANTS =============================

#define TEX_TAB_STARTING_LEN (20)

// ============================== LOCAL STRUCTS ==============================

typedef struct texture_table {
  char *path;
  unsigned int texture;
  int status;
} TEX_TAB;
static TEX_TAB *tex_tab = NULL;
static size_t tex_tab_len = 0;
static size_t tex_tab_size = 0;

// =============================== LOCAL ENUMS ===============================

typedef enum {
  AMB = 0,
  DIFF = 1,
  SPEC = 2,
  SPEC_EXPONENT = 3,
  BUMP = 4
} TEX_TYPE;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

size_t tex_tab_add(char *);
size_t tex_tab_search(char *);
int resize_tex_tab();

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

size_t get_str_hash(char *);
char *remove_double_dot(char *);
