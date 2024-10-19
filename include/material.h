#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <const.h>
#include <structs/material_str.h>
#include <structs/line_buffer_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

PROP_TYPE get_op(char **cur_line);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
LINE_BUFFER *get_lines(char *);
void free_line_buffer(LINE_BUFFER *);
size_t get_str_hash(char *str);
