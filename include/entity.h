#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>
#include <entity_str.h>

ENTITY *init_entity(MODEL *model);
void draw_entity(unsigned int shader, ENTITY *entity);
void draw_skeleton(unsigned int shader, ENTITY *entity);
void draw_model(unsigned int shader, MODEL *model);
void draw_bones(MODEL *model);
void free_entity(ENTITY *entity);
