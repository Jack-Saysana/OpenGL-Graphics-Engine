#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>
#include <structs/models/model_data_str.h>

// ============================= LOCAL CONSTANTS =============================

#define VERTEX_BUFF_STARTING_LEN (10)
#define NORMAL_BUFF_STARTING_LEN (10)
#define TEX_COORD_BUFF_STARTING_LEN (10)
#define VBO_STARTING_LEN (10)
#define INDEX_BUFF_STARTING_LEN (20)

// ============================== LOCAL STRUCTS ==============================

typedef struct line_buffer {
  char *dir;
  char *filename;
  char **buffer;
  size_t len;
} LINE_BUFFER;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int gen_texture_id(char *tex_path, unsigned int *);
LINE_BUFFER *get_lines(char *);
int preprocess_lines(LINE_BUFFER *);

