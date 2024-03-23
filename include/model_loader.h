#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <model_data_str.h>

// ============================= LOCAL CONSTANTS =============================

#define VERTEX_BUFF_STARTING_LEN (10)
#define NORMAL_BUFF_STARTING_LEN (10)
#define TEX_COORD_BUFF_STARTING_LEN (10)
#define VBO_STARTING_LEN (10)
#define INDEX_BUFF_STARTING_LEN (20)
#define TEX_TAB_STARTING_LEN (20)
#define INVALID_TEX (0xFFFFFFFF)

// =============================== LOCAL ENUMS ===============================

typedef enum {
  AMB = 0,
  DIFF = 1,
  SPEC = 2,
  SPEC_EXPONENT = 3,
  BUMP = 4
} TEX_TYPE;

// ============================== LOCAL STRUCTS ==============================

typedef struct line_buffer {
  char *dir;
  char *filename;
  char **buffer;
  size_t len;
} LINE_BUFFER;

typedef struct texture_table {
  char *path;
  unsigned int texture;
  int status;
} TEX_TAB;
static TEX_TAB *tex_tab = NULL;
static size_t tex_tab_len = 0;
static size_t tex_tab_size = 0;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

size_t tex_tab_add(char *);
size_t tex_tab_search(char *);
int resize_tex_tab();

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int gen_texture_id(char *tex_path, unsigned int *);
LINE_BUFFER *get_lines(char *);
int preprocess_lines(LINE_BUFFER *);
void free_materials(void *buffer, size_t buf_len);
size_t get_str_hash(char *);
char *remove_double_dot(char *);
