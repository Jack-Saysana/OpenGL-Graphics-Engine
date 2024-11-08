#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

long gen_shader(const char *source, GLenum type);
char *load_source(char *path);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================
