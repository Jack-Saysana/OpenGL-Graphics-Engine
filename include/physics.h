#include <stdio.h>
#include <math.h>
#include <cglm/vec3.h>
#include <cglm/ivec3.h>
#include <cglm/mat4.h>
#include <entity_str.h>

#define BUFF_STARTING_LEN (10)

#define COLLISION (1)
#define POSSIBLE (0)
#define REMOVED (-1)

typedef enum {
  LINE = 2,
  TRIANGLE = 3,
  TETRAHEDRON = 4
} SIMPLEX;

typedef struct face {
  ivec3 indicies;
  vec3 norm;
  float dist;
} FACE;

typedef struct face_heap {
  FACE *buffer;
  size_t buff_len;
  size_t buff_size;
} F_HEAP;

int collision_check(COLLIDER *a, COLLIDER *b, vec3 *simplex);
int epa_response(COLLIDER *a, COLLIDER *b, vec3 *simplex, vec3 p_dir,
                 float *p_depth);
void collision_point(COLLIDER *a, COLLIDER *b, vec3 p_vec, vec3 dest);
int tetrahedron_check(vec3 *simplex, unsigned int *num_used, vec3 dir);
int triangle_check(vec3 A, vec3 B, vec3 C, unsigned int *num_used, vec3 dir);
void support_func(COLLIDER *a, COLLIDER *b, vec3 dir, vec3 dest);
int max_dot(COLLIDER *a, vec3 dir);
void calc_dir_line(vec3 a, vec3 b, vec3 dir);

float calc_face_dist(vec3 a, vec3 b, vec3 c, vec3 dest_norm);
int add_unique_edges(int (**u_edges)[2], size_t *num_edges, size_t *e_buff_size,
                     int a, int b);
F_HEAP *init_face_heap();
int insert_face(F_HEAP *heap, int a, int b, int c, vec3 norm, float dist);
void remove_face(F_HEAP *heap, size_t index, ivec3 d_ind, vec3 d_norm);
void free_faces(F_HEAP *heap);

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
