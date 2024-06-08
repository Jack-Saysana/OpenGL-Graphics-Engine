#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>
#include <cglm/quat.h>
#include <cglm/affine.h>
#include <structs/entity_str.h>

ENTITY *init_entity(MODEL *model);
void draw_entity(unsigned int shader, ENTITY *entity);
void draw_skeleton(unsigned int shader, ENTITY *entity);
void draw_colliders(unsigned int shader, ENTITY *entity, MODEL *sphere);
void draw_model(unsigned int shader, MODEL *model);
void draw_bones(MODEL *model);
void free_entity(ENTITY *entity);
void get_model_mat(ENTITY *entity, mat4 model);
