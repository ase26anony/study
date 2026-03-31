/* This file is designed to trigger the OMP_ARRAY_SECTION pretty-printer
   logic in GCC's tree-pretty-print.cc. It compiles as both valid C and C++.
   Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -c [this_file.c] */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

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
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* Buffer for task depend clauses */
    int buffer[200];
    for (int i = 0; i < 200; i++) buffer[i] = i * 2;
    
    /* 1. OpenMP target data mapping with simple array section */
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
                matrix[i][j] += 1.5;
    }
    
    /* 4. Array section with complex base expression (pointer arithmetic) */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 5;
        }
    }
    
    /* 5. OpenMP target enter/exit data with array sections */
    #pragma omp target enter data map(to: matrix[5:rows][0:cols])
    
    /* Do some work on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] *= 0.5;
    
    #pragma omp target exit data map(from: matrix[5:rows][0:cols])
    
    /* 6. OpenMP task depend with array sections */
    int task_start = 50;
    int chunk_size = CHUNK;
    
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writes to buffer section */
        #pragma omp task depend(out: buffer[task_start:chunk_size])
        {
            for (int i = task_start; i < task_start + chunk_size; i++) {
                buffer[i] = i * 3;
            }
        }
        
        /* Second task reads the same buffer section */
        #pragma omp task depend(in: buffer[task_start:chunk_size])
        {
            int sum = 0;
            for (int i = task_start; i < task_start + chunk_size; i++) {
                sum += buffer[i];
            }
            /* Use sum to prevent dead code elimination */
            buffer[0] = sum % 100;
        }
        
        /* Third task with inout depend clause */
        #pragma omp task depend(inout: buffer[100:30])
        {
            for (int i = 100; i < 130; i++) {
                buffer[i] += 7;
            }
        }
    }
    
    /* Print checksums to ensure data is live and computations happen */
    int sum1D = 0;
    double sum2D = 0.0;
    
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    
    printf("Checksum 1D: %d\n", sum1D);
    printf("Checksum 2D: %f\n", sum2D);
    printf("Buffer[0]: %d\n", buffer[0]);
    printf("Buffer[100]: %d\n", buffer[100]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
