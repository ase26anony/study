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
            matrix[i][j] = i * 100.0 + j;
    
    /* Buffer for task depend clauses */
    float buffer[N * 2];
    for (int i = 0; i < N*2; i++) buffer[i] = i * 1.5f;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    int sum = 0;
    
    /* 1. OpenMP target data mapping with array sections */
    #pragma omp target data map(tofrom: arr1D[0:N])  /* Simple whole array */
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;  /* Trivial computation to keep region alive */
        }
    }
    
    /* 2. Subsection with non-zero lower bound and computed length */
    int start = 10;
    int length = 20;
    #pragma omp target data map(tofrom: arr1D[start:length])
    {
        #pragma omp target teams distribute parallel for
        for (int i = start; i < start + length; i++) {
            arr1D[i] *= 2;
        }
    }
    
    /* 3. Multi-dimensional array section */
    #pragma omp target data map(to: matrix[5:10][0:COLS])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 5; i < 15; i++)
            for (int j = 0; j < COLS; j++)
                matrix[i][j] += 1.0;
    }
    
    /* 4. Complex base expression with pointer arithmetic */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 3;
        }
    }
    
    /* 5. OpenMP target enter/exit data with array sections */
    #pragma omp target enter data map(to: matrix[20:5][10:20])
    
    /* Do some work on device (simulated) */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 20; i < 25; i++)
        for (int j = 10; j < 30; j++)
            matrix[i][j] *= 2.0;
    
    #pragma omp target exit data map(from: matrix[20:5][10:20])
    
    /* 6. OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(inout: buffer[0:CHUNK_SIZE])
        {
            for (int i = 0; i < CHUNK_SIZE; i++) buffer[i] += 10.0f;
        }
        
        #pragma omp task depend(inout: buffer[CHUNK_SIZE:CHUNK_SIZE])
        {
            for (int i = CHUNK_SIZE; i < 2*CHUNK_SIZE; i++) buffer[i] -= 5.0f;
        }
        
        #pragma omp task depend(in: buffer[0:CHUNK_SIZE]) \
                         depend(in: buffer[CHUNK_SIZE:CHUNK_SIZE])
        {
            float total = 0.0f;
            for (int i = 0; i < 2*CHUNK_SIZE; i++) total += buffer[i];
            printf("Task 3: buffer sum = %f\n", total);
        }
    }
    
    /* 7. Additional complex case: array section of a subscripted expression */
    int idx = 5;
    #pragma omp target data map(tofrom: matrix[idx][0:COLS])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < COLS; j++) {
            matrix[idx][j] = -matrix[idx][j];
        }
    }
    
    /* Compute checksums to prevent dead code elimination */
    for (int i = 0; i < N; i++) sum += arr1D[i];
    printf("arr1D checksum: %d\n", sum);
    
    double matrix_sum = 0.0;
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix_sum += matrix[i][j];
    printf("matrix sum: %f\n", matrix_sum);
    
    printf("Sample values: arr1D[0]=%d, matrix[5][10]=%f, buffer[0]=%f\n",
           arr1D[0], matrix[5][10], buffer[0]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
