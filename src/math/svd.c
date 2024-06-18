#include <math/svd.h>

/*
  Computes the singular value decomposition of a matrix A. The SVD is as
  follows: A=US(V^T). ASSUMPTION: m >= n
  Arguments:
  - amat a: m x n Matrix to decompose
  - amat U: preallocated m x m matrix to store U of the svd
  - amat S: preallocated m x n matrix to store S of the svd
  - amat V: preallocated n x n matrix to store V of the svd
*/
void svd(amat a, amat U, amat S, amat V) {
  amat_identity(U);
  amat_identity(V);
  amat_copy(a, S);

  amat t = init_amat(NULL, S.m, S.m);
  amat r = init_amat(NULL, S.n, S.n);
  // Create bi-directional matrix using householder matrices
  for (int i = 0; i < a.n && i < a.m; i++) {
    if (i < a.m - 1) {
      hh_clean_col(S, t, i);
      amat_mul(t, S, S);
      amat_mul(t, U, U);
    }
    if (i < a.n - 2) {
      hh_clean_row(S, r, i);
      amat_mul(S, r, S);
      amat_mul(V, r, V);
    }
  }

  int f = 0;
  int g = a.n - 1;
  amat giv = init_amat(NULL, 2, 2);
  for (int k = 0; g > 0 && k < MAX_SVD_ITERATIONS * S.n; k++) {
    // Only decompose sub-matrices with non-zero super-diagonal values
    if (fabs(AMAT_GET(S, g, g-1)) <= SVD_SUPER_DIAG_THRESHOLD) {
      g--;
      continue;
    }

    f = g - 1;
    while (f > 0) {
      if (fabs(AMAT_GET(S, f, f-1)) <= SVD_SUPER_DIAG_THRESHOLD) {
        break;
      }
      f--;
    }

    // Calculate and apply the "special" r_1 matrix
    calc_r1(S, f, g - f + 1, r);
    amat_mul(S, r, S);
    amat_mul(V, r, V);

    // Clean up the mess by re-bidiagonalizing S
    // (applying r_2,..r_Z and t_1,...t_P)
    for (int j = f; j < g; j++) {
      // Clean col
      if (j < g) {
        giv_clean_col(S, t, j);
        amat_mul(t, S, S);
        amat_mul(t, U, U);
      }

      // Clean row
      if (j < g - 1) {
        giv_clean_row(S, r, j);
        amat_mul(S, r, S);
        amat_mul(V, r, V);
      }
    }

    // Chase zeros out of diagonal
    for (int j = f; j < g; j++) {
      if (fabs(AMAT_GET(S, j, j)) > SVD_SUPER_DIAG_THRESHOLD) {
        continue;
      }
      for (int i = j; i < g; i++) {
        calc_givens(AMAT_GET(S, i+1, i), AMAT_GET(S, i+1, i+1), giv);
        amat_identity(t);
        amat_ins(giv, t, i, i);
        amat_mul(t, S, S);
        amat_mul(t, U, U);
      }
      for (int i = g; i > j; i--) {
        amat_identity(t);
        AMAT_GET(t, i-1, i-1) = 0.0;
        AMAT_GET(t, i-1, i) = 1.0;
        AMAT_GET(t, i, i-1) = 1.0;
        AMAT_GET(t, i, i) = 0.0;
        amat_mul(t, S, S);
        amat_mul(t, U, U);
      }
    }
  }

  // Make all singular values positive
  for (int i = 0; i < S.m && i < S.n; i++) {
    if (AMAT_GET(S, i, i) >= 0.0) {
      continue;
    }

    AMAT_GET(S, i, i) *= -1.0;
    // Apply the reflection to the corresponding column of V
    for (int j = 0; j < V.m; j++) {
      AMAT_GET(V, i, j) *= -1.0;
    }
  }

  amat_transpose(U, U);
  free_amat(giv);
  free_amat(t);
  free_amat(r);
}

void calc_r1(amat a, int offset, int size, amat dest) {
  amat b = init_amat(NULL, size, size);
  amat_pick(a, b, offset, offset);
  amat bt = init_amat(NULL, b.n, b.m);
  amat_transpose(b, bt);

  amat temp = init_amat(NULL, b.n, b.n);
  amat_mul(bt, b, temp);
  float phi = calc_phi(AMAT_GET(temp,temp.n-2,temp.m-2),  // upper left
                       AMAT_GET(temp,temp.n-1,temp.m-2),  // upper right
                       AMAT_GET(temp,temp.n-2,temp.m-1),  // lower left
                       AMAT_GET(temp,temp.n-1,temp.m-1)); // lower right
  for (int i = 0; i < temp.n; i++) {
    AMAT_GET(temp, i, i) -= phi;
  }

  amat_identity(dest);
  giv_clean_col(temp, b, 0);
  amat_ins(b, dest, offset, offset);
  amat_transpose(dest, dest);

  free_amat(bt);
  free_amat(temp);
}

float super_diag_elem(amat b) {
  float cur = 0.0;
  float min = 1.0;
  for (int i = 1; i < b.n && i < b.m; i++) {
    cur = fabs(AMAT_GET(b, i, i-1));
    if (cur < min) {
      min = cur;
    }
  }
  return min;
}

// Calculate the eiegenvalue of a 2x2 matrix which is closest to the value of
// the bottom right element of said matrix
float calc_phi(float a, float b, float c, float d) {
  float first = a + d;
  float second = ((a+d)*(a+d))-(4.0*((b*c)+(a*d)));
  if (second < 0.0) {
    return 1.0;
  } else {
    second = sqrt(second);
  }
  float phi1 = 0.5 * (first + second);
  float phi2 = 0.5 * (first - second);
  if (fabs(phi1 - d) < fabs(phi2 - d)) {
    return phi1;
  } else {
    return phi2;
  }
}
