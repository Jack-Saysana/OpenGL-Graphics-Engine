#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define VERTEX_BUFF_STARTING_LEN (10)
#define NORMAL_BUFF_STARTING_LEN (10)
#define TEX_COORD_BUFF_STARTING_LEN (10)
#define VBO_STARTING_LEN (10)
#define INDEX_BUFF_STARTING_LEN (20)
#define NUM_PROPS (5)

typedef enum {
  AMB = 0,
  DIFF = 1,
  SPEC = 2,
  SPEC_EXPONENT = 3,
  BUMP = 4
} TEX_TYPE;

typedef struct material {
  uint64_t name;
  char *mat_paths[NUM_PROPS];
} MATERIAL;

typedef struct line_buffer {
  char *dir;
  char *filename;
  char **buffer;
  size_t len;
} LINE_BUFFER;

typedef struct bone {
  float coords[3];
  int num_children;
} BONE;

typedef struct vbo {
  float vertex[3];
  float normal[3];
  float tex_coord[2];
  int bone_ids[4];
  float weights[4];
} VBO;

typedef struct model {
  unsigned int textures[NUM_PROPS];
  BONE *bones;
  unsigned int VAO;
  size_t num_bones;
  unsigned int num_indicies;
} MODEL;

MODEL *load_model(char *);
unsigned int genTextureId(char *tex_path);
LINE_BUFFER *get_lines(char *);
int preprocess_lines(LINE_BUFFER *);
void free_line_buffer(LINE_BUFFER *);
void free_materials(void *buffer, size_t buf_len);
int double_buffer(void **, size_t *, size_t);
