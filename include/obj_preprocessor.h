#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <const.h>
#include <cglm/cglm.h>
#include <structs/material_str.h>
#include <structs/line_buffer_str.h>
#include <structs/models/entity_str.h>

typedef struct face_vert {
  struct face_vert *prev;
  struct face_vert *next;
  int index;
} FACE_VERT;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

int preprocess_face(vec3 *vertices, vec3 *normals, ivec3 **vbo_index_combos,
                    size_t *vbo_len, size_t *vbo_buff_len, ivec3 **faces,
                    size_t *f_len, size_t *face_buff_len, size_t v_len,
                    size_t t_len, size_t n_len, FILE *file, char *line);
int triangulate_polygon(vec3 *vertices, vec3 *normals, ivec3 *vbo_index_combos,
                        ivec3 **faces, size_t *f_len, size_t *face_buff_len,
                        FILE *file, FACE_VERT *head, size_t num_verts);
int is_ear(vec3 *verticies, ivec3 *vbo_index_combos, ivec3 triangle,
           FACE_VERT *ref_vert, float *polygon_normal);
int sort_colliders(BONE *bones, COLLIDER *colliders, int *collider_links,
                   int *bone_links, size_t b_len, size_t col_len);
void swap_colliders(COLLIDER *colliders, int *collider_links, int *bone_links,
                    size_t b_len, size_t cur, size_t dest);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

size_t get_str_hash(char *str);
int parse_mtllib(MATERIAL *materials, size_t *mat_buff_len, size_t *mat_len,
                 char *dir, char *lib);
void free_line_buffer(LINE_BUFFER *lb);
void free_materials(void *buffer, size_t buf_len);
int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
int max_dot(vec3 *verts, unsigned int len, vec3 dir);
