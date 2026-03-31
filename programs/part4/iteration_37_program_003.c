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
#define CHUNK 20

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
    for (int i = 0; i < N*2; i++) buffer[i] = i * 2;
    
    /* Pointer for complex base expression tests */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* Variable for computed length */
    int computed_len = 25;
    
    /* 1. OpenMP target data mapping with simple and non-zero lower bound sections */
    #pragma omp target data map(tofrom: arr1D[0:N])  /* Whole array */
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
            arr1D[i] += 2;
        }
    }
    
    /* 2. OpenMP target enter/exit data with array sections */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])
    
    /* Do some work on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] *= 2.0;
        }
    }
    
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* 3. OpenMP task depend with array sections */
    int start = 40;
    int chunk_size = CHUNK;
    
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writes to buffer section */
        #pragma omp task depend(out: buffer[start:chunk_size])
        {
            for (int i = start; i < start + chunk_size; i++) {
                buffer[i] = i * 3;
            }
        }
        
        /* Second task reads from that section */
        #pragma omp task depend(in: buffer[start:chunk_size])
        {
            int sum = 0;
            for (int i = start; i < start + chunk_size; i++) {
                sum += buffer[i];
            }
            /* Use sum to prevent dead code elimination */
            buffer[0] = sum % 100;
        }
        
        /* Task with inout depend clause */
        #pragma omp task depend(inout: buffer[60:15])
        {
            for (int i = 60; i < 75; i++) {
                buffer[i] += 5;
            }
        }
    }
    
    /* 4. Multi-dimensional array sections */
    double grid[ROWS][COLS];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            grid[i][j] = 0.0;
    
    #pragma omp target data map(to: grid[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                grid[i][j] = i + j;
            }
        }
    }
    
    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic as base */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] *= 2;
        }
    }
    
    /* Using subscripted access as base with computed length */
    int idx = 5;
    #pragma omp target data map(to: arr1D[idx:computed_len])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < computed_len; i++) {
            arr1D[idx + i] += i;
        }
    }
    
    /* 6. Print checksums to ensure data is live and computations happen */
    int sum1D = 0;
    double sumMatrix = 0.0;
    int sumBuffer = 0;
    double sumGrid = 0.0;
    
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sumMatrix += matrix[i][j];
    for (int i = 0; i < N*2; i++) sumBuffer += buffer[i];
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            sumGrid += grid[i][j];
    
    printf("Checksums:\n");
    printf("  arr1D: %d\n", sum1D);
    printf("  matrix: %.2f\n", sumMatrix);
    printf("  buffer: %d\n", sumBuffer);
    printf("  grid[0:10][0:20]: %.2f\n", sumGrid);
    
    /* Print a few sample values */
    printf("Sample values - arr1D[0]=%d, arr1D[10]=%d, matrix[5][0]=%.2f, buffer[40]=%d\n",
           arr1D[0], arr1D[10], matrix[5][0], buffer[40]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
