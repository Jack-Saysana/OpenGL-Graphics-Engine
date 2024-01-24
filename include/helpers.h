#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <glad/glad.h>
#include <cglm/cglm.h>

#define LINE_BUFF_STARTING_LEN (50)
#define FILE_CONTENTS_STARTING_LEN (500)

typedef struct line_buffer {
  char *dir;
  char *filename;
  char **buffer;
  size_t len;
} LINE_BUFFER;

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
//int double_complex_buffer(void **buffer, size_t *buff_size, size_t buff_len,
//                          size_t unit_size, void free_buff(void *, size_t));
uint64_t get_hash(char *str);
LINE_BUFFER *get_lines(char *);
void free_line_buffer(LINE_BUFFER *);
void vec3_remove_noise(vec3 vec, float threshold);
