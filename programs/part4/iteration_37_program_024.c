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

    /* Buffer for task depend clauses */
    int buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 2;

    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    /* Variable for computed bounds */
    int start = 5;
    int length = 15;

    /* --- OpenMP target data with array sections --- */
    /* Simple whole array section */
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
    #pragma omp parallel
    #pragma omp single
    {
        /* Task with array section in depend clause */
        #pragma omp task depend(inout: buffer[0:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++) {
                buffer[i] = -buffer[i];
            }
        }

        /* Another task with computed bounds */
        #pragma omp task depend(inout: buffer[start:length])
        {
            for (int i = start; i < start + length; i++) {
                buffer[i] += 777;
            }
        }

        #pragma omp taskwait
    }

    /* --- Array sections with complex base expressions --- */
    /* Using pointer arithmetic as base (triggers op_prio checks) */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 999;
        }
    }

    /* Multi-dimensional with subscripted base */
    int (*arr2D)[COLS] = matrix;
    int row_idx = 20;
    #pragma omp target data map(tofrom: arr2D[row_idx][0:COLS])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < COLS; j++) {
            arr2D[row_idx][j] -= 50;
        }
    }

    /* --- Print results for observable behavior --- */
    int checksum = 0;
    checksum += arr1D[0] + arr1D[N-1];
    checksum += (int)matrix[5][0] + (int)matrix[14][COLS-1];
    checksum += buffer[0] + buffer[CHUNK_SIZE-1] + buffer[start];

    printf("Checksum: %d\n", checksum);
    printf("Sample values - arr1D[0]=%d, matrix[5][0]=%.2f, buffer[0]=%d\n",
           arr1D[0], matrix[5][0], buffer[0]);

    return 0;
}

#ifdef __cplusplus
}
#endif
