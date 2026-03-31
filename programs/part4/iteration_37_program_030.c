/* This file is designed to be compiled as both C and C++ with OpenMP support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using OpenMP 4.0+ array section syntax in various contexts. */

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
            matrix[i][j] = i * 1.0 + j * 0.01;

    /* Another buffer for task depend clauses */
    int buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i % 7;

    /* Pointer for complex base expression tests */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    /* Variable for computed length */
    int computed_len = 20;

    /* --- OpenMP target data mapping with array sections --- */
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
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

    /* --- OpenMP target enter/exit data with array sections --- */
    /* Multi-dimensional array section */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])

    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] += 100.0;
        }
    }

    #pragma omp target exit data map(from: matrix[5:10][0:COLS])

    /* --- OpenMP task depend with array sections --- */
    int start = 40;
    #pragma omp parallel
    #pragma omp single
    {
        /* Task with inout depend on array section */
        #pragma omp task depend(inout: buffer[start:CHUNK_SIZE])
        {
            for (int i = start; i < start + CHUNK_SIZE; i++) {
                buffer[i] += 5;
            }
        }

        /* Another task with out depend on different section */
        #pragma omp task depend(out: buffer[70:30])
        {
            for (int i = 70; i < 100; i++) {
                buffer[i] = 0;
            }
        }

        #pragma omp taskwait
    }

    /* --- Array sections with complex base expressions --- */
    /* Using pointer arithmetic in base expression (triggers op_prio checks) */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] -= 3;
        }
    }

    /* Using subscripted access as base */
    int idx = 2;
    #pragma omp target data map(to: matrix[idx][0:computed_len])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < computed_len; j++) {
            matrix[idx][j] *= 2.0;
        }
    }

    /* --- Print checksums for observable behavior --- */
    int sum1 = 0;
    double sum2 = 0.0;
    int sum3 = 0;

    for (int i = 0; i < N; i++) sum1 += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2 += matrix[i][j];
    for (int i = 0; i < N * 2; i++) sum3 += buffer[i];

    printf("Checksums:\n");
    printf("  arr1D: %d\n", sum1);
    printf("  matrix: %.2f\n", sum2);
    printf("  buffer: %d\n", sum3);

    /* Print sample elements to ensure data is live */
    printf("Sample values:\n");
    printf("  arr1D[0]=%d, arr1D[10]=%d\n", arr1D[0], arr1D[10]);
    printf("  matrix[5][0]=%.2f, matrix[2][10]=%.2f\n", matrix[5][0], matrix[2][10]);
    printf("  buffer[40]=%d, buffer[70]=%d\n", buffer[40], buffer[70]);

    return 0;
}

#ifdef __cplusplus
}
#endif
