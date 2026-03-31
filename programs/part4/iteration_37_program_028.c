/* This file is designed to be compiled as both C and C++ with OpenMP 4.0+ support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using array sections in various OpenMP constructs. */

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
    
    int start = 5;
    int chunk = CHUNK_SIZE;
    
    printf("Initial checksum - arr1D[0]=%d, matrix[0][0]=%.2f, buffer[0]=%.2f\n",
           arr1D[0], matrix[0][0], buffer[0]);
    
    /* ======================================================================
       REQUIREMENT 1: OpenMP Target Data Mapping with Array Sections
       ====================================================================== */
    
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;  /* Trivial computation to keep region alive */
        }
    }
    
    /* Subsection with non-zero lower bound and computed length */
    #pragma omp target data map(tofrom: arr1D[10:20])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 30; i++) {
            arr1D[i] *= 2;
        }
    }
    
    /* ======================================================================
       REQUIREMENT 4: Multi-dimensional Array Sections
       ====================================================================== */
    #pragma omp target data map(tofrom: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 20; j++)
                matrix[i][j] += 1.0;
    }
    
    /* ======================================================================
       REQUIREMENT 5: Array Sections with Complex Base Expressions
       ====================================================================== */
    
    /* Using pointer arithmetic in base expression - will test op_prio logic */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        /* Access through pointer */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 3;
        }
    }
    
    /* Another complex base: subscripted access with variable index */
    int idx = 2;
    #pragma omp target data map(tofrom: matrix[idx][0:COLS])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < COLS; j++) {
            matrix[idx][j] *= 2.0;
        }
    }
    
    /* ======================================================================
       REQUIREMENT 2: OpenMP Target Enter/Exit Data with Array Sections
       ====================================================================== */
    #pragma omp target enter data map(to: matrix[5:rows-10][0:cols])
    
    /* Do some work on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < ROWS - 5; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] -= 0.5;
    
    #pragma omp target exit data map(from: matrix[5:rows-10][0:cols])
    
    /* ======================================================================
       REQUIREMENT 3: OpenMP Task Depend with Array Sections
       ====================================================================== */
    
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writes to buffer section */
        #pragma omp task depend(out: buffer[start:chunk])
        {
            for (int i = start; i < start + chunk; i++) {
                buffer[i] = i * 2.0f;
            }
        }
        
        /* Second task reads from that section (inout dependency) */
        #pragma omp task depend(inout: buffer[start:chunk])
        {
            float sum = 0.0f;
            for (int i = start; i < start + chunk; i++) {
                sum += buffer[i];
                buffer[i] += 1.0f;
            }
            /* Prevent dead code elimination */
            buffer[start] = sum / chunk;
        }
        
        /* Another task with different section */
        #pragma omp task depend(inout: buffer[50:25])
        {
            for (int i = 50; i < 75; i++) {
                buffer[i] = buffer[i] * 0.5f;
            }
        }
    }
    
    /* ======================================================================
       Final verification and output to prevent optimization
       ====================================================================== */
    int sum_arr = 0;
    double sum_mat = 0.0;
    float sum_buf = 0.0f;
    
    for (int i = 0; i < N; i++) sum_arr += arr1D[i];
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++)
            sum_mat += matrix[i][j];
    for (int i = 0; i < 10; i++) sum_buf += buffer[i];
    
    printf("Final checksums - arr1D: %d, matrix: %.2f, buffer: %.2f\n",
           sum_arr, sum_mat, sum_buf);
    
    /* Return something based on results to ensure all code paths matter */
    return (sum_arr > 0 && sum_mat > 0.0) ? 0 : 1;
}

#ifdef __cplusplus
}
#endif
