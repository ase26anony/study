/* This file is designed to be compiled as both C and C++ with OpenMP support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using OpenMP 4.0+ array section syntax in various clauses. */

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
    /* 1D arrays for basic array sections */
    int arr1D[N];
    double arr2[2*N];
    
    /* 2D array for multi-dimensional sections */
    double matrix[ROWS][COLS];
    
    /* Pointer for complex base expression */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    /* Buffer for task depend clauses */
    int buffer[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1D[i] = i;
        buffer[i] = 0;
    }
    
    for (int i = 0; i < 2*N; i++) {
        arr2[i] = i * 0.5;
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = i * 100.0 + j;
        }
    }
    
    /* ============================================
       Test 1: OpenMP target data with array sections
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
    
    /* Subsection with non-zero lower bound */
    #pragma omp target data map(tofrom: arr1D[10:20])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 30; i++) {
            arr1D[i] += 2;
        }
    }
    
    /* ============================================
       Test 2: Complex base expression with parentheses
       This triggers op_prio checks in the pretty-printer
       ============================================ */
    
    /* Array section with pointer arithmetic base */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            ptr[offset + i] += 3;
        }
    }
    
    /* ============================================
       Test 3: Multi-dimensional array sections
       ============================================ */
    
    /* Whole 2D array section */
    #pragma omp target enter data map(to: matrix[0:ROWS][0:COLS])
    
    /* Subsection of 2D array */
    #pragma omp target data map(tofrom: matrix[5:10][10:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 5; i < 15; i++) {
            for (int j = 10; j < 30; j++) {
                matrix[i][j] *= 2.0;
            }
        }
    }
    
    #pragma omp target exit data map(from: matrix[0:ROWS][0:COLS])
    
    /* ============================================
       Test 4: OpenMP task depend with array sections
       ============================================ */
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Task with depend clause on array section */
            #pragma omp task depend(inout: buffer[0:CHUNK_SIZE])
            {
                for (int i = 0; i < CHUNK_SIZE; i++) {
                    buffer[i] = 1;
                }
            }
            
            /* Another task depending on different section */
            #pragma omp task depend(inout: buffer[CHUNK_SIZE:CHUNK_SIZE])
            {
                for (int i = CHUNK_SIZE; i < 2*CHUNK_SIZE; i++) {
                    buffer[i] = 2;
                }
            }
            
            /* Wait for tasks to complete */
            #pragma omp taskwait
        }
    }
    
    /* ============================================
       Test 5: target enter/exit data with computed bounds
       ============================================ */
    
    int start = 20;
    int length = 40;
    
    #pragma omp target enter data map(to: arr2[start:length])
    
    #pragma omp target teams distribute parallel for
    for (int i = start; i < start + length; i++) {
        arr2[i] += 10.0;
    }
    
    #pragma omp target exit data map(from: arr2[start:length])
    
    /* ============================================
       Final verification and output
       ============================================ */
    
    /* Compute checksums to ensure data is live and computations happened */
    int sum1 = 0;
    double sum2 = 0.0;
    double sum3 = 0.0;
    int sum4 = 0;
    
    for (int i = 0; i < N; i++) {
        sum1 += arr1D[i];
    }
    
    for (int i = 0; i < 2*N; i++) {
        sum2 += arr2[i];
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            sum3 += matrix[i][j];
        }
    }
    
    for (int i = 0; i < N; i++) {
        sum4 += buffer[i];
    }
    
    printf("Checksum 1 (arr1D): %d\n", sum1);
    printf("Checksum 2 (arr2): %.2f\n", sum2);
    printf("Checksum 3 (matrix): %.2f\n", sum3);
    printf("Checksum 4 (buffer): %d\n", sum4);
    
    /* Print sample values to show array sections were modified */
    printf("Sample values after array section operations:\n");
    printf("arr1D[0] = %d, arr1D[10] = %d, arr1D[99] = %d\n", 
           arr1D[0], arr1D[10], arr1D[99]);
    printf("matrix[5][10] = %.2f\n", matrix[5][10]);
    printf("buffer[0] = %d, buffer[CHUNK_SIZE] = %d\n", 
           buffer[0], buffer[CHUNK_SIZE]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
