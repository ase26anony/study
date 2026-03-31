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
    for (int i = 0; i < N * 2; i++) buffer[i] = i % 10;

    /* Pointer for complex base expression tests */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    /* Variable-length computations for dynamic array sections */
    int start = 5;
    int length = 15;
    int rows_section = 10;
    int cols_section = 20;

    /* 1. OpenMP target data mapping with simple and non-zero lower bound array sections */
    #pragma omp target data map(tofrom: arr1D[0:N])  /* Whole array */
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }

    /* Subsection with non-zero lower bound */
    #pragma omp target data map(tofrom: arr1D[10:20])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = 10; i < 30; i++) {
            arr1D[i] *= 2;
        }
    }

    /* 2. OpenMP target enter/exit data with multi-dimensional array sections */
    #pragma omp target enter data map(to: matrix[5:rows_section][0:cols_section])

    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 5 + rows_section; i++) {
        for (int j = 0; j < cols_section; j++) {
            matrix[i][j] += 1.5;
        }
    }

    #pragma omp target exit data map(from: matrix[5:rows_section][0:cols_section])

    /* 3. OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writes to buffer section */
        #pragma omp task depend(out: buffer[0:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++) {
                buffer[i] = i * 2;
            }
        }

        /* Second task reads from first section and writes to second */
        #pragma omp task depend(in: buffer[0:CHUNK_SIZE]) depend(out: buffer[CHUNK_SIZE:CHUNK_SIZE])
        {
            for (int i = CHUNK_SIZE; i < 2 * CHUNK_SIZE; i++) {
                buffer[i] = buffer[i - CHUNK_SIZE] + 1;
            }
        }

        /* Third task with inout depend on combined section */
        #pragma omp task depend(inout: buffer[0:2*CHUNK_SIZE])
        {
            int sum = 0;
            for (int i = 0; i < 2 * CHUNK_SIZE; i++) {
                sum += buffer[i];
            }
            /* Use sum to prevent dead code elimination */
            buffer[0] = sum % 100;
        }
    }

    /* 4. Multi-dimensional array sections in target data */
    #pragma omp target data map(tofrom: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                matrix[i][j] -= 0.5;
            }
        }
    }

    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic as base */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 3;
        }
    }

    /* Using subscripted access as base (arr1D[i] is not valid array section base in standard OpenMP,
       but we can use a more complex expression with a computed base pointer) */
    int *base_ptr = &arr1D[start];
    #pragma omp target data map(tofrom: base_ptr[0:length])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < length; i++) {
            base_ptr[i] -= 1;
        }
    }

    /* Print results to prevent optimization and verify execution */
    printf("arr1D[0] = %d, arr1D[10] = %d, arr1D[N-1] = %d\n", 
           arr1D[0], arr1D[10], arr1D[N-1]);
    printf("matrix[5][10] = %.2f, matrix[9][19] = %.2f\n", 
           matrix[5][10], matrix[9][19]);
    printf("buffer[0] = %d, buffer[CHUNK_SIZE] = %d\n", 
           buffer[0], buffer[CHUNK_SIZE]);

    return 0;
}

#ifdef __cplusplus
}
#endif
