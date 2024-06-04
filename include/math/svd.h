#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <const.h>
#include <structs/math/amat_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

float calc_phi(float, float, float, float);
void calc_r1(amat, amat);
float super_diag_elem(amat);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int hh_clean_row(amat a, amat dest, int i);
int hh_clean_col(amat a, amat dest, int i);
