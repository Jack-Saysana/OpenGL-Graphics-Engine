#include <stdio.h>
#include <stdlib.h>

#define AMBIENT (0)
#define DIFFUSE (1)
#define SPECULAR(2)
//...

typedef struct line_buffer {
  char *path;
  char **buffer;
  size_t len;
} LINE_BUFFER;

typedef struct map {
  char *path;
  int type;
} TEX_MAP;

typedef struct material {
  char *name;
  float (*basic_colors)[3];
  TEX_MAP *maps;
  int num_colors;
  int num_maps;
} MATERIAL;

int parse_mtllib(MATERIAL *materials, size_t *mat_buff_len, size_t mat_len,
                 char *lib);
LINE_BUFFER *get_lines(char *);
void free_line_buffer(LINE_BUFFER *);
