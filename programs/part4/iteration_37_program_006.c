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
    float buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 0.5f;

    /* Pointer for complex base expression tests */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    int sum = 0;

    /* Requirement 1: OpenMP target data mapping with array sections */
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }

    /* Subsection with non-zero lower bound and computed length */
    int start = 10;
    int length = 20;
    #pragma omp target data map(tofrom: arr1D[start:length])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = start; i < start + length; i++) {
            arr1D[i] *= 2;
        }
    }

    /* Requirement 5: Complex base expression with pointer arithmetic */
    /* Note: Some compilers may require the base expression to be lvalue,
       so we use a pointer array section */
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) dynamic_arr[i] = i;

    #pragma omp target data map(tofrom: dynamic_arr[0:N])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < N; i++) {
            dynamic_arr[i] += 3;
        }
    }

    /* Requirement 4: Multi-dimensional array sections */
    /* Map a slice of the 2D matrix */
    #pragma omp target data map(to: matrix[5:10][0:COLS])
    {
        /* Dummy target region to use the mapped data */
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 5; i < 15; i++) {
            for (int j = 0; j < COLS; j++) {
                matrix[i][j] += 1.0;
            }
        }
    }

    /* Requirement 2: OpenMP target enter/exit data with array sections */
    /* Enter data with a 2D array section */
    #pragma omp target enter data map(to: matrix[20:5][10:20])

    /* ... some computation could happen here ... */

    /* Exit data with the same section */
    #pragma omp target exit data map(from: matrix[20:5][10:20])

    /* Requirement 3: OpenMP task depend with array sections */
    /* Use array sections in task depend clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Task producing data in a buffer section */
            #pragma omp task depend(out: buffer[0:CHUNK])
            {
                for (int i = 0; i < CHUNK; i++) {
                    buffer[i] = 100.0f + i;
                }
            }

            /* Task consuming the same buffer section */
            #pragma omp task depend(in: buffer[0:CHUNK])
            {
                float local_sum = 0.0f;
                for (int i = 0; i < CHUNK; i++) {
                    local_sum += buffer[i];
                }
                /* Use the result to prevent optimization */
                buffer[CHUNK] = local_sum;
            }

            /* Another task with non-zero lower bound */
            #pragma omp task depend(inout: buffer[CHUNK:CHUNK])
            {
                for (int i = CHUNK; i < 2*CHUNK; i++) {
                    buffer[i] *= 1.5f;
                }
            }
        }
    }

    /* Compute checksums to ensure data is live and used */
    for (int i = 0; i < N; i++) sum += arr1D[i];
    double dsum = 0.0;
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            dsum += matrix[i][j];
    
    float fsum = 0.0f;
    for (int i = 0; i < N*2; i++) fsum += buffer[i];

    /* Print results to prevent dead code elimination */
    printf("Checksums: int=%d double=%.2f float=%.2f\n", 
           sum, dsum, fsum);
    
    free(dynamic_arr);
    return 0;
}

#ifdef __cplusplus
}
#endif
