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
    /* 1D arrays for simple array sections */
    int arr1D[N];
    double arr2[2*N];
    
    /* 2D arrays for multi-dimensional sections */
    double matrix[ROWS][COLS];
    int grid[30][40];
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1D[i] = i;
        if (i < 2*N) arr2[i] = i * 0.5;
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = i * 100.0 + j;
        }
    }
    
    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 40; j++) {
            grid[i][j] = i * 40 + j;
        }
    }
    
    int start = 5;
    int chunk = CHUNK_SIZE;
    
    /* ============================================
       REQUIREMENT 1: OpenMP Target Data Mapping with Array Sections
       ============================================ */
    
    /* Simple whole-array section */
    #pragma omp target data map(tofrom: arr1D[0:N])
    {
        /* Trivial computation to prevent optimization */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            arr1D[i] += 1;
        }
    }
    
    /* Subsection with non-zero lower bound and computed length */
    int lower = 10;
    int length = 20;
    #pragma omp target data map(tofrom: arr1D[lower:length])
    {
        #pragma omp target teams distribute parallel for
        for (int i = lower; i < lower + length; i++) {
            arr1D[i] *= 2;
        }
    }
    
    /* ============================================
       REQUIREMENT 4: Multi-dimensional Array Sections
       ============================================ */
    
    /* 2D array section */
    #pragma omp target data map(tofrom: grid[0:10][0:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                grid[i][j] += 1000;
            }
        }
    }
    
    /* ============================================
       REQUIREMENT 2: OpenMP Target Enter/Exit Data with Array Sections
       ============================================ */
    
    /* Enter data with 2D array section */
    #pragma omp target enter data map(to: matrix[5:10][0:COLS])
    
    /* Use the mapped section */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 5; i < 15; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] *= 1.5;
        }
    }
    
    /* Exit data with same section */
    #pragma omp target exit data map(from: matrix[5:10][0:COLS])
    
    /* ============================================
       REQUIREMENT 5: Array Sections with Complex Base Expressions
       ============================================ */
    
    /* Complex base: pointer arithmetic - this tests op_prio checks */
    #pragma omp target data map(tofrom: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 100;
        }
    }
    
    /* Complex base: subscripted access in multi-dim array */
    int idx = 2;
    #pragma omp target data map(tofrom: matrix[idx][0:20])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 0; j < 20; j++) {
            matrix[idx][j] -= 50.0;
        }
    }
    
    /* ============================================
       REQUIREMENT 3: OpenMP Task Depend with Array Sections
       ============================================ */
    
    int buffer[100];
    for (int i = 0; i < 100; i++) buffer[i] = i;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* First task writes to array section */
            #pragma omp task depend(out: buffer[0:CHUNK_SIZE])
            {
                for (int i = 0; i < CHUNK_SIZE; i++) {
                    buffer[i] = buffer[i] * 2;
                }
            }
            
            /* Second task reads the same section */
            #pragma omp task depend(in: buffer[0:CHUNK_SIZE])
            {
                int sum = 0;
                for (int i = 0; i < CHUNK_SIZE; i++) {
                    sum += buffer[i];
                }
                /* Use sum to prevent optimization */
                buffer[0] = sum % 100;
            }
            
            /* Task with non-zero lower bound */
            #pragma omp task depend(inout: buffer[start:chunk])
            {
                for (int i = start; i < start + chunk; i++) {
                    buffer[i] += 5;
                }
            }
        }
    }
    
    /* ============================================
       Additional test: Mixed expressions in length
       ============================================ */
    
    int a = 5, b = 10;
    #pragma omp target data map(tofrom: arr2[a:b-a])
    {
        #pragma omp target teams distribute parallel for
        for (int i = a; i < b; i++) {
            arr2[i] /= 2.0;
        }
    }
    
    /* ============================================
       Print results to ensure code has observable behavior
       and prevent dead code elimination
       ============================================ */
    
    printf("Checksum verification:\n");
    int sum1 = 0, sum2 = 0;
    double sum3 = 0.0;
    
    for (int i = 0; i < N; i++) sum1 += arr1D[i];
    for (int i = 0; i < 100; i++) sum2 += buffer[i];
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            sum3 += matrix[i][j];
        }
    }
    
    printf("arr1D checksum: %d\n", sum1);
    printf("buffer checksum: %d\n", sum2);
    printf("matrix checksum: %.2f\n", sum3);
    printf("Sample values: arr1D[0]=%d, grid[5][5]=%d, arr2[10]=%.2f\n", 
           arr1D[0], grid[5][5], arr2[10]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
