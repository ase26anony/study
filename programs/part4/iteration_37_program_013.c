/* This file is designed to be compiled as both C and C++ with OpenMP support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using OpenMP 4.0+ array section syntax in various clauses. */

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
    #pragma omp target data map(tofrom: arr1D[0:N])  /* Simple whole array */
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }

    /* Subsection with non-zero lower bound */
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
    for (int i = 5; i < 15; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] += 1.5;

    #pragma omp target exit data map(from: matrix[5:10][0:COLS])

    /* 3. OpenMP task depend with array sections */
    int start = 25;
    int chunk_size = CHUNK;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(inout: buffer[start:chunk_size])
        {
            for (int i = start; i < start + chunk_size; i++) {
                buffer[i] += 100;
            }
        }

        /* Another task with different section */
        #pragma omp task depend(inout: buffer[0:start])
        {
            for (int i = 0; i < start; i++) {
                buffer[i] -= 50;
            }
        }
    }

    /* 4. Multi-dimensional array section in target data */
    #pragma omp target data map(tofrom: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 20; j++)
                matrix[i][j] *= 0.5;
    }

    /* 5. Array section with complex base expression */
    /* Using pointer arithmetic in the base expression */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 5;
        }
    }

    /* Another complex base: subscripted access */
    int idx = 2;
    #pragma omp target data map(tofrom: matrix[idx][0:COLS])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < COLS; j++) {
            matrix[idx][j] += 3.14;
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
    for (int i = 0; i < N * 2; i++) sum3 += buffer[i];

    printf("Checksums: arr1D=%d, matrix=%.2f, buffer=%d\n", sum1, sum2, sum3);
    printf("Sample values: arr1D[0]=%d, matrix[5][5]=%.2f, buffer[25]=%d\n",
           arr1D[0], matrix[5][5], buffer[25]);

    return 0;
}

#ifdef __cplusplus
}
#endif
