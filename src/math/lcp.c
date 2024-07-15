#include <math/lcp.h>

/*
  // TODO Populate
  Arguments:
  - amat M:
  - amat q:
  - amat z:
  - int *ops:
*/
void lcp(amat M, amat q, amat z, int *ops) {
  for (int i = 0; i < z.m; i++) {
    AMAT_GET(z, 0, i) = 1.0;
  }
  amat prev = init_amat(NULL, z.m, 1);
  amat_copy(z, prev);
  int cont = 1;
  float temp = 0.0;
  for (int k = 0; k < MAX_LCP_ITERATIONS && cont; k++) {
    for (int i = 0; i < z.m; i++) {
      temp = AMAT_GET(q, 0, i);
      if (ops[i] == LCP_EQ) {
        // Bilateral constraint
        // TODO fix convergance issue
        for (int j = 0; j < i; j++) {
          temp += (AMAT_GET(M, j, i) * AMAT_GET(z, 0, j));
        }
        for (int j = i+1; j < z.m; j++) {
          temp += (AMAT_GET(M, j, i) * AMAT_GET(z, 0, j));
        }
        if (fabs(AMAT_GET(M, i, i)) <= ZERO_THRESHOLD) {
          temp = 0.0;
        } else {
          temp = (0.5 * AMAT_GET(z, 0, i)) -
                 ((0.5 * temp) / AMAT_GET(M, i, i));
        }
        AMAT_GET(z, 0, i) = temp;
      } else if (ops[i] == LCP_GEQ) {
        // Unitlateral constraint
        for (int j = 0; j < z.m; j++) {
          temp += (AMAT_GET(M, j, i) * AMAT_GET(z, 0, j));
        }
        if (fabs(AMAT_GET(M, i, i)) <= ZERO_THRESHOLD) {
          temp = 0.0;
        } else {
          temp = AMAT_GET(z, 0, i) - (temp / AMAT_GET(M, i, i));
        }

        AMAT_GET(z, 0, i) = fmax(0.0, temp);
      }
    }

    cont = 0;
    for (int i = 0; i < z.m; i++) {
      if (fabs(AMAT_GET(z, 0, i) - AMAT_GET(prev, 0, i)) > ZERO_THRESHOLD) {
        cont = 1;
        break;
      }
    }
    amat_copy(z, prev);
    //if (cont == 0) {
    //  fprintf(stderr, "%d\n", k + 1);
    //}
  }
  //if (cont) {
  //  fprintf(stderr, "%d\n", MAX_LCP_ITERATIONS);
  //}
  free_amat(prev);
}
// TODO Delete
/*
void lcp(amat M, amat q, amat z) {
  int *C = malloc(sizeof(int) * z.m);
  int c_len = 0;
  int *NC = malloc(sizeof(int) * z.m);
  int nc_len = 0;

  amat_zero(z);
  amat a = init_amat(NULL, z.m, 1);
  amat A = init_amat(NULL, M.m, M.n);
  amat delta_f = init_amat(NULL, z.m, 1);
  amat v = init_amat(NULL, z.m, 1);
  float s = 0.0;
  float temp = 0.0;
  int to_pivot = -1;
  int pivot = 0;
  for (int i = 0; i < z.m; i++) {
    amat_mul(M, z, a);
    amat_add(a, q, a);
    while (AMAT_GET(a, 0, i) < -ZERO_THRESHOLD) {
      s = -1.0;
      if (c_len) {
        amat_zero(delta_f);
        delta_f.m = c_len;
        v.m = c_len;
        for (int j = 0; j < c_len; j++) {
          AMAT_GET(v, 0, j) = -AMAT_GET(M, i, C[j]);
        }

        A.m = c_len;
        A.n = c_len;
        for (int j = 0; j < c_len; j++) {
          for (int k = 0; k < c_len; k++) {
            AMAT_GET(A, j, k) = AMAT_GET(M, C[j], C[k]);
          }
        }

        solve_system(A, v, delta_f);
        amat_copy(delta_f, v);
      }

      delta_f.m = z.m;
      amat_zero(delta_f);
      for (int j = 0; j < c_len; j++) {
        AMAT_GET(delta_f, 0, C[j]) = AMAT_GET(v, 0, j);
      }
      AMAT_GET(delta_f, 0, i) = 1.0;
      v.m = z.m;
      amat_mul(M, delta_f, v);

      s = -1.0;
      to_pivot = -1;
      if (AMAT_GET(v, 0, i) > ZERO_THRESHOLD) {
        s = -AMAT_GET(a, 0, i) / AMAT_GET(v, 0, i);
      }
      for (int j = 0; j < c_len; j++) {
        if (AMAT_GET(delta_f, 0, C[j]) < -ZERO_THRESHOLD) {
          temp = -AMAT_GET(z, 0, C[j]) / AMAT_GET(delta_f, 0, C[j]);
          if (s == -1.0 || temp < s) {
            s = temp;
            pivot = j;
            to_pivot = LCP_C;
          }
        }
      }
      for (int j = 0; j < nc_len; j++) {
        if (AMAT_GET(v, 0, j) < -ZERO_THRESHOLD) {
          temp = -AMAT_GET(a, 0, NC[j]) / AMAT_GET(v, 0, NC[j]);
          if (s == -1.0 || temp < s) {
            s = temp;
            pivot = j;
            to_pivot = LCP_NC;
          }
        }
      }

      if (s != -1.0) {
        amat_scale(delta_f, delta_f, s);
        amat_add(delta_f, z, z);
        amat_mul(M, z, a);
        amat_add(a, q, a);
      }

      if (to_pivot == LCP_C) {
        c_len--;
        temp = C[pivot];
        C[pivot] = C[c_len];
        NC[nc_len] = temp;
        nc_len++;
      } else if (to_pivot == LCP_NC) {
        nc_len--;
        temp = NC[pivot];
        NC[pivot] = NC[nc_len];
        C[c_len] = temp;
        c_len++;
      } else {
        C[c_len] = i;
        c_len++;
      }
    }
    if (AMAT_GET(a, 0, i) > ZERO_THRESHOLD &&
        fabs(AMAT_GET(z, 0, i)) <= ZERO_THRESHOLD) {
      NC[nc_len] = i;
      nc_len++;
    }
  }
}
*/
