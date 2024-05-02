#include <stdio.h>
#include <stdlib.h>
#include <entity_str.h>
#include <font_str.h>

#define DIR "./"

int init_scene();

unsigned int init_shader_prog(char *, char *, char *);
MODEL *load_model(char *path);
ENTITY *init_entity(MODEL *model);
int import_font(char *bin_path, char *tex_path, F_GLYPH **);
