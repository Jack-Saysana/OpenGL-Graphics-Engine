#include <math/svd.h>

/*
  Computes the singular value decomposition of a matrix A. The SVD is as
  follows: A=US(V^T)
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

  // B_(k+1) = (u^T)_k * B_k * v_k
  // Where v_k = r_1 * r_2 * ... * r_Z
  // and u_k = t_1 * t_2 * ... * t_P
  for (int i = 0; i < MAX_SVD_ITERATIONS; i++) {
    // Calculate and apply the "special" r_1 matrix
    calc_r1(S, r);
    amat_mul(S, r, S);
    amat_mul(V, r, V);

    // Clean up the mess by re-bidiagonalizing S
    // (applying r_2,..r_Z and t_1,...t_P)
    for (int j = 0; j < S.m || j < S.n; j++) {
      // Clean col
      if (j < S.m - 1) {
        hh_clean_col(S, t, j);
        amat_mul(t, S, S);
        amat_mul(t, U, U);
      }

      // Clean row
      if (j < a.n - 2) {
        hh_clean_row(S, r, j);
        amat_mul(S, r, S);
        amat_mul(V, r, V);
      }
    }

    if (super_diag_elem(S) < SVD_SUPER_DIAG_THRESHOLD) {
      break;
    }
  }

  amat_transpose(U, U);
  free_amat(t);
  free_amat(r);
}

void calc_r1(amat b, amat dest) {
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

  hh_clean_col(temp, dest, 0);
  amat_transpose(dest, dest);

  free_amat(bt);
  free_amat(temp);
}

float super_diag_elem(amat b) {
  float total = 0.0;
  int cnt = 0;
  for (int i = 1; i < b.n && i < b.m; i++) {
    total += fabs(AMAT_GET(b, i, i-1));
    cnt++;
  }

  return total / ((float) cnt);
}

// Calculate the eiegenvalue of a 2x2 matrix which is closest to the value of
// the bottom right element of said matrix
float calc_phi(float a, float b, float c, float d) {
  float first = a + d;
  float second = ((a+d)*(a+d))-(4.0*((b*c)+(a*d)));
  if (second < 0.0) {
    return 0.0;
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
