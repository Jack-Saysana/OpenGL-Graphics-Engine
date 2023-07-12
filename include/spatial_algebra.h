#include <cglm/vec3.h>
#include <cglm/mat3.h>

#define VEC6_ZERO_INIT { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
#define MAT6_ZERO_INIT {VEC6_ZERO_INIT, VEC6_ZERO_INIT, VEC6_ZERO_INIT, VEC6_ZERO_INIT, VEC6_ZERO_INIT, VEC6_ZERO_INIT}

typedef float vec6[6];
typedef vec6 mat6[6];

// dest = m
void mat6_copy(mat6 m, mat6 dest);
// dest = [[m1, m2], [m3, m4]]
void mat6_compose(mat3 m1, mat3 m2, mat3 m3, mat3 m4, mat6 dest);
// dest = m1m2
void mat6_mul(mat6 m1, mat6 m2, mat6 dest);
// dest = mv
void mat6_mulv(mat6 m, vec6 v, vec6 dest);
// R = rotate_fg
// T = translate_fg
// dest = [[R, 0],[TR, R]]
void mat6_spatial_transform(mat3 rotate_fg, mat3 translate_fg, mat6 dest);

// v = [0, 0, 0, 0, 0, 0];
void vec6_zero(vec6 v);
// dest = v
void vec6_copy(vec6 v, vec6 dest);
// dest = [a, b]
void vec6_compose(vec3 a, vec3 b, vec6 dest);
// dest = a + b
void vec6_add(vec6 a, vec6 b, vec6 dest);
// dest = a - b
void vec6_sub(vec6 a, vec6 b, vec6 dest);
// a = [a1, a2]
// b = [b1, b2]
// = a2*b1 + a1*b2
float vec6_dot(vec6 a, vec6 b);
