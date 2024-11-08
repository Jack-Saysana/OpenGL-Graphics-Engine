#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <const.h>
#include <structs/math/amat_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

void partial_pivot(amat, amat);
void lu_decomp(amat, amat, amat);
void unwrap_pivot(amat, int *);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int solve_upper(amat, amat, amat);
int solve_lower(amat, amat, amat);
