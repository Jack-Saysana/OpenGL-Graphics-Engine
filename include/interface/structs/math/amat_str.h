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

#endif
