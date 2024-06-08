#include <math/householder.h>

/*
  Calculates a householder matrix P to "clean" the ith column of some mxn
  matrix A when A and P are multiplied: P*A
  Arguments:
  - amat a: Matrix to "clean"
  - amat dest: Matrix to store the householder matrix
  - amat i: Index of column in a that dest will "clean"
*/
void hh_clean_col(amat a, amat dest, int i) {
  amat u = init_amat(NULL, a.m, 1);
  // u[0...i-1] = 0.0
  for (int j = 0; j < i; j++) {
    u.data[j] = 0.0;
  }
  // u[i] = a[i, i] + sqrt(x[i, i]^2 + ... + x[i,m-1]^2)
  float sum = 0.0;
  for (int j = i; j < a.m; j++) {
    sum += (AMAT_GET(a, i, j) * AMAT_GET(a, i, j));
  }
  u.data[i] = sqrt(sum) + AMAT_GET(a, i, i);
  // u[j=i+1,...,m-1] = a[i,j]
  for (int j = i+1; j < a.m; j++) {
    u.data[j] = AMAT_GET(a, i, j);
  }

  // 2/|u|^2
  float mag = 0.0;
  for (int j = 0; j < u.m; j++) {
    mag += (u.data[j] * u.data[j]);
  }
  mag = 2.0 / mag;

  // (2/|u|^2) * u*u^T
  amat_outer(u, u, dest);
  amat_scale(dest, dest, mag);

  // I - ((2/|u|^2) * u*u^T)
  amat identity = init_amat(NULL, a.m, a.m);
  amat_identity(identity);
  amat_sub(identity, dest, dest);

  free_amat(u);
  free_amat(identity);
}


/*
  Calculates a householder matrix V to "clean" the ith row of some mxn matrix A
  when A and V are multiplied: A*V
  Arguments:
  - amat a: Matrix to "clean"
  - amat dest: Matrix to store the householder matrix
  - amat i: Index of row in a that dest will "clean"
*/
void hh_clean_row(amat a, amat dest, int i) {
  amat v = init_amat(NULL, a.n, 1);
  // v[0...i] = 0.0
  for (int j = 0; j <= i; j++) {
    v.data[j] = 0.0;
  }
  // u[i+1] = a[i+1, i] + sqrt(x[i+1, i]^2 + ... + x[m-1, i]^2)
  float sum = 0.0;
  for (int j = i+1; j < a.n; j++) {
    sum += (AMAT_GET(a, j, i) * AMAT_GET(a, j, i));
  }
  v.data[i+1] = sqrt(sum) + AMAT_GET(a, i+1, i);
  // u[j=i+2,...,m-1] = a[i,j]
  for (int j = i+2; j < a.n; j++) {
    v.data[j] = AMAT_GET(a, j, i);
  }

  // 2/|v|^2
  float mag = 0.0;
  for (int j = 0; j < v.m; j++) {
    mag += (v.data[j] * v.data[j]);
  }
  mag = 2.0 / mag;

  // (2/|v|^2) * v*v^T
  amat_outer(v, v, dest);
  amat_scale(dest, dest, mag);

  // I - ((2/|v|^2) * v*v^T)
  amat identity = init_amat(NULL, a.m, a.m);
  amat_identity(identity);
  amat_sub(identity, dest, dest);

  free_amat(v);
  free_amat(identity);
}
