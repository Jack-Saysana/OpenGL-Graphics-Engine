#include <math/givens.h>

/*
  Calculates the givens matrix P to zero the sub-diagonal element of the ith
  column of some mxn matrix A when A and P are multiplied: P*A
  Arguments:
  - amat a: Matrix to "clean"
  - amat dest: Matrix to store the givens matrix
  - int i: Index of column in a that dest will "clean"
*/
void giv_clean_col(amat a, amat dest, int i) {
  amat_identity(dest);
  amat givens = init_amat(NULL, 2, 2);
  calc_givens(AMAT_GET(a,i,i), AMAT_GET(a,i,i+1), givens);

  amat_ins(givens, dest, i, i);

  free_amat(givens);
}

/*
  Calculates the givens matrix V to zero the element of the ith row, which is 2
  elements left of the main diagonal, of some mxn matrix A when A and V are
  multiplied: A*V
  Arguments:
  - amat a: Matrix to "clean"
  - amat dest: Matrix to store the givens matrix
  - int i: Index of row in a that dest will "clean"
*/
void giv_clean_row(amat a, amat dest, int i) {
  amat_identity(dest);
  amat givens = init_amat(NULL, 2, 2);
  calc_givens(AMAT_GET(a,i+1,i), AMAT_GET(a,i+2,i), givens);

  amat_transpose(givens, givens);
  amat_ins(givens, dest, i+1, i+1);

  free_amat(givens);
}

/*
  Computes a 2x2 givens matrix. Assume A=[a,b]. The givens matrix G is
  calculated such that G*A=[r,0]
  Arguments:
  - float a: First element in vector to be multiplied by givens matrix
  - float b: Second element in vector to be multiplied by givens matrix
  - amat dest: Preallocated 2x2 matrix to store calculated givens matrix
*/
void calc_givens(float f, float g, amat dest) {
  float c, s, t;
  if (g == 0.0) {
    c = 1.0;
    s = 0.0;
  } else if (fabs(g) > fabs(f)) {
    t = -f / g;
    s = 1.0 / sqrt(1.0 + (t * t));
    c = s * t;
  } else {
    t = - g / f;
    c = 1.0 / sqrt(1.0 + (t * t));
    s = c * t;
  }

  dest.data[0] = c;
  dest.data[1] = s;
  dest.data[2] = -s;
  dest.data[3] = c;
}
