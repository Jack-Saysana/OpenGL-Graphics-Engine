#include <math/amat.h>

/*
  ================================== AMAT.H ==================================

  Provides the functionality for working with arbitrarily sized matrices

  ============================================================================
*/

/*
  Initilizes an m x n matrix
  Arguments:
  - float *data: Initial content of new matrix. NULL if no initialization
    should be done.
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

  amat col = { NULL, dest.n, 1 };
  amat row = { NULL, dest.n, 1 };
  for (int i = 0; i < dest.n; i++) {
    for (int j = 0; j < dest.m; j++) {
      col.data = &AMAT_GET(b, i, 0);
      row.data = &AMAT_GET(a, 0, j);
      amat_dot(col, row, &AMAT_GET(c, i, j));
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
      AMAT_GET(dest, i, j) = AMAT_GET(a, j, i);
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

  for (int i = 0; i < dest.n; i++) {
    for (int j = 0; j < dest.m; j++) {
      AMAT_GET(dest, i, j) = AMAT_GET(a, 0, j) * AMAT_GET(b, 0, i);
    }
  }

  return 0;
}
