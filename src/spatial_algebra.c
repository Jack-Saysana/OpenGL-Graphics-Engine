#include <spatial_algebra.h>

void mat6_zero(mat6 m) {
  m[0][0] = 0.0;
  m[0][1] = 0.0;
  m[0][2] = 0.0;
  m[0][3] = 0.0;
  m[0][4] = 0.0;
  m[0][5] = 0.0;
  m[1][0] = 0.0;
  m[1][1] = 0.0;
  m[1][2] = 0.0;
  m[1][3] = 0.0;
  m[1][4] = 0.0;
  m[1][5] = 0.0;
  m[2][0] = 0.0;
  m[2][1] = 0.0;
  m[2][2] = 0.0;
  m[2][3] = 0.0;
  m[2][4] = 0.0;
  m[2][5] = 0.0;
  m[3][0] = 0.0;
  m[3][1] = 0.0;
  m[3][2] = 0.0;
  m[3][3] = 0.0;
  m[3][4] = 0.0;
  m[3][5] = 0.0;
  m[4][0] = 0.0;
  m[4][1] = 0.0;
  m[4][2] = 0.0;
  m[4][3] = 0.0;
  m[4][4] = 0.0;
  m[4][5] = 0.0;
  m[5][0] = 0.0;
  m[5][1] = 0.0;
  m[5][2] = 0.0;
  m[5][3] = 0.0;
  m[5][4] = 0.0;
  m[5][5] = 0.0;
}

void mat6_copy(mat6 m, mat6 dest) {
  dest[0][0] = m[0][0];
  dest[0][1] = m[0][1];
  dest[0][2] = m[0][2];
  dest[0][3] = m[0][3];
  dest[0][4] = m[0][4];
  dest[0][5] = m[0][5];
  dest[1][0] = m[1][0];
  dest[1][1] = m[1][1];
  dest[1][2] = m[1][2];
  dest[1][3] = m[1][3];
  dest[1][4] = m[1][4];
  dest[1][5] = m[1][5];
  dest[2][0] = m[2][0];
  dest[2][1] = m[2][1];
  dest[2][2] = m[2][2];
  dest[2][3] = m[2][3];
  dest[2][4] = m[2][4];
  dest[2][5] = m[2][5];
  dest[3][0] = m[3][0];
  dest[3][1] = m[3][1];
  dest[3][2] = m[3][2];
  dest[3][3] = m[3][3];
  dest[3][4] = m[3][4];
  dest[3][5] = m[3][5];
  dest[4][0] = m[4][0];
  dest[4][1] = m[4][1];
  dest[4][2] = m[4][2];
  dest[4][3] = m[4][3];
  dest[4][4] = m[4][4];
  dest[4][5] = m[4][5];
  dest[5][0] = m[5][0];
  dest[5][1] = m[5][1];
  dest[5][2] = m[5][2];
  dest[5][3] = m[5][3];
  dest[5][4] = m[5][4];
  dest[5][5] = m[5][5];
}

void mat6_compose(mat3 m1, mat3 m2, mat3 m3, mat3 m4, mat6 dest) {
  dest[0][0] = m1[0][0];
  dest[0][1] = m1[0][1];
  dest[0][2] = m1[0][2];
  dest[1][0] = m1[1][0];
  dest[1][1] = m1[1][1];
  dest[1][2] = m1[1][2];
  dest[2][0] = m1[2][0];
  dest[2][1] = m1[2][1];
  dest[2][2] = m1[2][2];
  dest[0][3] = m3[0][0];
  dest[0][4] = m3[0][1];
  dest[0][5] = m3[0][2];
  dest[1][3] = m3[1][0];
  dest[1][4] = m3[1][1];
  dest[1][5] = m3[1][2];
  dest[2][3] = m3[2][0];
  dest[2][4] = m3[2][1];
  dest[2][5] = m3[2][2];
  dest[3][0] = m2[0][0];
  dest[3][1] = m2[0][1];
  dest[3][2] = m2[0][2];
  dest[4][0] = m2[1][0];
  dest[4][1] = m2[1][1];
  dest[4][2] = m2[1][2];
  dest[5][0] = m2[2][0];
  dest[5][1] = m2[2][1];
  dest[5][2] = m2[2][2];
  dest[3][3] = m4[0][0];
  dest[3][4] = m4[0][1];
  dest[3][5] = m4[0][2];
  dest[4][3] = m4[1][0];
  dest[4][4] = m4[1][1];
  dest[4][5] = m4[1][2];
  dest[5][3] = m4[2][0];
  dest[5][4] = m4[2][1];
  dest[5][5] = m4[2][2];
}

void mat6_add(mat6 m1, mat6 m2, mat6 dest) {
  dest[0][0] = m1[0][0] + m2[0][0];
  dest[0][1] = m1[0][1] + m2[0][1];
  dest[0][2] = m1[0][2] + m2[0][2];
  dest[0][3] = m1[0][3] + m2[0][3];
  dest[0][4] = m1[0][4] + m2[0][4];
  dest[0][5] = m1[0][5] + m2[0][5];
  dest[1][0] = m1[1][0] + m2[1][0];
  dest[1][1] = m1[1][1] + m2[1][1];
  dest[1][2] = m1[1][2] + m2[1][2];
  dest[1][3] = m1[1][3] + m2[1][3];
  dest[1][4] = m1[1][4] + m2[1][4];
  dest[1][5] = m1[1][5] + m2[1][5];
  dest[2][0] = m1[2][0] + m2[2][0];
  dest[2][1] = m1[2][1] + m2[2][1];
  dest[2][2] = m1[2][2] + m2[2][2];
  dest[2][3] = m1[2][3] + m2[2][3];
  dest[2][4] = m1[2][4] + m2[2][4];
  dest[2][5] = m1[2][5] + m2[2][5];
  dest[3][0] = m1[3][0] + m2[3][0];
  dest[3][1] = m1[3][1] + m2[3][1];
  dest[3][2] = m1[3][2] + m2[3][2];
  dest[3][3] = m1[3][3] + m2[3][3];
  dest[3][4] = m1[3][4] + m2[3][4];
  dest[3][5] = m1[3][5] + m2[3][5];
  dest[4][0] = m1[4][0] + m2[4][0];
  dest[4][1] = m1[4][1] + m2[4][1];
  dest[4][2] = m1[4][2] + m2[4][2];
  dest[4][3] = m1[4][3] + m2[4][3];
  dest[4][4] = m1[4][4] + m2[4][4];
  dest[4][5] = m1[4][5] + m2[4][5];
  dest[5][0] = m1[5][0] + m2[5][0];
  dest[5][1] = m1[5][1] + m2[5][1];
  dest[5][2] = m1[5][2] + m2[5][2];
  dest[5][3] = m1[5][3] + m2[5][3];
  dest[5][4] = m1[5][4] + m2[5][4];
  dest[5][5] = m1[5][5] + m2[5][5];
}

void mat6_sub(mat6 m1, mat6 m2, mat6 dest) {
  dest[0][0] = m1[0][0] - m2[0][0];
  dest[0][1] = m1[0][1] - m2[0][1];
  dest[0][2] = m1[0][2] - m2[0][2];
  dest[0][3] = m1[0][3] - m2[0][3];
  dest[0][4] = m1[0][4] - m2[0][4];
  dest[0][5] = m1[0][5] - m2[0][5];
  dest[1][0] = m1[1][0] - m2[1][0];
  dest[1][1] = m1[1][1] - m2[1][1];
  dest[1][2] = m1[1][2] - m2[1][2];
  dest[1][3] = m1[1][3] - m2[1][3];
  dest[1][4] = m1[1][4] - m2[1][4];
  dest[1][5] = m1[1][5] - m2[1][5];
  dest[2][0] = m1[2][0] - m2[2][0];
  dest[2][1] = m1[2][1] - m2[2][1];
  dest[2][2] = m1[2][2] - m2[2][2];
  dest[2][3] = m1[2][3] - m2[2][3];
  dest[2][4] = m1[2][4] - m2[2][4];
  dest[2][5] = m1[2][5] - m2[2][5];
  dest[3][0] = m1[3][0] - m2[3][0];
  dest[3][1] = m1[3][1] - m2[3][1];
  dest[3][2] = m1[3][2] - m2[3][2];
  dest[3][3] = m1[3][3] - m2[3][3];
  dest[3][4] = m1[3][4] - m2[3][4];
  dest[3][5] = m1[3][5] - m2[3][5];
  dest[4][0] = m1[4][0] - m2[4][0];
  dest[4][1] = m1[4][1] - m2[4][1];
  dest[4][2] = m1[4][2] - m2[4][2];
  dest[4][3] = m1[4][3] - m2[4][3];
  dest[4][4] = m1[4][4] - m2[4][4];
  dest[4][5] = m1[4][5] - m2[4][5];
  dest[5][0] = m1[5][0] - m2[5][0];
  dest[5][1] = m1[5][1] - m2[5][1];
  dest[5][2] = m1[5][2] - m2[5][2];
  dest[5][3] = m1[5][3] - m2[5][3];
  dest[5][4] = m1[5][4] - m2[5][4];
  dest[5][5] = m1[5][5] - m2[5][5];
}

void mat6_mul(mat6 m1, mat6 m2, mat6 dest) {
  mat6 c = MAT6_ZERO_INIT;
  c[0][0] = m1[0][0]*m2[0][0]+m1[1][0]*m2[0][1]+m1[2][0]*m2[0][2]+m1[3][0]*m2[0][3]+m1[4][0]*m2[0][4]+m1[5][0]*m2[0][5];
  c[1][0] = m1[0][0]*m2[1][0]+m1[1][0]*m2[1][1]+m1[2][0]*m2[1][2]+m1[3][0]*m2[1][3]+m1[4][0]*m2[1][4]+m1[5][0]*m2[1][5];
  c[2][0] = m1[0][0]*m2[2][0]+m1[1][0]*m2[2][1]+m1[2][0]*m2[2][2]+m1[3][0]*m2[2][3]+m1[4][0]*m2[2][4]+m1[5][0]*m2[2][5];
  c[3][0] = m1[0][0]*m2[3][0]+m1[1][0]*m2[3][1]+m1[2][0]*m2[3][2]+m1[3][0]*m2[3][3]+m1[4][0]*m2[3][4]+m1[5][0]*m2[3][5];
  c[4][0] = m1[0][0]*m2[4][0]+m1[1][0]*m2[4][1]+m1[2][0]*m2[4][2]+m1[3][0]*m2[4][3]+m1[4][0]*m2[4][4]+m1[5][0]*m2[4][5];
  c[5][0] = m1[0][0]*m2[5][0]+m1[1][0]*m2[5][1]+m1[2][0]*m2[5][2]+m1[3][0]*m2[5][3]+m1[4][0]*m2[5][4]+m1[5][0]*m2[5][5];

  c[0][1] = m1[0][1]*m2[0][0]+m1[1][1]*m2[0][1]+m1[2][1]*m2[0][2]+m1[3][1]*m2[0][3]+m1[4][1]*m2[0][4]+m1[5][1]*m2[0][5];
  c[1][1] = m1[0][1]*m2[1][0]+m1[1][1]*m2[1][1]+m1[2][1]*m2[1][2]+m1[3][1]*m2[1][3]+m1[4][1]*m2[1][4]+m1[5][1]*m2[1][5];
  c[2][1] = m1[0][1]*m2[2][0]+m1[1][1]*m2[2][1]+m1[2][1]*m2[2][2]+m1[3][1]*m2[2][3]+m1[4][1]*m2[2][4]+m1[5][1]*m2[2][5];
  c[3][1] = m1[0][1]*m2[3][0]+m1[1][1]*m2[3][1]+m1[2][1]*m2[3][2]+m1[3][1]*m2[3][3]+m1[4][1]*m2[3][4]+m1[5][1]*m2[3][5];
  c[4][1] = m1[0][1]*m2[4][0]+m1[1][1]*m2[4][1]+m1[2][1]*m2[4][2]+m1[3][1]*m2[4][3]+m1[4][1]*m2[4][4]+m1[5][1]*m2[4][5];
  c[5][1] = m1[0][1]*m2[5][0]+m1[1][1]*m2[5][1]+m1[2][1]*m2[5][2]+m1[3][1]*m2[5][3]+m1[4][1]*m2[5][4]+m1[5][1]*m2[5][5];

  c[0][2] = m1[0][2]*m2[0][0]+m1[1][2]*m2[0][1]+m1[2][2]*m2[0][2]+m1[3][2]*m2[0][3]+m1[4][2]*m2[0][4]+m1[5][2]*m2[0][5];
  c[1][2] = m1[0][2]*m2[1][0]+m1[1][2]*m2[1][1]+m1[2][2]*m2[1][2]+m1[3][2]*m2[1][3]+m1[4][2]*m2[1][4]+m1[5][2]*m2[1][5];
  c[2][2] = m1[0][2]*m2[2][0]+m1[1][2]*m2[2][1]+m1[2][2]*m2[2][2]+m1[3][2]*m2[2][3]+m1[4][2]*m2[2][4]+m1[5][2]*m2[2][5];
  c[3][2] = m1[0][2]*m2[3][0]+m1[1][2]*m2[3][1]+m1[2][2]*m2[3][2]+m1[3][2]*m2[3][3]+m1[4][2]*m2[3][4]+m1[5][2]*m2[3][5];
  c[4][2] = m1[0][2]*m2[4][0]+m1[1][2]*m2[4][1]+m1[2][2]*m2[4][2]+m1[3][2]*m2[4][3]+m1[4][2]*m2[4][4]+m1[5][2]*m2[4][5];
  c[5][2] = m1[0][2]*m2[5][0]+m1[1][2]*m2[5][1]+m1[2][2]*m2[5][2]+m1[3][2]*m2[5][3]+m1[4][2]*m2[5][4]+m1[5][2]*m2[5][5];

  c[0][3] = m1[0][3]*m2[0][0]+m1[1][3]*m2[0][1]+m1[2][3]*m2[0][2]+m1[3][3]*m2[0][3]+m1[4][3]*m2[0][4]+m1[5][3]*m2[0][5];
  c[1][3] = m1[0][3]*m2[1][0]+m1[1][3]*m2[1][1]+m1[2][3]*m2[1][2]+m1[3][3]*m2[1][3]+m1[4][3]*m2[1][4]+m1[5][3]*m2[1][5];
  c[2][3] = m1[0][3]*m2[2][0]+m1[1][3]*m2[2][1]+m1[2][3]*m2[2][2]+m1[3][3]*m2[2][3]+m1[4][3]*m2[2][4]+m1[5][3]*m2[2][5];
  c[3][3] = m1[0][3]*m2[3][0]+m1[1][3]*m2[3][1]+m1[2][3]*m2[3][2]+m1[3][3]*m2[3][3]+m1[4][3]*m2[3][4]+m1[5][3]*m2[3][5];
  c[4][3] = m1[0][3]*m2[4][0]+m1[1][3]*m2[4][1]+m1[2][3]*m2[4][2]+m1[3][3]*m2[4][3]+m1[4][3]*m2[4][4]+m1[5][3]*m2[4][5];
  c[5][3] = m1[0][3]*m2[5][0]+m1[1][3]*m2[5][1]+m1[2][3]*m2[5][2]+m1[3][3]*m2[5][3]+m1[4][3]*m2[5][4]+m1[5][3]*m2[5][5];

  c[0][4] = m1[0][4]*m2[0][0]+m1[1][4]*m2[0][1]+m1[2][4]*m2[0][2]+m1[3][4]*m2[0][3]+m1[4][4]*m2[0][4]+m1[5][4]*m2[0][5];
  c[1][4] = m1[0][4]*m2[1][0]+m1[1][4]*m2[1][1]+m1[2][4]*m2[1][2]+m1[3][4]*m2[1][3]+m1[4][4]*m2[1][4]+m1[5][4]*m2[1][5];
  c[2][4] = m1[0][4]*m2[2][0]+m1[1][4]*m2[2][1]+m1[2][4]*m2[2][2]+m1[3][4]*m2[2][3]+m1[4][4]*m2[2][4]+m1[5][4]*m2[2][5];
  c[3][4] = m1[0][4]*m2[3][0]+m1[1][4]*m2[3][1]+m1[2][4]*m2[3][2]+m1[3][4]*m2[3][3]+m1[4][4]*m2[3][4]+m1[5][4]*m2[3][5];
  c[4][4] = m1[0][4]*m2[4][0]+m1[1][4]*m2[4][1]+m1[2][4]*m2[4][2]+m1[3][4]*m2[4][3]+m1[4][4]*m2[4][4]+m1[5][4]*m2[4][5];
  c[5][4] = m1[0][4]*m2[5][0]+m1[1][4]*m2[5][1]+m1[2][4]*m2[5][2]+m1[3][4]*m2[5][3]+m1[4][4]*m2[5][4]+m1[5][4]*m2[5][5];

  c[0][5] = m1[0][5]*m2[0][0]+m1[1][5]*m2[0][1]+m1[2][5]*m2[0][2]+m1[3][5]*m2[0][3]+m1[4][5]*m2[0][4]+m1[5][5]*m2[0][5];
  c[1][5] = m1[0][5]*m2[1][0]+m1[1][5]*m2[1][1]+m1[2][5]*m2[1][2]+m1[3][5]*m2[1][3]+m1[4][5]*m2[1][4]+m1[5][5]*m2[1][5];
  c[2][5] = m1[0][5]*m2[2][0]+m1[1][5]*m2[2][1]+m1[2][5]*m2[2][2]+m1[3][5]*m2[2][3]+m1[4][5]*m2[2][4]+m1[5][5]*m2[2][5];
  c[3][5] = m1[0][5]*m2[3][0]+m1[1][5]*m2[3][1]+m1[2][5]*m2[3][2]+m1[3][5]*m2[3][3]+m1[4][5]*m2[3][4]+m1[5][5]*m2[3][5];
  c[4][5] = m1[0][5]*m2[4][0]+m1[1][5]*m2[4][1]+m1[2][5]*m2[4][2]+m1[3][5]*m2[4][3]+m1[4][5]*m2[4][4]+m1[5][5]*m2[4][5];
  c[5][5] = m1[0][5]*m2[5][0]+m1[1][5]*m2[5][1]+m1[2][5]*m2[5][2]+m1[3][5]*m2[5][3]+m1[4][5]*m2[5][4]+m1[5][5]*m2[5][5];

  mat6_copy(c, dest);
}

void mat6_mulv(mat6 m, vec6 v, vec6 dest) {
  vec6 c = VEC6_ZERO_INIT;
  c[0] = m[0][0]*v[0]+m[1][0]*v[1]+m[2][0]*v[2]+m[3][0]*v[3]+m[4][0]*v[4]+m[5][0]*v[5];
  c[1] = m[0][1]*v[0]+m[1][1]*v[1]+m[2][1]*v[2]+m[3][1]*v[3]+m[4][1]*v[4]+m[5][1]*v[5];
  c[2] = m[0][2]*v[0]+m[1][2]*v[1]+m[2][2]*v[2]+m[3][2]*v[3]+m[4][2]*v[4]+m[5][2]*v[5];
  c[3] = m[0][3]*v[0]+m[1][3]*v[1]+m[2][3]*v[2]+m[3][3]*v[3]+m[4][3]*v[4]+m[5][3]*v[5];
  c[4] = m[0][4]*v[0]+m[1][4]*v[1]+m[2][4]*v[2]+m[3][4]*v[3]+m[4][4]*v[4]+m[5][4]*v[5];
  c[5] = m[0][5]*v[0]+m[1][5]*v[1]+m[2][5]*v[2]+m[3][5]*v[3]+m[4][5]*v[4]+m[5][5]*v[5];

  vec6_copy(c, dest);
}

void mat6_scale(mat6 m, float s, mat6 dest) {
  dest[0][0] = s * m[0][0];
  dest[0][1] = s * m[0][1];
  dest[0][2] = s * m[0][2];
  dest[0][3] = s * m[0][3];
  dest[0][4] = s * m[0][4];
  dest[0][5] = s * m[0][5];
  dest[1][0] = s * m[1][0];
  dest[1][1] = s * m[1][1];
  dest[1][2] = s * m[1][2];
  dest[1][3] = s * m[1][3];
  dest[1][4] = s * m[1][4];
  dest[1][5] = s * m[1][5];
  dest[2][0] = s * m[2][0];
  dest[2][1] = s * m[2][1];
  dest[2][2] = s * m[2][2];
  dest[2][3] = s * m[2][3];
  dest[2][4] = s * m[2][4];
  dest[2][5] = s * m[2][5];
  dest[3][0] = s * m[3][0];
  dest[3][1] = s * m[3][1];
  dest[3][2] = s * m[3][2];
  dest[3][3] = s * m[3][3];
  dest[3][4] = s * m[3][4];
  dest[3][5] = s * m[3][5];
  dest[4][0] = s * m[4][0];
  dest[4][1] = s * m[4][1];
  dest[4][2] = s * m[4][2];
  dest[4][3] = s * m[4][3];
  dest[4][4] = s * m[4][4];
  dest[4][5] = s * m[4][5];
  dest[5][0] = s * m[5][0];
  dest[5][1] = s * m[5][1];
  dest[5][2] = s * m[5][2];
  dest[5][3] = s * m[5][3];
  dest[5][4] = s * m[5][4];
  dest[5][5] = s * m[5][5];
}

void mat6_spatial_transform(mat3 rotate_fg, mat3 translate_fg, mat6 dest) {
  mat6 c = MAT6_ZERO_INIT;
  mat3 t = GLM_MAT3_IDENTITY_INIT;
  glm_mat3_mul(translate_fg, rotate_fg, t);

  c[0][0] = rotate_fg[0][0];
  c[0][1] = rotate_fg[0][1];
  c[0][2] = rotate_fg[0][2];
  c[1][0] = rotate_fg[1][0];
  c[1][1] = rotate_fg[1][1];
  c[1][2] = rotate_fg[1][2];
  c[2][0] = rotate_fg[2][0];
  c[2][1] = rotate_fg[2][1];
  c[2][2] = rotate_fg[2][2];

  c[0][3] = t[0][0];
  c[0][4] = t[0][1];
  c[0][5] = t[0][2];
  c[1][3] = t[1][0];
  c[1][4] = t[1][1];
  c[1][5] = t[1][2];
  c[2][3] = t[2][0];
  c[2][4] = t[2][1];
  c[2][5] = t[2][2];

  c[3][3] = rotate_fg[0][0];
  c[3][4] = rotate_fg[0][1];
  c[3][5] = rotate_fg[0][2];
  c[4][3] = rotate_fg[1][0];
  c[4][4] = rotate_fg[1][1];
  c[4][5] = rotate_fg[1][2];
  c[5][3] = rotate_fg[2][0];
  c[5][4] = rotate_fg[2][1];
  c[5][5] = rotate_fg[2][2];

  mat6_copy(c, dest);
}

void vec6_zero(vec6 v) {
  v[0] = 0.0;
  v[1] = 0.0;
  v[2] = 0.0;
  v[3] = 0.0;
  v[4] = 0.0;
  v[5] = 0.0;
}

void vec6_copy(vec6 v, vec6 dest) {
  dest[0] = v[0];
  dest[1] = v[1];
  dest[2] = v[2];
  dest[3] = v[3];
  dest[4] = v[4];
  dest[5] = v[5];
}

void vec6_compose(vec3 a, vec3 b, vec6 dest) {
  dest[0] = a[0];
  dest[1] = a[1];
  dest[2] = a[2];
  dest[3] = b[0];
  dest[4] = b[1];
  dest[5] = b[2];
}

void vec6_add(vec6 a, vec6 b, vec6 dest) {
  vec6 c = VEC6_ZERO_INIT;
  c[0] = a[0]+b[0];
  c[1] = a[1]+b[1];
  c[2] = a[2]+b[2];
  c[3] = a[3]+b[3];
  c[4] = a[4]+b[4];
  c[5] = a[5]+b[5];

  vec6_copy(c, dest);
}

void vec6_sub(vec6 a, vec6 b, vec6 dest) {
  vec6 c = VEC6_ZERO_INIT;
  c[0] = a[0]-b[0];
  c[1] = a[1]-b[1];
  c[2] = a[2]-b[2];
  c[3] = a[3]-b[3];
  c[4] = a[4]-b[4];
  c[5] = a[5]-b[5];

  vec6_copy(c, dest);
}

void vec6_scale(vec6 a, float s, vec6 dest) {
  dest[0] = a[0] * s;
  dest[1] = a[1] * s;
  dest[2] = a[2] * s;
  dest[3] = a[3] * s;
  dest[4] = a[4] * s;
  dest[5] = a[5] * s;
}

void vec6_spatial_transpose_mulm(vec6 v, mat6 m, vec6 dest) {
  dest[0] = v[3]*m[0][0]+v[4]*m[0][1]+v[5]*m[0][2]+v[0]*m[0][3]+v[1]*m[0][4]+v[2]*m[0][5];
  dest[1] = v[3]*m[1][0]+v[4]*m[1][1]+v[5]*m[1][2]+v[0]*m[1][3]+v[1]*m[1][4]+v[2]*m[1][5];
  dest[2] = v[3]*m[2][0]+v[4]*m[2][1]+v[5]*m[2][2]+v[0]*m[2][3]+v[1]*m[2][4]+v[2]*m[2][5];
  dest[3] = v[3]*m[3][0]+v[4]*m[3][1]+v[5]*m[3][2]+v[0]*m[3][3]+v[1]*m[3][4]+v[2]*m[3][5];
  dest[4] = v[3]*m[4][0]+v[4]*m[4][1]+v[5]*m[4][2]+v[0]*m[4][3]+v[1]*m[4][4]+v[2]*m[4][5];
  dest[5] = v[3]*m[5][0]+v[4]*m[5][1]+v[5]*m[5][2]+v[0]*m[5][3]+v[1]*m[5][4]+v[2]*m[5][5];
}

void vec6_spatial_transpose_mulv(vec6 a, vec6 b, mat6 dest) {
  dest[0][0] = a[0] * b[3];
  dest[1][0] = a[0] * b[4];
  dest[2][0] = a[0] * b[5];
  dest[3][0] = a[0] * b[0];
  dest[4][0] = a[0] * b[1];
  dest[5][0] = a[0] * b[2];
  dest[0][1] = a[1] * b[3];
  dest[1][1] = a[1] * b[4];
  dest[2][1] = a[1] * b[5];
  dest[3][1] = a[1] * b[0];
  dest[4][1] = a[1] * b[1];
  dest[5][1] = a[1] * b[2];
  dest[0][2] = a[2] * b[3];
  dest[1][2] = a[2] * b[4];
  dest[2][2] = a[2] * b[5];
  dest[3][2] = a[2] * b[0];
  dest[4][2] = a[2] * b[1];
  dest[5][2] = a[2] * b[2];
  dest[0][3] = a[3] * b[3];
  dest[1][3] = a[3] * b[4];
  dest[2][3] = a[3] * b[5];
  dest[3][3] = a[3] * b[0];
  dest[4][3] = a[3] * b[1];
  dest[5][3] = a[3] * b[2];
  dest[0][4] = a[4] * b[3];
  dest[1][4] = a[4] * b[4];
  dest[2][4] = a[4] * b[5];
  dest[3][4] = a[4] * b[0];
  dest[4][4] = a[4] * b[1];
  dest[5][4] = a[4] * b[2];
  dest[0][5] = a[5] * b[3];
  dest[1][5] = a[5] * b[4];
  dest[2][5] = a[5] * b[5];
  dest[3][5] = a[5] * b[0];
  dest[4][5] = a[5] * b[1];
  dest[5][5] = a[5] * b[2];
}

float vec6_dot(vec6 a, vec6 b) {
  float c = a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]+a[4]*b[4]+a[5]*b[5];
  return c;
}

float vec6_inner_product(vec6 a, vec6 b) {
  float c = a[3]*b[0]+a[4]*b[1]+a[5]*b[2]+a[0]*b[3]+a[1]*b[4]+a[2]*b[5];
  return c;
}

void vec6_remove_noise(vec6 vec, float threshold) {
  if (vec[0] < threshold && vec[0] > -threshold) {
    vec[0] = 0.0;
  }
  if (vec[1] < threshold && vec[1] > -threshold) {
    vec[1] = 0.0;
  }
  if (vec[2] < threshold && vec[2] > -threshold) {
    vec[2] = 0.0;
  }
  if (vec[3] < threshold && vec[3] > -threshold) {
    vec[3] = 0.0;
  }
  if (vec[4] < threshold && vec[4] > -threshold) {
    vec[4] = 0.0;
  }
  if (vec[5] < threshold && vec[5] > -threshold) {
    vec[5] = 0.0;
  }
}

void vec3_singular_cross(vec3 a, mat3 dest) {
  dest[0][0] = 0.0;
  dest[0][1] = a[2];
  dest[0][2] = -a[1];
  dest[1][0] = -a[2];
  dest[1][1] = 0.0;
  dest[1][2] = a[0];
  dest[2][0] = a[1];
  dest[2][1] = -a[0];
  dest[2][2] = 0.0;
}
