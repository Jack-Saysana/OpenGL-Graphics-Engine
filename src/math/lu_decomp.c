#include <math/lu_decomp.h>

/*
  Attemps to perfom a LU-decomposition-based solver on a matrix A.
  Arguments:
  - amat a: Matrix A which to attempt to solve using LU-decomposition in the
            system AX=B. Must be non-singular of size mxm to work.
  - amat b: Vector B in the system AX=B
  - amat x: Solution vector to populate in the event a can be solved using LU-
            decomposition. Must be preallocated of size mx1.
  -1 if a is singular or the inputs are invalid, 0 if successful
*/
int lu_solve(amat a, amat b, amat x) {
  if (a.m != a.n) {
    return -1;
  }

  amat pivoted_a = init_amat(NULL, a.m, a.n);
  amat pivoted_b = init_amat(NULL, b.m, b.n);
  amat_copy(a, pivoted_a);
  amat_copy(b, pivoted_b);

  partial_pivot(pivoted_a, pivoted_b);

  amat l = init_amat(NULL, pivoted_a.m, pivoted_a.n);
  amat u = init_amat(NULL, pivoted_a.m, pivoted_a.n);
  lu_decomp(pivoted_a, l, u);

  amat y = init_amat(NULL, x.m, 1);
  if (solve_lower(l, pivoted_b, y) == -1 ||
      solve_upper(u, y, x) == -1) {
    free_amat(pivoted_a);
    free_amat(pivoted_b);
    free_amat(l);
    free_amat(u);
    free_amat(y);
    return -1;
  }

  free_amat(pivoted_a);
  free_amat(pivoted_b);
  free_amat(l);
  free_amat(u);
  free_amat(y);
  return 0;
}

void lu_decomp(amat a, amat l, amat u) {
  // Initialize diagonal of l to all 1.0
  for (int i = 0; i < l.m; i++) {
    AMAT_GET(l, i, i) = 1.0;
  }

  float sum = 0.0;
  for (int j = 0; j < a.m; j++) {
    for (int i = 0; i <= j; i++) {
      sum = 0.0;
      for (int k = 0; k < i; k++) {
        sum += (AMAT_GET(l, k, i) * AMAT_GET(u, j, k));
      }

      AMAT_GET(u, j, i) = AMAT_GET(a, j, i) - sum;
    }

    for (int i = j+1; i < a.m; i++) {
      sum = 0.0;
      for (int k = 0; k < j; k++) {
        sum += (AMAT_GET(l, k, i) * AMAT_GET(u, j, k));
      }

      AMAT_GET(l, j, i) = (AMAT_GET(a, j, i) - sum) / AMAT_GET(u, j, j);
    }
  }
}

void partial_pivot(amat a, amat b) {
  float *implicit_scales = malloc(sizeof(float) * a.m);
  if (!implicit_scales) {
    return;
  }

  // Calculate implicit scaling of each row of a, so pivot comparison can be
  // normalized
  float cur = 0.0;
  for (int i = 0; i < a.m; i++) {
    implicit_scales[i] = 0.0;
    for (int j = 0; j < a.n; j++) {
      cur = fabs(AMAT_GET(a, j, i));
      if (cur > implicit_scales[i]) {
        implicit_scales[i] = cur;
      }
    }
    if (fabs(implicit_scales[i]) <= ZERO_THRESHOLD) {
      free(implicit_scales);
      return;
    }
    implicit_scales[i] = 1.0 / implicit_scales[i];
  }

  // Swap rows such that diagonal is populated with large values
  float max = 0.0;
  int swap = -1;
  for (int i = 0; i < a.m; i++) {
    max = 0.0;
    swap = -1;
    for (int j = i; j < a.m; j++) {
      cur = implicit_scales[j] * fabs(AMAT_GET(a, i, j));
      if (cur > max) {
        swap = j;
        max = cur;
      }
    }
    if (swap == -1) {
      free(implicit_scales);
      return;
    } else if (swap != i) {
      amat_swap_row(a, a, i, swap);
      amat_swap_row(b, b, i, swap);

      cur = implicit_scales[i];
      implicit_scales[i] = implicit_scales[swap];
      implicit_scales[swap] = cur;
    }
  }

  free(implicit_scales);
}

void unwrap_pivot(amat x, int *swap_log) {
  for (int i = x.m - 1; i >= 0; i--) {
    amat_swap_row(x, x, i, swap_log[i]);
  }
}
