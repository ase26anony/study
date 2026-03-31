/* This file is designed to be compiled as both C and C++ with OpenMP 4.0+ support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using array section syntax [lower:length] in various OpenMP constructs. */

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

    /* Buffer for task depend clauses */
    int buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i % 10;

    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    int sum = 0;

    /* 1. OpenMP target data mapping with array sections */
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
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

    /* 2. OpenMP target enter/exit data with array sections */
    /* Multi-dimensional array section */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])

    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] += 1.0;

    #pragma omp target exit data map(from: matrix[5:10][0:COLS])

    /* 3. OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        /* Task with array section in depend clause */
        #pragma omp task depend(inout: buffer[0:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++) {
                buffer[i] += 5;
            }
        }

        /* Another task depending on a different section */
        #pragma omp task depend(inout: buffer[CHUNK_SIZE:CHUNK_SIZE])
        {
            for (int i = CHUNK_SIZE; i < 2*CHUNK_SIZE; i++) {
                buffer[i] -= 3;
            }
        }

        #pragma omp taskwait
    }

    /* 4. Multi-dimensional array sections in target data */
    /* Full slice of 2D array */
    #pragma omp target data map(tofrom: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 20; j++)
                matrix[i][j] *= 0.5;
    }

    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic as base */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 100;
        }
    }

    /* Using subscripted access as base (arr1D[i] is not valid as base in standard OpenMP,
       but arr1D[0:N] is valid. Let's use a more realistic complex base through a function) */
    {
        int *get_ptr(void) { return arr1D + 5; }
        /* This creates a more complex tree for the base */
        #pragma omp target data map(tofrom: get_ptr()[0:15])
        {
            #pragma omp target teams distribute parallel for
            for (int i = 0; i < 15; i++) {
                get_ptr()[i] += 50;
            }
        }
    }

    /* Compute checksum to ensure all computations happened */
    for (int i = 0; i < N; i++) sum += arr1D[i];
    
    double dsum = 0.0;
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            dsum += matrix[i][j];
    
    int bsum = 0;
    for (int i = 0; i < N*2; i++) bsum += buffer[i];

    /* Print results to prevent dead code elimination */
    printf("Checksums: arr1D=%d, matrix=%.2f, buffer=%d\n", sum, dsum, bsum);
    printf("Sample values: arr1D[0]=%d, matrix[5][5]=%.2f, buffer[10]=%d\n",
           arr1D[0], matrix[5][5], buffer[10]);

    return 0;
}

#ifdef __cplusplus
}
#endif
