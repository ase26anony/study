/* This file is designed to be compiled as both C and C++ with OpenMP support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using OpenMP 4.0+ array section syntax [lower:length] in various contexts. */

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
#define CHUNK_SIZE 25

int main(void) {
    /* 1D array for basic array sections */
    int arr1D[N];
    for (int i = 0; i < N; i++) arr1D[i] = i;

    /* 2D array for multi-dimensional sections */
    double matrix[ROWS][COLS];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] = i * 1.5 + j * 0.5;

    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    /* Buffer for task depend clauses */
    int buffer[CHUNK_SIZE * 2];
    for (int i = 0; i < CHUNK_SIZE * 2; i++) buffer[i] = 0;

    int start = 5;
    int chunk = CHUNK_SIZE;

    /* 1. OpenMP target data mapping with simple and non-zero lower bound sections */
    #pragma omp target data map(tofrom: arr1D[0:N])  /* Simple whole array */
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }

    /* Subsection with non-zero lower bound and computed length */
    #pragma omp target data map(tofrom: arr1D[10:20])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 30; i++) {
            arr1D[i] *= 2;
        }
    }

    /* 2. OpenMP target enter/exit data with multi-dimensional array sections */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])

    /* Use the mapped section */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] += 1.0;
        }
    }

    #pragma omp target exit data map(from: matrix[5:10][0:COLS])

    /* 3. OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(inout: buffer[start:chunk])
        {
            for (int i = start; i < start + chunk; i++) {
                buffer[i] = i * 3;
            }
        }

        #pragma omp task depend(inout: buffer[start:chunk])
        {
            for (int i = start; i < start + chunk; i++) {
                buffer[i] += 100;
            }
        }
    }

    /* 4. Multi-dimensional array section in target data */
    #pragma omp target data map(to: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                matrix[i][j] -= 0.5;
            }
        }
    }

    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic in the base expression */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] = -ptr[offset + i];
        }
    }

    /* Using subscripted access as base (arr1D[i] is not valid as base for array section,
       but arr1D itself with variable subscript in another dimension would be).
       Instead, we demonstrate with a 2D access: */
    int (*arr2D)[COLS] = matrix;
    int row_idx = 2;
    #pragma omp target data map(to: arr2D[row_idx][0:COLS])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < COLS; j++) {
            arr2D[row_idx][j] = j * 10;
        }
    }

    /* Print checksums to ensure data is live and computations occur */
    int sum1D = 0;
    double sum2D = 0.0;
    int sumBuffer = 0;

    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    for (int i = 0; i < CHUNK_SIZE * 2; i++) sumBuffer += buffer[i];

    printf("Checksums:\n");
    printf("  arr1D: %d\n", sum1D);
    printf("  matrix: %.2f\n", sum2D);
    printf("  buffer: %d\n", sumBuffer);
    printf("Sample values: arr1D[0]=%d, matrix[5][5]=%.2f, buffer[5]=%d\n",
           arr1D[0], matrix[5][5], buffer[5]);

    return 0;
}

#ifdef __cplusplus
}
#endif
