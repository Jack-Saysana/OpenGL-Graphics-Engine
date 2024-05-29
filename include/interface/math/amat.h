#ifndef __AMAT_H__
#define __AMAT_H__

#include "../structs/math/amat_str.h"

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

#define AMAT_GET(a, i, j) (a.data[(i*a.m)+j])

#endif
