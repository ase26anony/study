/* This file is designed to be compiled as both C and C++ with OpenMP 4.0+ support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using OpenMP array section syntax [lower:length] in various contexts. */

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

    /* 1. OpenMP target data mapping with simple and non-zero lower bound array sections */
    #pragma omp target data map(tofrom: arr1D[0:N])  /* Simple whole array section */
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }

    /* Subsection with non-zero lower bound and computed length */
    int start = 10;
    int length = 20;
    #pragma omp target data map(tofrom: arr1D[start:length])
    {
        #pragma omp target teams distribute parallel for
        for (int i = start; i < start + length; i++) {
            arr1D[i] *= 2;
        }
    }

    /* 2. OpenMP target enter/exit data with multi-dimensional array sections */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])  /* Slice of rows */

    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] += 100.0;
        }
    }

    #pragma omp target exit data map(from: matrix[5:10][0:COLS])

    /* 3. OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writes to buffer section */
        #pragma omp task depend(out: buffer[0:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++) {
                buffer[i] = -i;
            }
        }

        /* Second task reads/writes overlapping section with dependency */
        #pragma omp task depend(inout: buffer[CHUNK_SIZE/2:CHUNK_SIZE])
        {
            for (int i = CHUNK_SIZE/2; i < CHUNK_SIZE + CHUNK_SIZE/2; i++) {
                buffer[i] *= 3;
            }
        }

        #pragma omp taskwait
    }

    /* 4. Multi-dimensional array section in target data */
    int grid[20][30];
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 30; j++)
            grid[i][j] = i + j;

    #pragma omp target data map(tofrom: grid[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                grid[i][j] += 5;
            }
        }
    }

    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic in base expression - may trigger op_prio parentheses */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 1000;
        }
    }

    /* Using subscripted access as base */
    int arr2D[10][20];
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            arr2D[i][j] = i * 20 + j;

    #pragma omp target data map(tofrom: arr2D[2][0:15])  /* Row slice */
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < 15; j++) {
            arr2D[2][j] *= 2;
        }
    }

    /* Print checksums to ensure data is live and computations occurred */
    int sum1D = 0;
    double sum2D = 0.0;
    int sumBuffer = 0;
    int sumGrid = 0;

    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    for (int i = 0; i < CHUNK_SIZE * 2; i++) sumBuffer += buffer[i];
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            sumGrid += grid[i][j];

    printf("Checksums:\n");
    printf("arr1D: %d\n", sum1D);
    printf("matrix: %.2f\n", sum2D);
    printf("buffer: %d\n", sumBuffer);
    printf("grid: %d\n", sumGrid);
    printf("Sample elements - arr1D[0]=%d, matrix[5][0]=%.2f, buffer[10]=%d\n",
           arr1D[0], matrix[5][0], buffer[10]);

    return 0;
}

#ifdef __cplusplus
}
#endif
