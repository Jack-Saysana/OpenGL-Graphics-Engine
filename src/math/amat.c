#include <math/amat.h>

/*
  ================================== AMAT.H ==================================

  Provides the functionality for working with arbitrarily sized matrices

  ============================================================================
*/

/*
  Initilizes an m x n matrix
  Arguments:
  - float *data: Initial content of new matrix. NULL if matrix should be zeroed
                 out.
  - int m: Number of rows
  - int n: Number of columns
  Returns:
  - The newly initialized matrix. The returned amat's data will be set to NULL
    if an error has occured
*/
amat init_amat(float *data, int m, int n) {
  amat ret = {
    malloc(sizeof(float) * m * n),
    m,
    n
  };
  if (data) {
    memcpy(ret.data, data, sizeof(float) * m * n);
  } else {
    memset(ret.data, 0, sizeof(float) * m * n);
  }
  return ret;
}

/*
  Frees the memory associated with an allocated amat
  Arguments:
  - amat a: Matrix to free
*/
void free_amat(amat a) {
  free(a.data);
}

/*
  Copy a matrix
  Arguments:
  - amat src: Source matrix of size m x n
  - amat dest: Destination matrix. Must be preallocated of size m x n
  Returns:
  -1 if input is invalid, 0 if successful
*/
int amat_copy(amat src, amat dest) {
  if (!dest.data || src.m != dest.m || src.n != dest.n) {
    return -1;
  }

  for (int i = 0; i < dest.n; i++) {
    for (int j = 0; j < dest.m; j++) {
      AMAT_GET(dest, i, j) = AMAT_GET(src, i, j);
    }
  }

  return 0;
}

/*
  Zeros out the contents of a matrix
  Arguments:
  - amat a: Matrix to zero
*/
void amat_zero(amat a) {
  for (int i = 0; i < a.m * a.n; i++) {
    a.data[i] = 0.0;
  }
}

/*
  Zeros a matrix and puts 1.0 across the main diagonal
  Arguments:
  - amat a: Matrix to manipulate
*/
void amat_identity(amat a) {
  amat_zero(a);
  for (int i = 0; i < a.m && i < a.n; i++) {
    AMAT_GET(a, i, i) = 1.0;
  }
}

/*
  Scales all elements in a matrix by a scalar
  Arguments:
  - amat a: Matrix to mulitiply by scalar
  - amat dest: Destination matrix, can be a
  - float s: Scalar to multiply each element of a by
  Returns:
  -1 if input is invalid, 0 if successful
*/
int amat_scale(amat a, amat dest, float s) {
  if (!dest.data || a.m != dest.m || a.n != dest.n) {
    return -1;
  }

  for (int i = 0; i < a.m * a.n; i++) {
    dest.data[i] = a.data[i] * s;
  }

  return 0;
}

/*
  Computes the sum of two m x n matrices: a + b
  Arguments:
  - amat a: Matrix a of size m x n (row x col)
  - amat b: Matrix b of size m x n (row x col)
  - amat dest: Place to store the computed sum. It is safe if a or b is used as
    the dest. Must be pre-allocated of size m x n.
  Returns:
  -1 if input is invalid, 0 if successful
*/
int amat_add(amat a, amat b, amat dest) {
  if (!dest.data || a.m != b.m || a.n != b.n || a.m != dest.m ||
      a.n != dest.n) {
    return -1;
  }

  for (int i = 0; i < dest.n; i++) {
    for (int j = 0; j < dest.m; j++) {
      AMAT_GET(dest, i, j) = AMAT_GET(a, i, j) + AMAT_GET(b, i, j);
    }
  }

  return 0;
}

/*
  Computes the difference of two m x n matrices: a - b
  Arguments:
  - amat a: Matrix a of size m x n (row x col)
  - amat b: Matrix b of size m x n (row x col)
  - amat dest: Place to store the computed difference. It is safe if a or b is
    used as the dest. Must be pre-allocated of size m x n.
  Returns:
  -1 if input is invalid, 0 if successful
*/
int amat_sub(amat a, amat b, amat dest) {
  if (!dest.data || a.m != b.m || a.n != b.n || a.m != dest.m ||
      a.n != dest.n) {
    return -1;
  }

  for (int i = 0; i < dest.n; i++) {
    for (int j = 0; j < dest.m; j++) {
      AMAT_GET(dest, i, j) = AMAT_GET(a, i, j) - AMAT_GET(b, i, j);
    }
  }

  return 0;
}

/*
  Computes the product of two matrices: ab
  Arguments:
  - amat a: Matrix a of size m x n (row x col)
  - amat b: Matrix b of size n x o (row x col)
  - amat dest: Place to store the computed product. It is safe if a or b is
    used as the dest if they are sized correctly. Must be pre-allocated of
    size m x o.
  Returns:
  -1 if error occured, 0 if successful
*/
int amat_mul(amat a, amat b, amat dest) {
  if (!dest.data || a.n != b.m || dest.m != a.m || dest.n != b.n) {
    return -1;
  }

  amat c = init_amat(NULL, dest.m, dest.n);
  if (!c.data) {
    return -1;
  }

  float val = 0.0;
  for (int i = 0; i < dest.n; i++) {
    for (int j = 0; j < dest.m; j++) {
      val = 0.0;
      for (int k = 0; k < a.n; k++) {
        val += AMAT_GET(a, k, j) * AMAT_GET(b, i, k);
      }
      AMAT_GET(c, i, j) = val;
    }
  }

  memcpy(dest.data, c.data, sizeof(float) * dest.m * dest.n);
  free_amat(c);
  return 0;
}

/*
  Computes the transpose of a matrix
  Arguments:
  - amat a: Matrix a of size m x n (row x col)
  - amat dest: Place to store the computed transpose. It is safe if a is used
    as the dest if a is sized appropriately. Must be pre-allocated of size
    n x m.
  Returns:
  -1 if input is invalid, 0 if successful
*/
int amat_transpose(amat a, amat dest) {
  if (!dest.data || a.m != dest.n || a.n != dest.m) {
    return -1;
  }

  amat c = init_amat(NULL, dest.m, dest.n);
  if (!c.data) {
    return -1;
  }

  for (int i = 0; i < dest.n; i++) {
    for (int j = 0; j < dest.m; j++) {
      AMAT_GET(c, i, j) = AMAT_GET(a, j, i);
    }
  }

  memcpy(dest.data, c.data, sizeof(float) * dest.m * dest.n);
  free_amat(c);
  return 0;
}

/*
  Computes the inner/dot product of two m x 1 vectors: a dot b
  Arguments:
  - amat a: Matrix a of size m x 1 (row x col)
  - amat b: Matrix b of size m x 1 (row x col)
  - float *dest: Place to store the computed inner product.
  Returns:
  - The computed inner product
*/
int amat_dot(amat a, amat b, float *dest) {
  if (!dest || a.m != b.m) {
    return -1;
  }

  float sum = 0;
  for (int i = 0; i < a.m; i++) {
    sum += (AMAT_GET(a, 0, i) * AMAT_GET(b, 0, i));
  }
  *dest = sum;

  return 0;
}

/*
  Computes the outer product of two m x 1 vectors: a outer b
  Arguments:
  - amat a: Matrix a of size m x 1 (row x col)
  - amat b: Matrix b of size m x 1 (row x col)
  - amat dest: Place to store the computed outer product. Must be pre-allocated
    of size m x m.
  Returns:
  -1 if input is invalid, 0 if successful
*/
int amat_outer(amat a, amat b, amat dest) {
  if (!dest.data || a.m != b.m || a.m != dest.m || dest.m != dest.n) {
    return -1;
  }

  amat c = init_amat(NULL, dest.m, dest.n);
  if (!c.data) {
    return -1;
  }

  for (int i = 0; i < dest.n; i++) {
    for (int j = 0; j < dest.m; j++) {
      AMAT_GET(c, i, j) = AMAT_GET(a, 0, j) * AMAT_GET(b, 0, i);
    }
  }

  memcpy(dest.data, c.data, sizeof(float) * dest.m * dest.n);
  free_amat(c);
  return 0;
}

/*
  Swaps two rows in an m x n matrix
  Arguments:
  - amat a: Matrix a of size m x n
  - amat dest: Place to store the computed swap. It is safe for a to be dest.
  - int u: Zero indexed row to swap with row v
  - int v: Zero indexed row to swap with row u
  Returns:
  -1 if input is invalid, 0 if successful
*/
int amat_swap_row(amat a, amat dest, int u, int v) {
  if (!dest.data || a.m != dest.m || a.n != dest.n || u < 0 || u >= a.m ||
      v < 0 || v >= a.m) {
    return -1;
  }

  amat c = init_amat(NULL, dest.m, dest.n);
  if (!c.data) {
    return -1;
  }
  amat_copy(a, c);

  float temp = 0.0;
  for (int i = 0; i < dest.n; i++) {
    temp = AMAT_GET(c, i, u);
    AMAT_GET(c, i, u) = AMAT_GET(c, i, v);
    AMAT_GET(c, i, v) = temp;
  }

  memcpy(dest.data, c.data, sizeof(float) * dest.m * dest.n);
  free_amat(c);
  return 0;
}

/*
  Swaps two columns in an m x n matrix
  Arguments:
  - amat a: Matrix a of size m x n
  - int u: Zero indexed column to swap with column v
  - int v: Zero indexed column to swap with column u
  Returns:
  -1 if input is invalid, 0 if successful
*/
int amat_swap_col(amat a, amat dest, int u, int v) {
  if (!dest.data || a.m != dest.m || a.n != dest.n || u < 0 || u >= a.n ||
      v < 0 || v >= a.n) {
    return -1;
  }

  amat c = init_amat(NULL, dest.m, dest.n);
  if (!c.data) {
    return -1;
  }
  amat_copy(a, c);

  float temp = 0.0;
  for (int i = 0; i < dest.m; i++) {
    temp = AMAT_GET(c, u, i);
    AMAT_GET(c, u, i) = AMAT_GET(c, v, i);
    AMAT_GET(c, v, i) = temp;
  }

  memcpy(dest.data, c.data, sizeof(float) * dest.m * dest.n);
  free_amat(c);
  return 0;
}

/*
  Copys an m x n sub matrix of a, where the sub matrix's (0, 0) element is
  (u, v) in a
  Arguments:
  - amat a: Matrix to copy from
  - amat dest: Matrix to paste sub matrix. Can be a if a and dest are same
               size.
  - int u: Row in a corresponding to the value of a which will occupy the
           (0, 0) spot in dest
  - int v: Column in a corresponding to the value of a which will occupy the
           (0, 0) spot in dest
  Returns:
  -1 if input invalid, 0 if successful
*/
int amat_pick(amat a, amat dest, int u, int v) {
  if (!dest.data || u + dest.m > a.m || v + dest.n > a.n) {
    return -1;
  }

  amat c = init_amat(NULL, dest.m, dest.n);
  if (!c.data) {
    return -1;
  }

  for (int i = 0; i < c.n; i++) {
    for (int j = 0; j < c.m; j++) {
      AMAT_GET(c, i, j) = AMAT_GET(a, i + v, j + u);
    }
  }

  memcpy(dest.data, c.data, sizeof(float) * dest.m * dest.n);
  free_amat(c);
  return 0;
}

/*
  Pastes an m x n matrix into dest, beginning at (u, v) in dest
  Arguments:
  - amat a: Matrix to paste
  - amat dest: Matrix to paste sub matrix. Can be a if a and dest are same
               size.
  - int u: Row in dest corresponding to where the (0, 0) element of a will be
           pasted
  - int v: Column in dest corresponding to the (0, 0) element of a will be
           pasted
  Returns:
  -1 if input is invalid, 0 if successful
*/
int amat_ins(amat a, amat dest, int u, int v) {
  if (!dest.data || u + a.m > dest.m || v + a.n > dest.n) {
    return -1;
  }

  amat c = init_amat(NULL, a.m, a.n);
  if (!c.data) {
    return -1;
  }
  amat_copy(a, c);

  for (int i = 0; i < c.n; i++) {
    for (int j = 0; j < c.m; j++) {
      AMAT_GET(dest, i + v, j + u) = AMAT_GET(c, i, j);
    }
  }

  free_amat(c);
  return 0;
}

// Print an amat
void print_amat(amat a) {
  for (int i = 0; i < a.m; i++) {
    for (int j = 0; j < a.n; j++) {
      fprintf(stderr, "%.4f ", AMAT_GET(a, j, i));
    }
    fprintf(stderr, "\n");
  }
}

// Print a formatted amat that can be easily copy-pasted into wolfram-alpha
void print_amat2(amat a) {
  fprintf(stderr,"{");
  for (int i = 0; i < a.m; i++) {
    fprintf(stderr,"{");
    for (int j = 0; j < a.n; j++) {
      fprintf(stderr, "%.4f", AMAT_GET(a, j, i));
      if (j < a.n-1) {
        fprintf(stderr,",");
      }
    }
    if (i < a.m-1) {
      fprintf(stderr, "},\n");
    } else {
      fprintf(stderr, "}");
    }
  }
  fprintf(stderr,"}\n");
}
