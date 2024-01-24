#ifndef __ENGINE_ENTITY_H__
#define __ENGINE_ENTITY_H__

unsigned int init_shader_prog(char *vs_path, char *gs_path, char *fs_path);
MODEL *load_model(char *path);
ENTITY *init_entity(MODEL *model);
void draw_entity(unsigned int shader, ENTITY *entity);
void draw_skeleton(unsigned int shader, ENTITY *entity);
void draw_colliders(unsigned int shader, ENTITY *entity, MODEL *sphere);
void draw_model(MODEL *mode);
void free_entity(ENTITY *entity);

#endif
