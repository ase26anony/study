/* This file is designed to be compiled as both C and C++ with OpenMP support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using OpenMP 4.0+ array section syntax [lower:length] in various clauses. */

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
    float buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 0.5f;

    /* Pointer for complex base expression tests */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    int sum = 0;
    double checksum = 0.0;

    /* REQUIREMENT 1 & 5: OpenMP target data mapping with array sections,
       including complex base expression (pointer arithmetic) */
    #pragma omp target data map(tofrom: arr1D[0:N]) \
                            map(to: (ptr+offset)[0:size]) \
                            map(tofrom: matrix[5:10][0:COLS])
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }

        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 5; i < 15; i++) {
            for (int j = 0; j < COLS; j++) {
                matrix[i][j] *= 2.0;
            }
        }
    }

    /* REQUIREMENT 2: OpenMP target enter/exit data with array sections */
    int subsection[20][30];
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 30; j++)
            subsection[i][j] = i + j;

    #pragma omp target enter data map(to: subsection[0:20][0:30])

    /* Dummy target region using the data */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 30; j++) {
            subsection[i][j] += 1;
        }
    }

    #pragma omp target exit data map(from: subsection[0:20][0:30])

    /* REQUIREMENT 3: OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writes to a section */
        #pragma omp task depend(out: buffer[0:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++) {
                buffer[i] = buffer[i] * 2.0f;
            }
        }

        /* Second task reads that section and writes to another */
        #pragma omp task depend(in: buffer[0:CHUNK_SIZE]) \
                         depend(out: buffer[CHUNK_SIZE:CHUNK_SIZE])
        {
            for (int i = CHUNK_SIZE; i < 2*CHUNK_SIZE; i++) {
                buffer[i] = buffer[i-CHUNK_SIZE] + 1.0f;
            }
        }

        /* Third task depends on the second's output */
        #pragma omp task depend(in: buffer[CHUNK_SIZE:CHUNK_SIZE])
        {
            float local_sum = 0.0f;
            for (int i = CHUNK_SIZE; i < 2*CHUNK_SIZE; i++) {
                local_sum += buffer[i];
            }
            /* Use local_sum to prevent dead code elimination */
            buffer[0] += local_sum;
        }
    }

    /* REQUIREMENT 4: Multi-dimensional array section in target data */
    int grid[10][20];
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            grid[i][j] = i * j;

    #pragma omp target data map(tofrom: grid[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                grid[i][j] += 2;
            }
        }
    }

    /* Final computations to ensure data is live and observable */
    for (int i = 0; i < N; i++) sum += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            checksum += matrix[i][j];

    printf("Checksum values: %d, %.2f, %.2f\n", 
           sum, checksum, (double)buffer[0]);
    printf("Sample grid[5][5] = %d\n", grid[5][5]);
    printf("Sample subsection[10][15] = %d\n", subsection[10][15]);

    return 0;
}

#ifdef __cplusplus
}
#endif
