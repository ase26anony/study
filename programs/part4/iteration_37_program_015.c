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
#define CHUNK_SIZE 20

int main(void) {
    /* 1D array for basic array sections */
    int arr1D[N];
    for (int i = 0; i < N; i++) arr1D[i] = i;
    
    /* 2D array for multi-dimensional sections */
    double matrix[ROWS][COLS];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            matrix[i][j] = i * 1.5 + j * 0.5;
    
    /* Another buffer for task depend clauses */
    int buffer[200];
    for (int i = 0; i < 200; i++) buffer[i] = 100 + i;
    
    /* Pointer for complex base expression tests */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* Variable for computed length */
    int computed_len = 25;
    
    /* 1. OpenMP target data mapping with simple and non-zero lower bound array sections */
    #pragma omp target data map(tofrom: arr1D[0:N])  /* Whole array section */
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
            arr1D[i] *= 2;
        }
    }
    
    /* 2. OpenMP target enter/exit data with array sections */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])
    
    /* Do some work on device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] += 1.0;
        }
    }
    
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* 3. OpenMP task depend with array sections */
    int start = 50;
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* First task writes to buffer section */
            #pragma omp task depend(out: buffer[start:CHUNK_SIZE])
            {
                for (int i = start; i < start + CHUNK_SIZE; i++) {
                    buffer[i] = i * 3;
                }
            }
            
            /* Second task reads the same buffer section */
            #pragma omp task depend(in: buffer[start:CHUNK_SIZE])
            {
                int sum = 0;
                for (int i = start; i < start + CHUNK_SIZE; i++) {
                    sum += buffer[i];
                }
                /* Use sum to prevent dead code elimination */
                buffer[0] = sum % 100;
            }
        }
    }
    
    /* 4. Multi-dimensional array sections */
    #pragma omp target data map(to: matrix[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                matrix[i][j] *= 0.5;
            }
        }
    }
    
    /* 5. Array sections with complex base expressions */
    /* Using pointer arithmetic as base */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 5;
        }
    }
    
    /* Using subscripted access as base with computed length */
    int idx = 15;
    #pragma omp target data map(tofrom: arr1D[idx:computed_len])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = idx; i < idx + computed_len; i++) {
            arr1D[i] -= 3;
        }
    }
    
    /* 6. Mixed complex case: pointer with offset and computed length */
    int *dynamic_ptr = &arr1D[30];
    int dyn_offset = 5;
    int dyn_len = 15;
    #pragma omp target enter data map(to: (dynamic_ptr + dyn_offset)[0:dyn_len])
    
    #pragma omp target teams distribute parallel for simd
    for (int i = 0; i < dyn_len; i++) {
        dynamic_ptr[dyn_offset + i] = i * 7;
    }
    
    #pragma omp target exit data map(from: (dynamic_ptr + dyn_offset)[0:dyn_len])
    
    /* Print checksums to ensure data is live and computations happen */
    int sum1D = 0;
    double sum2D = 0.0;
    
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    
    printf("Checksum 1D array: %d\n", sum1D);
    printf("Checksum 2D array: %f\n", sum2D);
    printf("Buffer[%d] = %d\n", start, buffer[start]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
