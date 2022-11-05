#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef enum {
  NO_OP = -2,
  NEWMTL = -1,
  AMB = 0,
  DIFF = 1,
  SPEC = 2,
  SPEC_EXPONENT = 3,
  BUMP = 4
} PROP_TYPE;

#define NUM_PROPS (5)

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

typedef struct material {
  uint64_t name;
  char *mat_paths[NUM_PROPS];
} MATERIAL;

int parse_mtllib(MATERIAL *materials, size_t *mat_buff_len, size_t *mat_len,
                 char *dir, char *lib);
PROP_TYPE get_op(char **cur_line);
void free_materials(void *buffer, size_t buf_len);

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
int double_complex_buffer(void **buffer, size_t *buff_size, size_t buff_len,
                          size_t unit_size, void free_buff(void *, size_t));
LINE_BUFFER *get_lines(char *);
void free_line_buffer(LINE_BUFFER *);
uint64_t get_hash(char *str);
