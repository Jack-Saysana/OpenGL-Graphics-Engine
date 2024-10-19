#ifndef __LINE_BUFFER_STR_H__
#define __LINE_BUFFER_STR_H__

typedef struct line_buffer {
  char *dir;
  char *filename;
  char **buffer;
  size_t len;
} LINE_BUFFER;

#endif
