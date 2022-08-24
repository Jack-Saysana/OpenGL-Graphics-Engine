#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>

unsigned int init_shader_prog(char *, char *, char *);
long gen_shader(const char *, GLenum);
char *load_source(char *);
