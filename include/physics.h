#include <stdio.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <entity_str.h>

typedef enum {
  LINE = 2,
  TRIANGLE = 3,
  TETRAHEDRON = 4
} SIMPLEX;

int collision_check(COLLIDER *a, COLLIDER *b);
void triangle_check(vec3 a, vec3 b, vec3 c, unsigned int *num_used);
void support_func(COLLIDER *a, COLLIDER *b, vec3 dir, vec3 dest);
int max_dot(COLLIDER *a, vec3 dir);
