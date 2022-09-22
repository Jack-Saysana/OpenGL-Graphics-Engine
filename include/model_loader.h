#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>

#define LINE_BUFF_STARTING_LEN (50)

#define VERTEX_BUFF_STARTING_LEN (10)
#define NORMAL_BUFF_STARTING_LEN (10)
#define TEX_COORD_BUFF_STARTING_LEN (10)
#define VBO_STARTING_LEN (10)
#define INDEX_BUFF_STARTING_LEN (20)

typedef struct line_buffer {
  char *path;
  char **buffer;
  size_t len;
} LINE_BUFFER;

typedef struct vbo {
  float vertex[3];
  float normal[3];
  float tex_coord[2];
} VBO;

typedef struct model {
  unsigned int VAO;
  unsigned int num_verts;
} MODEL;

extern float (*verticies)[3];
extern size_t v_buff_len;
extern size_t v_len;

extern float (*normals)[3];
extern size_t n_buff_len;
extern size_t n_len;

extern float (*tex_coords)[2];
extern size_t t_buff_len;
extern size_t t_len;

extern int (*vbo_index_combos)[3];
extern size_t vbo_buff_len;
extern size_t vbo_len;

extern int (*indicies)[3];
extern size_t i_buff_len;
extern size_t i_len;

MODEL *load_model(char *);
int parse_lines(LINE_BUFFER *);
LINE_BUFFER *get_lines(char *);
int preprocess_lines(LINE_BUFFER *);
void free_line_buffer(LINE_BUFFER *);
int double_buffer(void **, size_t *, size_t);
int parse_face(char *);
int add_vertex_attrib(int *index_combo);
