#include <stdlib.h>
#include <stdio.h>

#include "common.h"

// calculates \sum_K v_i'*A*v_i
double dosum(Matrix A, Matrix v)
{
  Matrix v2 = createMatrix(v->rows, v->cols);
  MxM(A, v, v2, 1.0, 0.0);
  double alpha = innerproduct(v->as_vec, v2->as_vec);
  freeMatrix(v2);
#ifdef HAVE_MPI
  double s2=alpha;
  MPI_Allreduce(&s2, &alpha, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
#endif

  return alpha;
}

int main(int argc, char** argv)
{
  int rank, size;
  init_app(argc, argv, &rank, &size);

  if (argc < 3) {
    printf("need two parameters, the matrix size and the number of vectors\n");
    close_app();
    return 1;
  }
  int N=atoi(argv[1]);
  int K=atoi(argv[2]);

  Matrix A = createMatrix(N,N);
  // identity matrix
  for (int i=0;i<N;++i)
    A->data[i][i] = 1.0;
  
  int ofs, cols;
  splitVector(K, rank, size, &cols, &ofs);
  Matrix v = createMatrix(N,cols);
  // fill with column number
  for (int i=0;i<cols;++i)
    for (int j=0;j<N;++j)
      v->data[i][j] = i+ofs;

  double time = WallTime();
  double sum = dosum(A,v);

  if (rank == 0) {
    printf("sum: %f\n", sum);
    printf("elapsed: %f\n", WallTime()-time);
  }

  freeMatrix(v);
  freeMatrix(A);
  close_app();
  return 0;
}
