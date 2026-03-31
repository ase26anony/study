/* This file is designed to trigger coverage of the OMP_ARRAY_SECTION case
   in GCC's tree-pretty-print.cc (lines 2736-2748). It uses OpenMP 4.0+ array
   section syntax in various contexts to ensure the pretty-printer handles
   all code paths, including complex base expressions and multi-dimensional
   sections. Compile with -fopenmp and -fdump-tree-omplower to see the nodes. */

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
            matrix[i][j] = i * 100.0 + j;
    
    /* Buffer for task depend clauses */
    int buffer[2 * N];
    for (int i = 0; i < 2 * N; i++) buffer[i] = i * 2;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* 1. OpenMP target data mapping with simple and non-zero lower bound sections */
    #pragma omp target data map(tofrom: arr1D[0:N])  /* Simple whole array */
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }
    
    /* Subsection with non-zero lower bound and computed length */
    int start = 10;
    int length = 20;
    #pragma omp target data map(tofrom: arr1D[start:length])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = start; i < start + length; i++) {
            arr1D[i] *= 2;
        }
    }
    
    /* 2. OpenMP target enter/exit data with array sections */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])
    
    /* Do some work on the device */
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
        /* First task writes to a section */
        #pragma omp task depend(out: buffer[0:CHUNK])
        {
            for (int i = 0; i < CHUNK; i++) buffer[i] = -i;
        }
        
        /* Second task reads that section and writes to another */
        #pragma omp task depend(in: buffer[0:CHUNK]) depend(out: buffer[CHUNK:CHUNK])
        {
            int sum = 0;
            for (int i = 0; i < CHUNK; i++) sum += buffer[i];
            for (int i = CHUNK; i < 2*CHUNK; i++) buffer[i] = sum + i;
        }
        
        /* Third task depends on the second's output */
        #pragma omp task depend(in: buffer[CHUNK:CHUNK])
        {
            buffer[CHUNK] *= 2;
        }
    }
    
    /* 4. Multi-dimensional array sections */
    #pragma omp target data map(to: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                matrix[i][j] /= 2.0;
            }
        }
    }
    
    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic in base expression - triggers op_prio checks */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 100;
        }
    }
    
    /* Using subscripted access as base: arr1D[i][0:N] isn't valid for 1D,
       so use a 2D slice with variable index */
    int idx = 25;
    #pragma omp target data map(to: matrix[idx][0:10])
    {
        #pragma omp target teams distribute parallel for simd
        for (int j = 0; j < 10; j++) {
            matrix[idx][j] = 3.14159;
        }
    }
    
    /* 6. Print checksums to ensure data is live and computations happen */
    int sum1D = 0;
    double sum2D = 0.0;
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    
    printf("Array checksum: %d\n", sum1D);
    printf("Matrix checksum: %f\n", sum2D);
    printf("Buffer samples: %d, %d, %d\n", buffer[0], buffer[CHUNK], buffer[2*CHUNK-1]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
