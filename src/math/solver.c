#include <math/solver.h>

/*
  Solves a linear system of equations in the form AX=B
  Arguements:
  - amat a: m x n matrix A
  - amat b: m x 1 matrix B
  - amat x: preallocated n x 1 matrix X
*/
void solve_system(amat a, amat b, amat x) {
  int singular = lu_solve(a, b, x);
  if (!singular) {
    return;
  }

  amat U = init_amat(NULL, a.m, a.m);
  amat S = init_amat(NULL, a.m, a.n);
  amat S_T = init_amat(NULL, a.n, a.m);
  amat V = init_amat(NULL, a.n, a.n);
  svd(a, U, S, V);
  amat_transpose(S, S_T);
  amat_transpose(U, U);

  float cur = 0.0;
  for (int i = 0; i < S_T.n && i < S_T.m; i++) {
    cur = AMAT_GET(S_T, i, i);
    if (cur) {
      AMAT_GET(S_T, i, i) = 1.0 / cur;
    }
  }

  amat_mul(V, S_T, S_T);
  amat_mul(S_T, U, S_T);
  amat_mul(S_T, b, x);
}

/*
  Solves a linear system of equations of the form AX=B, where A is an
  upper-triangular matrix
  Arguments:
  - amat a: Upper triangular, non-singular matrix A of size m x m of system
            AX=B
  - amat b: Vector B of size m x 1 of system AX=B
  - amat x: Vector X of size m x 1 of system AX=B, will be populated
  Returns:
  -1 if input is invalid, 0 if successful

*/
int solve_upper(amat a, amat b, amat x) {
  if (a.n != a.m || a.n != b.m || a.m != x.m) {
    return -1;
  }

  float sum = 0.0;
  float div = 0.0;
  for (int i = a.m - 1; i >= 0; i--) {
    div = AMAT_GET(a, i, i);
    if (fabs(div) <= 0.00001) {
      return -1;
    }
    sum = 0.0;
    for (int j = a.m - 1; j > i; j--) {
      sum += (AMAT_GET(a, j, i) * AMAT_GET(x, 0, j));
    }

    AMAT_GET(x, 0, i) = (AMAT_GET(b, 0, i) - sum) / div;
  }

  return 0;
}

/*
  Solves a linear system of equations of the form AX=B, where A is a
  lower-triangular matrix
  Arguments:
  - amat a: Lower triangular, non-singular matrix A of size m x m of system
            AX=B
  - amat b: Vector B of size m x 1 of system AX=B
  - amat x: Vector X of size m x 1 of system AX=B, will be populated
  Returns:
  -1 if input is invalid, 0 if successful
*/
int solve_lower(amat a, amat b, amat x) {
  if (a.n != a.m || a.n != b.m || a.m != x.m) {
    return -1;
  }

  float sum = 0.0;
  float div = 0.0;
  for (int i = 0; i < a.m; i++) {
    div = AMAT_GET(a, i, i);
    if (fabs(div) <= 0.00001) {
      return -1;
    }
    sum = 0.0;
    for (int j = 0; j < i; j++) {
      sum += (AMAT_GET(a, j, i) * AMAT_GET(x, 0, j));
    }

    AMAT_GET(x, 0, i) = (AMAT_GET(b, 0, i) - sum) / div;
  }

  return 0;
}
