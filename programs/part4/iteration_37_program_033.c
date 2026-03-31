/* This file is designed to be compiled as both C and C++ with OpenMP support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using OpenMP 4.0+ array section syntax ([lower:length]) in various clauses. */

#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
extern "C" {
#else
#include <stdio.h>
#include <stdlib.h>
#endif

#define N 100
#define ROWS 50
#define COLS 50
#define CHUNK 20

int main(void) {
    /* 1D array for basic array sections */
    int arr1D[N];
    for (int i = 0; i < N; i++) arr1D[i] = i;

    /* 2D array for multi-dimensional sections */
    double matrix[ROWS][COLS];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] = i * 1.0 + j * 0.01;

    /* Another buffer for task depend clauses */
    int buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i % 10;

    /* Pointer for complex base expression */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    /* 1. OpenMP target data with simple and non-zero lower bound array sections */
    #pragma omp target data map(tofrom: arr1D[0:N])  /* whole array */
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }

    /* 2. Target data with subsection and complex base expression */
    /* Using pointer arithmetic as base: (ptr+offset)[0:size] */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        /* Access through pointer to ensure the region is used */
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 2;
        }
    }

    /* 3. Multi-dimensional array section */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])

    /* 4. Task depend with array section */
    int start = 25;
    int chunk_size = CHUNK;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(inout: buffer[start:chunk_size])
        {
            for (int i = start; i < start + chunk_size; i++) {
                buffer[i] *= 2;
            }
        }
        #pragma omp task depend(inout: buffer[start:chunk_size])
        {
            for (int i = start; i < start + chunk_size; i++) {
                buffer[i] += 3;
            }
        }
    }

    /* 5. Another multi-dimensional section with non-zero lower bounds */
    int sub_rows = 8;
    int sub_cols = 12;
    #pragma omp target data map(tofrom: matrix[2:sub_rows][3:sub_cols])
    {
        #pragma omp target teams distribute parallel for simd collapse(2)
        for (int i = 2; i < 2 + sub_rows; i++) {
            for (int j = 3; j < 3 + sub_cols; j++) {
                matrix[i][j] *= 1.5;
            }
        }
    }

    /* 6. Array section with subscripted base: arr1D[i] is not valid as base for
          array section, but we can use a pointer into a 2D slice */
    int (*mat_ptr)[COLS] = &matrix[10];
    #pragma omp target data map(tofrom: mat_ptr[0:5][0:COLS])
    {
        #pragma omp target teams distribute parallel for simd collapse(2)
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < COLS; j++) {
                mat_ptr[i][j] += 0.5;
            }
        }
    }

    /* Print checksums to ensure data is live and computations happen */
    int sum1 = 0;
    double sum2 = 0.0;
    int sum3 = 0;
    for (int i = 0; i < N; i++) sum1 += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2 += matrix[i][j];
    for (int i = 0; i < N*2; i++) sum3 += buffer[i];

    printf("Checksums: %d, %.2f, %d\n", sum1, sum2, sum3);
    /* Print a sample element from each array to prevent dead code elimination */
    printf("Sample: arr1D[%d]=%d, matrix[%d][%d]=%.2f, buffer[%d]=%d\n",
           N/2, arr1D[N/2], ROWS/2, COLS/2, matrix[ROWS/2][COLS/2],
           start, buffer[start]);

    return 0;
}

#ifdef __cplusplus
}
#endif
