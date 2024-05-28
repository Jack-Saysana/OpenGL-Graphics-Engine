#ifndef __AMAT_STR_H__
#define __AMAT_STR_H__

typedef struct arbitrary_matrix {
  // Matrix data (COLUMN MAJOR [col][row])
  float *data;
  // Number of rows
  int m;
  // Number of columns
  int n;
} amat;

// ================================ FUNCTIONS ================================

amat init_amat(float *data, int m, int n);
void free_amat(amat a);
// Matrix addition
int amat_add(amat a, amat b, amat dest);
// Matrix subtraction
int amat_sub(amat a, amat b, amat dest);
// Matrix multiplication
int amat_mul(amat a, amat b, amat dest);
// Matrix transpose
int amat_transpose(amat a, amat dest);
// Vector inner/dot product: https://en.wikipedia.org/wiki/Dot_product
int amat_dot(amat a, amat b, float *dest);
// Vector outer product: https://en.wikipedia.org/wiki/Outer_product
int amat_outer(amat a, amat b, amat dest);

// ================================== MACROS =================================

#define AMAT_GET(a, i, j) (a.data[(i*a.n)+j])

#endif
