#include <stdio.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <entity_str.h>

#define COLLISION (1)
#define POSSIBLE (0)
#define REMOVED (-1)

typedef enum {
  LINE = 2,
  TRIANGLE = 3,
  TETRAHEDRON = 4
} SIMPLEX;

int collision_check(COLLIDER *a, COLLIDER *b);
int tetrahedron_check(vec3 *simplex, unsigned int *num_used, vec3 dir);
int triangle_check(vec3 A, vec3 B, vec3 C, unsigned int *num_used, vec3 dir);
void support_func(COLLIDER *a, COLLIDER *b, vec3 dir, vec3 dest);
int max_dot(COLLIDER *a, vec3 dir);
void calc_dir_line(vec3 a, vec3 b, vec3 dir);
