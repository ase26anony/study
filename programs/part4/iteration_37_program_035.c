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
#define CHUNK_SIZE 25

int main(void) {
    /* 1D array for basic array sections */
    int arr1D[N];
    for (int i = 0; i < N; i++) arr1D[i] = i;

    /* 2D array for multi-dimensional sections */
    double matrix[ROWS][COLS];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] = i * 100.0 + j;

    /* Another buffer for task depend clauses */
    float buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 1.5f;

    /* Pointer for complex base expression */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    /* 1. OpenMP target data mapping with simple and non-zero lower bound sections */
    #pragma omp target data map(tofrom: arr1D[0:N])  /* Whole array section */
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < N; i++)
            arr1D[i] += 1;
    }

    /* Subsection with non-zero lower bound and computed length */
    int start = 10, length = 20;
    #pragma omp target data map(tofrom: arr1D[start:length])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = start; i < start + length; i++)
            arr1D[i] *= 2;
    }

    /* 2. Multi-dimensional array section */
    #pragma omp target data map(to: matrix[5:10][0:COLS])  /* Slice of rows */
    {
        /* Dummy target region */
        #pragma omp target
        {
            volatile double dummy = matrix[5][0] + matrix[14][COLS-1];
            (void)dummy;
        }
    }

    /* 3. Array section with complex base expression (pointer arithmetic) */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target
        {
            volatile int dummy = ptr[offset];
            (void)dummy;
        }
    }

    /* 4. OpenMP target enter/exit data with array sections */
    #pragma omp target enter data map(to: matrix[20:5][10:20])

    /* ... some computation could happen here ... */

    #pragma omp target exit data map(from: matrix[20:5][10:20])

    /* 5. OpenMP task depend with array sections */
    int task_start = 40;
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writes to a section */
        #pragma omp task depend(out: buffer[task_start:CHUNK_SIZE])
        {
            for (int i = task_start; i < task_start + CHUNK_SIZE; i++)
                buffer[i] = -buffer[i];
        }

        /* Second task reads the same section (inout dependence) */
        #pragma omp task depend(inout: buffer[task_start:CHUNK_SIZE])
        {
            float sum = 0.0f;
            for (int i = task_start; i < task_start + CHUNK_SIZE; i++)
                sum += buffer[i];
            /* Use sum to prevent dead code elimination */
            buffer[0] = sum / CHUNK_SIZE;
        }
    }

    /* 6. Another complex base: array element as base (arr1D[i]) - 
         This may not directly map to OMP_ARRAY_SECTION in standard OpenMP,
         but we'll use a pointer derived from array element */
    int idx = 5;
    int *p = &arr1D[idx];
    #pragma omp target data map(to: p[0:10])
    {
        #pragma omp target
        {
            volatile int dummy = p[0];
            (void)dummy;
        }
    }

    /* Print checksums to ensure data is live and computations happen */
    int sum1D = 0;
    double sum2D = 0.0;
    float sumBuffer = 0.0f;

    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    for (int i = 0; i < N * 2; i++) sumBuffer += buffer[i];

    printf("Checksums: 1D array = %d, 2D array = %.2f, buffer = %.2f\n",
           sum1D, sum2D, sumBuffer);

    return 0;
}

#ifdef __cplusplus
}
#endif
