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
    for (int i = 0; i < N * 2; i++) buffer[i] = i * 1.5f;
    
    /* Pointer for complex base expression */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* Variable-length subsection bounds (non-constant) */
    int start = 5;
    int length = 15;
    
    /* 1. OpenMP target data mapping with simple and non-zero lower bound array sections */
    #pragma omp target data map(tofrom: arr1D[0:N])  /* Whole array section */
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
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])  /* 2D array section */
    
    /* Use the mapped section */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] += 1.0;
        }
    }
    
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* 3. OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        /* Task with inout dependence on array section */
        #pragma omp task depend(inout: buffer[0:CHUNK])
        {
            for (int i = 0; i < CHUNK; i++) {
                buffer[i] = buffer[i] * 2.0f;
            }
        }
        
        /* Another task with out dependence on different section */
        #pragma omp task depend(out: buffer[CHUNK:CHUNK])
        {
            for (int i = CHUNK; i < 2*CHUNK; i++) {
                buffer[i] = -buffer[i];
            }
        }
        
        #pragma omp taskwait
    }
    
    /* 4. Multi-dimensional array section in target data */
    int grid[20][30];
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 30; j++)
            grid[i][j] = i + j;
    
    #pragma omp target data map(tofrom: grid[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                grid[i][j] += 5;
            }
        }
    }
    
    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic in base expression - may trigger op_prio parentheses */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 100;
        }
    }
    
    /* Array section with subscripted base: arr1D[start:length] where start is variable */
    #pragma omp target data map(tofrom: arr1D[start:length])
    {
        #pragma omp target teams distribute parallel for
        for (int i = start; i < start + length; i++) {
            arr1D[i] -= 50;
        }
    }
    
    /* 6. Print checksums to ensure data is live and computations occurred */
    int sum1D = 0;
    double sumMatrix = 0.0;
    float sumBuffer = 0.0f;
    int sumGrid = 0;
    
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sumMatrix += matrix[i][j];
    for (int i = 0; i < N*2; i++) sumBuffer += buffer[i];
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 30; j++)
            sumGrid += grid[i][j];
    
    printf("Checksums (to prevent dead code elimination):\n");
    printf("arr1D sum: %d\n", sum1D);
    printf("matrix sum: %.2f\n", sumMatrix);
    printf("buffer sum: %.2f\n", sumBuffer);
    printf("grid sum: %d\n", sumGrid);
    
    /* Print a few sample values */
    printf("Sample values - arr1D[0]=%d, matrix[5][5]=%.2f, buffer[10]=%.2f, grid[2][3]=%d\n",
           arr1D[0], matrix[5][5], buffer[10], grid[2][3]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
