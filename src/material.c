#include <material.h>

int parse_mtllib(MATERIAL *materials, size_t *mat_buff_len, size_t mat_len,
                 char *lib) {
  LINE_BUFFER *lib_lines = get_lines(lib);
  if (lib == NULL) {
    printf("Unable to get mtllib line buffer\n");
    return -1;
  }

  MATERIAL cur_mat = NULL;
  char *cur_line = NULL;
  int status = 0;
  for (int i = 0; i < lib_lines->len; i++) {
    cur_line = lib_lines->buffer[i];
  }

  free_line_buffer(lb);
}
