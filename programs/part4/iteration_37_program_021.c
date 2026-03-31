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
    float buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 0.5f;

    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;

    /* Variable for computed length */
    int computed_len = N / 2;

    int sum = 0;
    double checksum = 0.0;

    /* REQUIREMENT 1: OpenMP target data mapping with array sections */
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
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
            arr1D[i] += 2;
        }
    }

    /* REQUIREMENT 2: OpenMP target enter/exit data with array sections */
    /* Multi-dimensional array section */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])

    /* Perform computation on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] *= 2.0;
        }
    }

    #pragma omp target exit data map(from: matrix[5:10][0:COLS])

    /* REQUIREMENT 3: OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        /* Task with depend clause on array section */
        #pragma omp task depend(inout: buffer[0:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++) {
                buffer[i] += 1.0f;
            }
        }

        /* Another task with different section */
        #pragma omp task depend(inout: buffer[CHUNK_SIZE:CHUNK_SIZE])
        {
            for (int i = CHUNK_SIZE; i < 2*CHUNK_SIZE; i++) {
                buffer[i] *= 2.0f;
            }
        }

        #pragma omp taskwait
    }

    /* REQUIREMENT 4: Multi-dimensional array sections */
    /* Full 2D array section */
    #pragma omp target data map(tofrom: matrix[0:ROWS][0:COLS])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                matrix[i][j] += 1.0;
            }
        }
    }

    /* REQUIREMENT 5: Array sections with complex base expressions */
    /* Using pointer arithmetic in base expression - triggers op_prio checks */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        /* The parentheses around (ptr+offset) are important for syntax */
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 3;
        }
    }

    /* Array section with computed length */
    #pragma omp target data map(tofrom: arr1D[N/4:computed_len])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = N/4; i < N/4 + computed_len; i++) {
            arr1D[i] *= 2;
        }
    }

    /* Multi-dimensional with computed bounds */
    int start_row = 20, num_rows = 5;
    int start_col = 10, num_cols = 15;
    #pragma omp target data map(tofrom: \
        matrix[start_row:num_rows][start_col:num_cols])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = start_row; i < start_row + num_rows; i++) {
            for (int j = start_col; j < start_col + num_cols; j++) {
                matrix[i][j] -= 0.5;
            }
        }
    }

    /* Compute checksums for observable behavior (prevents dead code elimination) */
    for (int i = 0; i < N; i++) sum += arr1D[i];
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            checksum += matrix[i][j];
        }
    }

    printf("Array checksum: %d\n", sum);
    printf("Matrix checksum: %.2f\n", checksum);
    printf("Buffer[0]=%.2f, Buffer[%d]=%.2f\n", 
           buffer[0], CHUNK_SIZE, buffer[CHUNK_SIZE]);

    return 0;
}

#ifdef __cplusplus
}
#endif
