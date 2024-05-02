#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <cglm/ivec3.h>
#include <cglm/quat.h>
#include <const.h>
#include <entity_str.h>
#include <simulation_str.h>
#include <globals.h>

//#define FRICTION
#define BUFF_STARTING_LEN (10)

#define COLLISION (1)
#define POSSIBLE (0)
#define REMOVED (-1)

#define POINT_COL (1)
#define EDGE_COL (2)
#define FACE_COL (3)

#define DAMP_FACTOR (0.999)

#define MAX_GJK_ITERATIONS (1000)
#define MAX_EPA_ITERATIONS (1000)

static vec3 U_DIR = { 0.0, 1.0, 0.0 };
static vec3 D_DIR = { 0.0, -1.0, 0.0 };
static vec3 L_DIR = { 1.0, 0.0, 0.0 };
static vec3 R_DIR = { -1.0, 1.0, 0.0 };
static vec3 F_DIR = { 0.0, 1.0, 1.0 };
static vec3 B_DIR = { 0.0, 1.0, -1.0 };

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

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

int tetrahedron_check(vec3 *simplex, unsigned int *num_used, vec3 dir);
int triangle_check(vec3 A, vec3 B, vec3 C, unsigned int *num_used, vec3 dir);
void support_func(COLLIDER *a, COLLIDER *b, vec3 dir, vec3 dest);
void calc_dir_line(vec3 a, vec3 b, vec3 dir);
float calc_face_dist(vec3 a, vec3 b, vec3 c, vec3 dest_norm);
int add_unique_edges(int (**u_edges)[2], size_t *num_edges, size_t *e_buff_size,
                     int a, int b);
F_HEAP *init_face_heap();
int insert_face(F_HEAP *heap, int a, int b, int c, vec3 norm, float dist);
void remove_face(F_HEAP *heap, size_t index, ivec3 d_ind, vec3 d_norm);
void free_faces(F_HEAP *heap);
void intersection_point(vec3 a, vec3 b, vec3 c, vec3 d, vec3 norm, vec3 dest);
float get_width(COLLIDER *, vec3);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);
void vec3_remove_noise(vec3 vec, float threshold);
float remove_noise(float, float);
int max_dot(vec3 *verts, unsigned int len, vec3 dir);
