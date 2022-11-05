#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define VERTEX_BUFF_STARTING_LEN (10)
#define NORMAL_BUFF_STARTING_LEN (10)
#define TEX_COORD_BUFF_STARTING_LEN (10)
#define VBO_STARTING_LEN (10)
#define FACE_BUFF_STARTING_LEN (10)
#define MATERIAL_BUFF_STARTING_LEN (10)
#define NUM_PROPS (5)

typedef struct material {
  uint64_t name;
  char *mat_paths[NUM_PROPS];
} MATERIAL;

/*typedef struct line_buffer {
  char *path;
  char **buffer;
  size_t len;
} LINE_BUFFER;*/
typedef struct line_buffer {
  char *dir;
  char *filename;
  char **buffer;
  size_t len;
} LINE_BUFFER;

typedef struct face_vert {
  struct face_vert *prev;
  struct face_vert *next;
  int index;
} FACE_VERT;

float (*verticies)[3];
size_t v_buff_len;
size_t v_len;

float (*normals)[3];
size_t n_buff_len;
size_t n_len;

float (*tex_coords)[2];
size_t t_buff_len;
size_t t_len;

int (*vbo_index_combos)[3];
size_t vbo_buff_len;
size_t vbo_len;

int (*faces)[3];
size_t face_buff_len;
size_t f_len;

MATERIAL *materials;
size_t mat_buff_len;
size_t mat_len;

int preprocess_lines(LINE_BUFFER *);
int preprocess_face(FILE *, char *);
//int write_triangle(FILE *, int *);
int triangulate_polygon(FILE *, FACE_VERT *, size_t);
int is_ear(int *, FACE_VERT *, float *);
uint64_t get_hash(char *str);
int parse_mtllib(MATERIAL *materials, size_t *mat_buff_len, size_t *mat_len,
                 char *dir, char *lib);
void free_line_buffer(LINE_BUFFER *);
void free_materials(void *buffer, size_t buf_len);
int double_buffer(void **, size_t *, size_t);
