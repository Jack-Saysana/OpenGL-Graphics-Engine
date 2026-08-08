#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cglm/cglm.h>
#include <glad/glad.h>
#include <structs/line_buffer_str.h>
#include <structs/models/model_data_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
int gen_texture_id(char *tex_path, unsigned int *dest);
LINE_BUFFER *get_lines(char *path);
int preprocess_lines(LINE_BUFFER *lb);

