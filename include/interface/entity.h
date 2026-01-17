#ifndef __ENGINE_ENTITY_H__
#define __ENGINE_ENTITY_H__

#include "./structs/models/model_data_str.h"
#include "./structs/2d/models/entity_2d_str.h"
#include "./structs/models/render_primitives_str.h"

unsigned int init_shader_prog(char *vs_path, char *gs_path, char *fs_path);
int gen_texture_id(char *tex_path, unsigned int *dest);
int gen_cubemap(char **paths, unsigned int *dest);
void free_textures();

// 3D Interface
MODEL_DATA *load_model_data(char *path);
MODEL *gen_model(MODEL_DATA *md, int gen_vao);
MODEL *load_model(char *path);
MODEL *load_model_vaoless(char *path);
void init_model_vao(MODEL *model);
ENTITY *init_entity(MODEL *model);
void draw_entity(unsigned int shader, ENTITY *entity);
void draw_skeleton(unsigned int shader, ENTITY *entity);
void draw_bone(unsigned int shader, ENTITY *entity, size_t bone);
void draw_bone_axis(unsigned int shader, ENTITY *entity, size_t bone);
void draw_colliders(unsigned int shader, ENTITY *entity, MODEL *sphere);
void draw_collider(unsigned int shader, ENTITY *entity, size_t col,
                   MODEL *sphere);
void draw_model(unsigned int shader, MODEL *model);
void free_model(MODEL *model);
void free_entity(ENTITY *entity);
void write_model_obj(MODEL_DATA *md, char *path);
int write_model_bin(MODEL_DATA *md, char *path);
void set_inv_mass(ENTITY *entity, size_t col, float inv_mass);
void rotate_inv_inertia(ENTITY *ent, size_t col, mat4 dest);

// 2D Interface
ENTITY_2D *init_entity_2d(int type, float height, float width);
void draw_entity_2d(unsigned int shader, ENTITY_2D *ent);
void draw_2d_colliders(unsigned int shader, ENTITY_2D *ent);
void draw_2d_collider(unsigned int shader, COLLIDER_2D *col, vec3 ent_pos);
void free_entity_2d(ENTITY_2D *);

// Render primatives
void draw_poly(vec3 *verts);
void draw_lines(L_VBO *lines, size_t num_lines);
void draw_square(vec2 center, float half_w, float half_h);
void draw_circle(vec2 center, float radius);
void draw_quad();

#endif
