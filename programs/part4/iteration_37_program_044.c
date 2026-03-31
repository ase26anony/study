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
            matrix[i][j] = i * 1.0 + j * 0.01;

    /* Another buffer for task depend clauses */
    int buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 2;

    /* Pointer for complex base expression */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    /* Variable for computed length */
    int computed_len = N / 2;

    printf("Starting OpenMP array section tests...\n");

    /* 1. OpenMP target data mapping with array sections */
    /* Simple whole array section */
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
        /* Task with inout depend on array section */
        #pragma omp task depend(inout: buffer[0:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++) {
                buffer[i] += 5;
            }
        }

        /* Another task with different section */
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
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                matrix[i][j] *= 2.0;
            }
        }
    }

    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic in base expression - this triggers op_prio checks */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = offset; i < offset + size; i++) {
            ptr[i] = ptr[i] * 3;
        }
    }

    /* Array section with subscripted base */
    int idx = 2;
    #pragma omp target data map(tofrom: matrix[idx][0:computed_len])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < computed_len; j++) {
            matrix[idx][j] += 50.0;
        }
    }

    /* 6. Mixed complex cases */
    /* Multiple array sections in same clause */
    #pragma omp target data map(tofrom: arr1D[0:50], arr1D[50:50])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += i;
        }
    }

    /* Verify results and prevent dead code elimination */
    int checksum = 0;
    checksum += arr1D[0] + arr1D[N-1];
    checksum += (int)matrix[0][0] + (int)matrix[ROWS-1][COLS-1];
    checksum += buffer[0] + buffer[N-1];
    
    printf("Checksum: %d\n", checksum);
    printf("Sample values - arr1D[0]=%d, matrix[5][5]=%.2f, buffer[10]=%d\n",
           arr1D[0], matrix[5][5], buffer[10]);

    return 0;
}

#ifdef __cplusplus
}
#endif
