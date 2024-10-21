#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include <const.h>
#include <structs/line_buffer_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================
