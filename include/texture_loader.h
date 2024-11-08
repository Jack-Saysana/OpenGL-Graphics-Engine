#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <structs/models/model_data_str.h>

// ============================== LOCAL STRUCTS ==============================

typedef struct texture_table {
  char *path;
  unsigned int texture;
  int status;
} TEX_TAB;
static TEX_TAB *tex_tab = NULL;
static size_t tex_tab_len = 0;
static size_t tex_tab_size = 0;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

size_t tex_tab_add(char *path);
size_t tex_tab_search(char *path);
int resize_tex_tab();

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

size_t get_str_hash(char *str);
char *remove_double_dot(char *path);
