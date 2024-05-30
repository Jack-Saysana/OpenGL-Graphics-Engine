#include <math/solver.h>

/*
  Solves a linear system of equations in the form AX=B
  Arguements:
  TODO Populate
  Returns:
  TODO Populate
*/
int solve_system(amat a, amat b, amat x) {
  return 0;
}

/*
  Solves a linear system of equations of the form AX=B, where A is an
  upper-triangular matrix
  Arguments:
  TODO Populate
  Returns:
  TODO Populate
*/
int solve_upper(amat a, amat b, amat x) {
  if (a.n != a.m || a.n != b.m || a.m != x.m) {
    return -1;
  }

  float sum = 0.0;
  float div = 0.0;
  for (int i = a.m - 1; i >= 0; i--) {
    div = AMAT_GET(a, i, i);
    if (abs(div) <= 0.00001) {
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
  TODO Populate
  Returns:
  TODO Populate
*/
int solve_lower(amat a, amat b, amat x) {
  if (a.n != a.m || a.n != b.m || a.m != x.m) {
    return -1;
  }

  float sum = 0.0;
  float div = 0.0;
  for (int i = 0; i < a.m; i++) {
    div = AMAT_GET(a, i, i);
    if (abs(div) <= 0.00001) {
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
