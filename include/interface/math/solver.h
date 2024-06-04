#ifndef __SOLVER_H__
#define __SOLVER_H__

#include "../structs/math/amat_str.h"

void solve_system(amat a, amat b, amat x);
int solve_upper(amat a, amat b, amat x);
int solve_lower(amat a, amat b, amat x);
int lu_solve(amat a, amat b, amat x);
void svd(amat a, amat U, amat S, amat V);

#endif
