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
    for (int i = 0; i < N * 2; i++) buffer[i] = i % 10;

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

    /* Subsection with non-zero lower bound and computed length */
    int start = 10, length = 20;
    #pragma omp target data map(tofrom: arr1D[start:length])
    {
        #pragma omp target teams distribute parallel for
        for (int i = start; i < start + length; i++) {
            arr1D[i] *= 2;
        }
    }

    /* 2. OpenMP target enter/exit data with multi-dimensional array sections */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])

    /* Perform computation on the device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] += 100.0;
        }
    }

    #pragma omp target exit data map(from: matrix[5:10][0:COLS])

    /* 3. OpenMP task depend with array sections */
    int task_start = 40;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(inout: buffer[task_start:CHUNK_SIZE])
        {
            for (int i = task_start; i < task_start + CHUNK_SIZE; i++) {
                buffer[i] = buffer[i] * 3 + 1;
            }
        }

        /* Another task with dependency on a different section */
        #pragma omp task depend(inout: buffer[task_start+CHUNK_SIZE:CHUNK_SIZE])
        {
            for (int i = task_start + CHUNK_SIZE; i < task_start + 2*CHUNK_SIZE; i++) {
                buffer[i] = buffer[i] / 2;
            }
        }
    }

    /* 4. Multi-dimensional array sections in target data */
    #pragma omp target data map(tofrom: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                matrix[i][j] -= 50.0;
            }
        }
    }

    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic in base expression */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] = ptr[offset + i] + 5;
        }
    }

    /* Using subscripted access as base (arr1D[i] is not valid for array section,
       but arr1D[0:N] is valid. Let's use a 2D subsection with non-zero lower bounds) */
    int row_start = 20, col_start = 5;
    #pragma omp target data map(tofrom: matrix[row_start:5][col_start:15])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = row_start; i < row_start + 5; i++) {
            for (int j = col_start; j < col_start + 15; j++) {
                matrix[i][j] = matrix[i][j] * 2.0;
            }
        }
    }

    /* Print checksums to ensure data is live and computations occurred */
    int sum1 = 0;
    double sum2 = 0.0;
    int sum3 = 0;

    for (int i = 0; i < N; i++) sum1 += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2 += matrix[i][j];
    for (int i = 0; i < N*2; i++) sum3 += buffer[i];

    printf("Checksums: arr1D=%d, matrix=%.2f, buffer=%d\n", sum1, sum2, sum3);
    printf("Sample values: arr1D[0]=%d, matrix[5][0]=%.2f, buffer[40]=%d\n",
           arr1D[0], matrix[5][0], buffer[40]);

    return 0;
}

#ifdef __cplusplus
}
#endif
