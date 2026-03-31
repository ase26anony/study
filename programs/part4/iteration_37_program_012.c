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
#define CHUNK 20

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
    int buffer[CHUNK * 3];
    for (int i = 0; i < CHUNK * 3; i++) buffer[i] = 0;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* 1. OpenMP target data mapping with simple array sections */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }
    
    /* 2. Target data with non-zero lower bound and computed length */
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
    #pragma omp target data map(to: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 20; j++)
                matrix[i][j] += 1.0;
    }
    
    /* 4. Complex base expression: pointer arithmetic */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 3;
        }
    }
    
    /* 5. Complex base: subscripted access */
    int idx = 5;
    #pragma omp target data map(to: matrix[idx][0:COLS])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < COLS; j++) {
            matrix[idx][j] *= 2.0;
        }
    }
    
    /* 6. OpenMP target enter/exit data with array sections */
    #pragma omp target enter data map(to: matrix[5:rows][0:cols])
    
    /* Do some work on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 5 + ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] -= 0.5;
    
    #pragma omp target exit data map(from: matrix[5:rows][0:cols])
    
    /* 7. OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writes to buffer section */
        #pragma omp task depend(out: buffer[0:CHUNK])
        {
            for (int i = 0; i < CHUNK; i++) buffer[i] = 1;
        }
        
        /* Second task reads/writes overlapping section */
        #pragma omp task depend(inout: buffer[CHUNK/2:CHUNK])
        {
            for (int i = CHUNK/2; i < CHUNK/2 + CHUNK; i++) {
                if (i < CHUNK * 3) buffer[i] += 2;
            }
        }
        
        /* Third task with different section */
        #pragma omp task depend(in: buffer[CHUNK:CHUNK])
        {
            int sum = 0;
            for (int i = CHUNK; i < 2*CHUNK; i++) sum += buffer[i];
            buffer[0] = sum;
        }
    }
    
    /* Print checksums to ensure data is live and computations happen */
    int sum1D = 0;
    double sum2D = 0.0;
    int sumBuf = 0;
    
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    for (int i = 0; i < CHUNK * 3; i++) sumBuf += buffer[i];
    
    printf("Checksums:\n");
    printf("  arr1D: %d\n", sum1D);
    printf("  matrix: %.2f\n", sum2D);
    printf("  buffer: %d\n", sumBuf);
    
    /* Print a few sample values */
    printf("Sample values:\n");
    printf("  arr1D[0]=%d, arr1D[10]=%d, arr1D[99]=%d\n", 
           arr1D[0], arr1D[10], arr1D[99]);
    printf("  matrix[5][10]=%.2f\n", matrix[5][10]);
    printf("  buffer[0]=%d, buffer[CHUNK]=%d\n", buffer[0], buffer[CHUNK]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
