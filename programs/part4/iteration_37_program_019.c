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
#define CHUNK_SIZE 20

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

    /* Pointer for complex base expression tests */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    /* 1. OpenMP target data mapping with simple and non-zero lower bound array sections */
    #pragma omp target data map(tofrom: arr1D[0:N])  /* Simple whole array */
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }

    #pragma omp target data map(tofrom: arr1D[10:20])  /* Subsection with non-zero lower bound */
    {
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 30; i++) {
            arr1D[i] += 2;
        }
    }

    /* 2. OpenMP target enter/exit data with multi-dimensional array sections */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])  /* Slice of rows */

    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] *= 2.0;
        }
    }

    #pragma omp target exit data map(from: matrix[5:10][0:COLS])

    /* 3. OpenMP task depend with array sections */
    int start = 40;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(inout: buffer[start:CHUNK_SIZE])
        {
            for (int i = start; i < start + CHUNK_SIZE; i++) {
                buffer[i] *= 3;
            }
        }

        #pragma omp task depend(inout: buffer[start:CHUNK_SIZE])
        {
            for (int i = start; i < start + CHUNK_SIZE; i++) {
                buffer[i] += 5;
            }
        }
    }

    /* 4. Multi-dimensional array sections in target data */
    #pragma omp target data map(tofrom: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                matrix[i][j] += 1.5;
            }
        }
    }

    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic in base expression */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] *= 2;
        }
    }

    /* Using subscripted access as base (arr1D[i] is not valid for array section,
       but arr1D[0:N] is; we'll use a 2D slice with computed index) */
    int idx = 2;
    #pragma omp target data map(tofrom: matrix[idx][0:COLS])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < COLS; j++) {
            matrix[idx][j] -= 0.5;
        }
    }

    /* 6. Print checksums to ensure data is live and computations occurred */
    int sum1D = 0;
    double sum2D = 0.0;
    int sumBuffer = 0;

    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    for (int i = start; i < start + CHUNK_SIZE; i++) sumBuffer += buffer[i];

    printf("Checksum arr1D: %d\n", sum1D);
    printf("Checksum matrix: %.2f\n", sum2D);
    printf("Checksum buffer[%d:%d]: %d\n", start, CHUNK_SIZE, sumBuffer);
    printf("Sample values: arr1D[0]=%d, matrix[5][5]=%.2f, buffer[40]=%d\n",
           arr1D[0], matrix[5][5], buffer[40]);

    return 0;
}

#ifdef __cplusplus
}
#endif
