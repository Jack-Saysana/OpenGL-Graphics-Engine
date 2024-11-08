#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <const.h>
#include <structs/math/amat_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

float calc_phi(float, float, float, float);
void calc_r1(amat, int offset, int size, amat);
float super_diag_elem(amat);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

void hh_clean_row(amat a, amat dest, int i);
void hh_clean_col(amat a, amat dest, int i);
void giv_clean_row(amat a, amat dest, int i);
void giv_clean_col(amat a, amat dest, int i);
void calc_givens(float f, float g, amat dest);

