/* This file is designed to be compiled as both C and C++ with OpenMP 4.0+ support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in tree-pretty-print.cc
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
            matrix[i][j] = i * 100.0 + j;

    /* Another buffer for task depend clauses */
    float buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 1.5f;

    /* Pointer for complex base expression tests */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    /* 1. OpenMP target data mapping with array sections */
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

    /* 2. OpenMP target enter/exit data with array sections */
    /* Multi-dimensional array section */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])

    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] += 1.0;
        }
    }

    #pragma omp target exit data map(from: matrix[5:10][0:COLS])

    /* 3. OpenMP task depend with array sections */
    int start = 40;
    #pragma omp parallel
    #pragma omp single
    {
        /* Task with inout depend on array section */
        #pragma omp task depend(inout: buffer[start:CHUNK_SIZE])
        {
            for (int i = start; i < start + CHUNK_SIZE; i++) {
                buffer[i] = buffer[i] * 2.0f;
            }
        }

        /* Another task with out depend on different section */
        #pragma omp task depend(out: buffer[start+CHUNK_SIZE:CHUNK_SIZE])
        {
            for (int i = start + CHUNK_SIZE; i < start + 2*CHUNK_SIZE; i++) {
                buffer[i] = -buffer[i];
            }
        }

        #pragma omp taskwait
    }

    /* 4. Multi-dimensional array sections in target data */
    /* Full slice of 2D array */
    #pragma omp target data map(tofrom: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                matrix[i][j] /= 2.0;
            }
        }
    }

    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic in base expression - tests op_prio parentheses */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 100;
        }
    }

    /* Array section with subscripted base */
    int idx = 2;
    #pragma omp target data map(to: matrix[idx][0:COLS])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < COLS; j++) {
            matrix[idx][j] += 50.0;
        }
    }

    /* 6. Print checksums to ensure data is live and computations occurred */
    int sum1D = 0;
    double sum2D = 0.0;
    float sumBuffer = 0.0f;

    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    for (int i = 0; i < N * 2; i++) sumBuffer += buffer[i];

    printf("Checksums:\n");
    printf("  arr1D: %d\n", sum1D);
    printf("  matrix: %.2f\n", sum2D);
    printf("  buffer: %.2f\n", sumBuffer);

    /* Print sample elements to show data was modified */
    printf("Sample elements after modifications:\n");
    printf("  arr1D[0] = %d, arr1D[10] = %d, arr1D[99] = %d\n",
           arr1D[0], arr1D[10], arr1D[99]);
    printf("  matrix[5][10] = %.2f, matrix[2][25] = %.2f\n",
           matrix[5][10], matrix[2][25]);
    printf("  buffer[40] = %.2f, buffer[65] = %.2f\n",
           buffer[40], buffer[65]);

    return 0;
}

#ifdef __cplusplus
}
#endif
