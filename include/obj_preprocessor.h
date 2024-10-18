#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <structs/models/entity_str.h>

typedef enum {
  NO_OP = -2,
  NEWMTL = -1,
  AMB = 0,
  DIFF = 1,
  SPEC = 2,
  SPEC_EXPONENT = 3,
  BUMP = 4
} PROP_TYPE;

#define BUFF_STARTING_LEN (10)
#define NUM_PROPS (5)

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

typedef struct face_vert {
  struct face_vert *prev;
  struct face_vert *next;
  int index;
} FACE_VERT;

BONE *bones;
size_t b_buff_len;
size_t b_len;

ANIMATION *animations;
size_t a_buff_len;
size_t a_len;

int (*bone_ids)[4];
float (*bone_weights)[4];
int *collider_links;

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

COLLIDER *colliders;
int *bone_links;
size_t col_buff_len;
size_t col_len;

int preprocess_lines(LINE_BUFFER *);
int preprocess_face(FILE *, char *);
int triangulate_polygon(FILE *, FACE_VERT *, size_t);
int is_ear(int *, FACE_VERT *, float *);
int sort_colliders();
void swap_colliders(size_t cur, size_t dest);
size_t get_str_hash(char *str);
int parse_mtllib(MATERIAL *materials, size_t *mat_buff_len, size_t *mat_len,
                 char *dir, char *lib);
void free_line_buffer(LINE_BUFFER *);
void free_materials(void *buffer, size_t buf_len);
int double_buffer(void **, size_t *, size_t);
int max_dot(vec3 *verts, unsigned int len, vec3 dir);
