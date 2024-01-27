#include <stdio.h>
#include <stdlib.h>
#include <entity_str.h>
#include <font_str.h>

#if WINDOWS
#define DIR "C:/Users/Jack/Documents/C/OpenGL-Graphics-Engine"
#elif
#define DIR "/home/jbs/Documents/C/OpenGL-Graphics-Engine"
#else
#define DIR ""
#endif

int init_scene();

unsigned int init_shader_prog(char *, char *, char *);
MODEL *load_model(char *path);
ENTITY *init_entity(MODEL *model);
int import_font(char *bin_path, char *tex_path, F_GLYPH **);
