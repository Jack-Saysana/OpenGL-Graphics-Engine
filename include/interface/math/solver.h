#ifndef __SOLVER_H__
#define __SOLVER_H__

#include "../structs/math/amat_str.h"

int solve_upper(amat a, amat b, amat x);
int solve_lower(amat a, amat b, amat x);
int lu_solve(amat a, amat b, amat x);

#endif
