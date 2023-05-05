#include <stdio.h>
#include <stdlib.h>
#include <cglm/vec3.h>
#include <entity_str.h>

#define LINUX (0)
#define LAPTOP (0)
#define PC (1)

#if LINUX == 1
#define DIR "/home/jbs/Documents/C/OpenGL-Graphics-Engine"
#elif LAPTOP == 1
#define DIR "C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine"
#elif PC == 1
#define DIR "C:/Users/Jack/Documents/C/OpenGL-Graphics-Engine"
#else
#define DIR ""
#endif

int init_scene();

unsigned int init_shader_prog(char *, char *, char *);
MODEL *load_model(char *path);
ENTITY *init_entity(MODEL *model);

