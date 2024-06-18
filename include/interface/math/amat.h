#ifndef __AMAT_H__
#define __AMAT_H__

#include "../structs/math/amat_str.h"

// ================================ FUNCTIONS ================================

amat init_amat(float *data, int m, int n);
void free_amat(amat a);
// Matrix copy
int amat_copy(amat a, amat dest);
// Make zero matrix
void amat_zero(amat a);
// Make identity matrix
void amat_identity(amat a);
// Scale matrix by scalar
int amat_scale(amat a, amat dest, float s);
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
// Swaps rows of matrix
int amat_swap_row(amat a, amat dest, int u, int v);
// Swaps columns of matrix
int amat_swap_col(amat a, amat dest, int v, int u);
// Copy sub matrix
int amat_pick(amat a, amat dest, int m, int n);
// Paste sub matrix
int amat_ins(amat a, amat dest, int m, int n);
// Print matrix
void print_amat(amat a);
void print_amat2(amat a);

// ================================== MACROS =================================

#define AMAT_GET(a, i, j) ((a).data[((i)*(a).m)+(j)])

#endif
