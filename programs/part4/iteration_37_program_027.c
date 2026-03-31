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
            matrix[i][j] = i * 1.0 + j * 0.01;

    /* Another buffer for task depend clauses */
    int buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 2;

    /* Pointer for complex base expression */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    /* Variable for computed length */
    int computed_len = N / 2;

    /* --- OpenMP target data with array sections --- */
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
            arr1D[i] += 2;
        }
    }

    /* --- Multi-dimensional array section --- */
    /* Map a slice of a 2D array */
    #pragma omp target data map(to: matrix[5:10][0:COLS])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 5; i < 15; i++)
            for (int j = 0; j < COLS; j++)
                matrix[i][j] *= 2.0;
    }

    /* --- Complex base expression with parentheses needed --- */
    /* This should trigger op_prio checks in the pretty-printer */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 3;
        }
    }

    /* --- OpenMP target enter/exit data with array sections --- */
    int start_row = 20;
    int slice_rows = 15;
    #pragma omp target enter data map(to: matrix[start_row:slice_rows][0:COLS])
    /* ... some computation could happen here ... */
    #pragma omp target exit data map(from: matrix[start_row:slice_rows][0:COLS])

    /* --- OpenMP task depend with array sections --- */
    int start = 40;
    #pragma omp parallel
    #pragma omp single
    {
        /* Task with depend clause on array section */
        #pragma omp task depend(inout: buffer[start:CHUNK_SIZE])
        {
            for (int i = start; i < start + CHUNK_SIZE; i++) {
                buffer[i] *= 3;
            }
        }

        /* Another task depending on a different section */
        #pragma omp task depend(inout: buffer[start+CHUNK_SIZE:CHUNK_SIZE])
        {
            for (int i = start + CHUNK_SIZE; i < start + 2*CHUNK_SIZE; i++) {
                buffer[i] += 5;
            }
        }

        #pragma omp taskwait
    }

    /* --- Array section with computed length --- */
    #pragma omp target data map(tofrom: arr1D[N/4:computed_len])
    {
        #pragma omp target teams distribute parallel for
        for (int i = N/4; i < N/4 + computed_len; i++) {
            arr1D[i] -= 1;
        }
    }

    /* --- Print checksums to prevent dead code elimination --- */
    int sum1 = 0;
    double sum2 = 0.0;
    int sum3 = 0;
    for (int i = 0; i < N; i++) sum1 += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2 += matrix[i][j];
    for (int i = 0; i < N*2; i++) sum3 += buffer[i];

    printf("Checksums: arr1D=%d, matrix=%.2f, buffer=%d\n", sum1, sum2, sum3);
    printf("Sample values: arr1D[0]=%d, matrix[5][5]=%.2f, buffer[40]=%d\n",
           arr1D[0], matrix[5][5], buffer[40]);

    return 0;
}

#ifdef __cplusplus
}
#endif
