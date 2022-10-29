#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  AMB,
  DIFF,
  SPEC,
  SPEC_EXPONENT,
  MAP_AMB,
  MAP_DIFF,
  MAP_SPEC,
  MAP_SPEC_EXPONENT,
  BUMP,
  NO_OP,
  NEWMTL
} PROP_TYPE;

typedef struct line_buffer {
  char *path;
  char **buffer;
  size_t len;
} LINE_BUFFER;

typedef union prop_data {
  char *path;
  float *col;
} PROP_DATA;

typedef struct prop {
  PROP_DATA data;
  PROP_TYPE type;
} MAT_PROP;

typedef struct material {
  char *name;
  MAT_PROP *properties;
  int num_props;
} MATERIAL;

int parse_mtllib(MATERIAL *materials, size_t *mat_buff_len, size_t mat_len,
                 char *lib);
PROP_TYPE get_op(char **cur_line);
void free_materials(void *buffer, size_t buf_size);

int double_complex_buffer(void **buffer, size_t *buff_size, size_t buff_len,
                          size_t unit_size, void free_unit(void *, size_t));
LINE_BUFFER *get_lines(char *);
void free_line_buffer(LINE_BUFFER *);
