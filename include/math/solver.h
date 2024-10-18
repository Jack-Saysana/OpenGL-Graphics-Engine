#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <const.h>
#include <structs/math/amat_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

void preprocess_system(amat, amat, amat, amat *, amat *, amat *, int *, int *);
void postprocess_sol(amat, amat, int *);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int lu_solve(amat a, amat b, amat x);
void svd(amat a, amat U, amat S, amat V);
