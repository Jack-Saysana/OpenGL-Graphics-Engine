#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERTEX_BUFF_STARTING_LEN (10)
#define NORMAL_BUFF_STARTING_LEN (10)
#define TEX_COORD_BUFF_STARTING_LEN (10)
#define VBO_STARTING_LEN (10)

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

typedef struct line_buffer {
  char *path;
  char **buffer;
  size_t len;
} LINE_BUFFER;

typedef struct face_vert {
  struct face_vert *prev;
  struct face_vert *next;
  int index;
} FACE_VERT;

void free_line_buffer(LINE_BUFFER *);
int double_buffer(void **, size_t *, size_t);
int preprocess_lines(LINE_BUFFER *);
int preprocess_face(FILE *, char *);
int write_triangle(FILE *, int *);
int triangulate_polygon(FILE *, FACE_VERT *, size_t);
int is_ear(int *, FACE_VERT *, float *);
