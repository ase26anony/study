/* This file is designed to be compiled as both C and C++ with OpenMP 4.0+ support.
   It exercises the OMP_ARRAY_SECTION pretty-printer logic in GCC's tree-pretty-print.cc
   by using OpenMP array section syntax [lower:length] in various contexts. */

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
            matrix[i][j] = i * 1.0 + j * 0.01;
    
    /* Buffer for task depend clauses */
    int buffer[N * 2];
    for (int i = 0; i < N * 2; i++) buffer[i] = 0;
    
    /* Pointer for complex base expressions */
    int *ptr = arr1D;
    int offset = 10;
    int size = 30;
    
    int start = 5;
    int chunk = CHUNK_SIZE;
    
    /* Requirement 1: OpenMP target data mapping with array sections */
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
    #pragma omp target data map(tofrom: arr1D[10:20])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 30; i++) {
            arr1D[i] *= 2;
        }
    }
    
    /* Requirement 5: Array section with complex base expression */
    /* Using pointer arithmetic in base expression - tests op_prio parentheses */
    #pragma omp target data map(to: (ptr+offset)[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            (ptr+offset)[i] = i * 3;
        }
    }
    
    /* Requirement 4: Multi-dimensional array sections */
    /* Full slice of 2D array */
    #pragma omp target data map(to: matrix[0:ROWS][0:COLS])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                matrix[i][j] += 1.0;
            }
        }
    }
    
    /* Subsection of 2D array */
    #pragma omp target data map(to: matrix[5:10][10:20])
    {
        #pragma omp target teams distribute parallel for collapse(2)
        for (int i = 5; i < 15; i++) {
            for (int j = 10; j < 30; j++) {
                matrix[i][j] *= 2.0;
            }
        }
    }
    
    /* Requirement 2: OpenMP target enter/exit data with array sections */
    #pragma omp target enter data map(to: matrix[20:5][0:COLS])
    
    /* Do some work on the device */
    #pragma omp target teams distribute parallel for collapse(2)
    for (int i = 20; i < 25; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = matrix[i][j] * 0.5 + 1.0;
        }
    }
    
    #pragma omp target exit data map(from: matrix[20:5][0:COLS])
    
    /* Requirement 3: OpenMP task depend with array sections */
    #pragma omp parallel
    #pragma omp single
    {
        /* First task writes to buffer section */
        #pragma omp task depend(out: buffer[start:chunk])
        {
            for (int i = start; i < start + chunk; i++) {
                buffer[i] = i * 10;
            }
        }
        
        /* Second task reads and modifies the same section */
        #pragma omp task depend(inout: buffer[start:chunk])
        {
            for (int i = start; i < start + chunk; i++) {
                buffer[i] += 5;
            }
        }
        
        /* Third task with different section */
        #pragma omp task depend(inout: buffer[50:25])
        {
            for (int i = 50; i < 75; i++) {
                buffer[i] = buffer[i] * 2;
            }
        }
    }
    
    /* Requirement 6: Print results to prevent dead code elimination */
    int sum1D = 0;
    double sum2D = 0.0;
    int sumBuffer = 0;
    
    for (int i = 0; i < N; i++) sum1D += arr1D[i];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sum2D += matrix[i][j];
    for (int i = 0; i < N * 2; i++) sumBuffer += buffer[i];
    
    printf("Checksums - 1D array: %d, 2D array: %.2f, buffer: %d\n",
           sum1D, sum2D, sumBuffer);
    printf("Sample values - arr1D[0]=%d, matrix[0][0]=%.2f, buffer[%d]=%d\n",
           arr1D[0], matrix[0][0], start, buffer[start]);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
