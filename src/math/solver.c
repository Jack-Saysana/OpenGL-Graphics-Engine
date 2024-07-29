#include <math/solver.h>

/*
  Solves a linear system of equations in the form AX=B
  Arguements:
  - amat a: m x n matrix A
  - amat b: m x 1 matrix B
  - amat x: preallocated n x 1 matrix X
*/
void solve_system(amat a, amat b, amat x) {
  amat A, B, C;
  int *row_history = malloc(sizeof(int) * a.m);
  int *col_history = malloc(sizeof(int) * a.n);
  preprocess_system(a, b, x, &A, &B, &C, row_history, col_history);

  int singular = lu_solve(A, B, C);
  if (!singular) {
    postprocess_sol(x, C, col_history);

    free_amat(A);
    free_amat(B);
    free_amat(C);
    free(row_history);
    free(col_history);
    return;
  }
  amat D, E, F;
  if (A.m < A.n) {
    D = init_amat(NULL, A.n, A.n);
    E = init_amat(NULL, A.n, 1);
    F = init_amat(NULL, A.n, 1);
  } else {
    D = init_amat(NULL, A.m, A.n);
    E = init_amat(NULL, B.m, B.n);
    F = init_amat(NULL, C.m, C.n);
  }
  amat_ins(A, D, 0, 0);
  amat_ins(B, E, 0, 0);

  amat U = init_amat(NULL, D.m, D.m);
  amat S = init_amat(NULL, D.m, D.n);
  amat S_T = init_amat(NULL, D.n, D.m);
  amat V = init_amat(NULL, D.n, D.n);
  svd(D, U, S, V);
  amat_transpose(S, S_T);
  amat_transpose(U, U);

  float cur = 0.0;
  for (int i = 0; i < S_T.n && i < S_T.m; i++) {
    cur = AMAT_GET(S_T, i, i);
    if (fabs(cur) > ZERO_THRESHOLD) {
      AMAT_GET(S_T, i, i) = 1.0 / cur;
    } else {
      cur = 0.0;
    }
  }

  amat_mul(V, S_T, S_T);
  amat_mul(S_T, U, S_T);
  amat_mul(S_T, E, F);
  amat_ins(F, C, 0, 0);
  postprocess_sol(x, C, col_history);

  free_amat(U);
  free_amat(S);
  free_amat(S_T);
  free_amat(V);
  free_amat(A);
  free_amat(B);
  free_amat(C);
  free_amat(D);
  free_amat(E);
  free_amat(F);
  free(row_history);
  free(col_history);
}

/*
  Eliminates zero rows and columns from a system of equations to simplify the
  linear system
  Arguments:
  - amat a: Original a matrix in system ac=b
  - amat b: Original b vector
  - amat c: Original c vector
  - amat *A: Resized a matrix with zero rows removed
  - amat *B: Resized b vector with entries corresponding to zero rows in a
             removed
  - amat *C: Resized c vector with entries corresponding to zero columns in a
             removed
  - int *row_history: Buffer of flags denoting if a row of A is zeroed out;
                      1 if so, 0 if not
  - int *col_history: Buffer of flags denoting if a column of A is zeroed out;
                      1 if so, 0 if not
*/
void preprocess_system(amat a, amat b, amat c, amat *A, amat *B, amat *C,
                       int *row_history, int *col_history) {
  for (int i = 0; i < a.m; i++) {
    row_history[i] = 0;
  }
  for (int i = 0; i < a.n; i++) {
    col_history[i] = 0;
  }

  int rows_used = 0;
  int cols_used = 0;
  for (int i = 0; i < a.m; i++) {
    for (int j = 0; j < a.n; j++) {
      if (fabs(AMAT_GET(a, j, i)) >= ZERO_THRESHOLD) {
        if (!row_history[i]) {
          row_history[i] = 1;
          rows_used++;
        }
        if (!col_history[j]) {
          col_history[j] = 1;
          cols_used++;
        }
      }
    }
  }

  (*A) = init_amat(NULL, rows_used, cols_used);
  (*B) = init_amat(NULL, rows_used, 1);
  (*C) = init_amat(NULL, cols_used, 1);

  rows_used = 0;
  for (int i = 0; i < a.m; i++) {
    if (!row_history[i]) {
      continue;
    }
    cols_used = 0;
    for (int j = 0; j < a.n; j++) {
      if (!col_history[j]) {
        continue;
      }
      AMAT_GET(*A, cols_used, rows_used) = AMAT_GET(a, j, i);
      cols_used++;
    }
    AMAT_GET(*B, 0, rows_used) = AMAT_GET(b, 0, i);
    rows_used++;
  }
}

/*
  Reverts the preprocessing done in preprocess_system() on the solution vector,
  placing the final solution into a properly sized vector
  Arguemnts:
  - amat c: originally sized solution vector
  - amat C: solution vector with preprocessed size and solution values
  - int *col_history: Array of flags denoting if a row of c is present in C,
                      1 if so, 0 if not
*/
void postprocess_sol(amat c, amat C, int *col_history) {
  int cur_row = 0;
  for (int i = 0; i < c.m; i++) {
    if (col_history[i]) {
      AMAT_GET(c, 0, i) = AMAT_GET(C, 0, cur_row);
      cur_row++;
    } else {
      AMAT_GET(c, 0, i) = 0.0;
    }
  }
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
    if (fabs(div) <= ZERO_THRESHOLD) {
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
    if (fabs(div) <= ZERO_THRESHOLD) {
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
