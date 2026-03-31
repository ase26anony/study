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
            matrix[i][j] = i * 100.0 + j;
    
    /* Another buffer for task depend clauses */
    float buffer[N * 2];
    for (int i = 0; i < N*2; i++) buffer[i] = i * 1.5f;
    
    /* Pointer for complex base expression */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    int checksum = 0;
    
    /* Requirement 1: OpenMP target data mapping with array sections */
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;  /* Trivial computation to prevent optimization */
        }
    }
    
    /* Subsection with non-zero lower bound and computed length */
    int start = 10;
    int length = 20;
    #pragma omp target data map(tofrom: arr1D[start:length])
    {
        #pragma omp target teams distribute parallel for
        for (int i = start; i < start + length; i++) {
            arr1D[i] *= 2;
        }
    }
    
    /* Requirement 5: Complex base expression with pointer arithmetic */
    /* Note: Some GCC versions may require the base expression to be wrapped */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 3;
        }
    }
    
    /* Requirement 4 & 2: Multi-dimensional array section with target enter/exit data */
    /* First dimension slice */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])
    
    /* Do some work on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] += 1000.0;
    
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* Requirement 3: OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        /* Task producing data */
        #pragma omp task depend(out: buffer[0:CHUNK])
        {
            for (int i = 0; i < CHUNK; i++) buffer[i] = -buffer[i];
        }
        
        /* Task consuming data */
        #pragma omp task depend(inout: buffer[CHUNK:CHUNK])
        {
            for (int i = CHUNK; i < 2*CHUNK; i++) buffer[i] *= 0.5f;
        }
        
        /* Task with complex base: array element as base for section */
        int idx = 40;
        #pragma omp task depend(inout: buffer[idx:10])
        {
            for (int i = idx; i < idx+10; i++) buffer[i] += 25.0f;
        }
    }
    
    /* Compute checksum to ensure data is live and used */
    for (int i = 0; i < N; i++) checksum += arr1D[i];
    checksum += (int)matrix[10][10];
    checksum += (int)buffer[0];
    
    printf("Checksum: %d\n", checksum);
    
    /* Print sample values to prevent dead code elimination */
    printf("Sample values - arr1D[0]=%d, matrix[10][10]=%.1f, buffer[0]=%.1f\n",
           arr1D[0], matrix[10][10], buffer[0]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
