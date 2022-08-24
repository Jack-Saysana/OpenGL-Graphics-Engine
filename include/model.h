#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define V_BUFF_STARTING_LEN (30)
#define LINE_BUFF_STARTING_LEN (50)

#define VERTEX_BUFF_STARTING_LEN (10)
#define NORMAL_BUFF_STARTING_LEN (10)
#define TEX_COORD_BUFF_STARTING_LEN (10)
#define VBO_STARTING_LEN (10)
#define INDEX_BUFF_STARTING_LEN (20)

#define NUM_TEX_COORDS (2)
#define NUM_VERT_COORDS (3)


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

typedef struct vertex_data {
  VBO *vertices;
  int *indicies[3];
} V_DATA;

int import_model(char *);
V_DATA *parse_lines(LINE_BUFFER *);
LINE_BUFFER *get_lines(char *);
void free_line_buffer(LINE_BUFFER *);
int double_buffer(void **, size_t *, size_t);
int parse_face(char *);
int add_vertex_attrib(int *index_combo);
