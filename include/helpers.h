#include <stdio.h>
#include <stdlib.h>

#define LINE_BUFF_STARTING_LEN (50)
#define FILE_CONTENTS_STARTING_LEN (500)

typedef struct line_buffer {
  char *path;
  char **buffer;
  size_t len;
} LINE_BUFFER;

int double_buffer(void **, size_t *, size_t);
int double_complex_buffer(void **buffer, size_t *buff_size, size_t buff_len,
                          size_t unit_size, void free_unit(void *, size_t));
LINE_BUFFER *get_lines(char *);
void free_line_buffer(LINE_BUFFER *);
